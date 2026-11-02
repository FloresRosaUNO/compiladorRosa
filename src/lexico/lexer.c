#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "lexer.h"

/* Variables globales públicas */
FILE *yyin = NULL;
int yylineno = 1;
char yytext[MAX_BUFFER_LEXICO];

/* Variables internas del analizador */
static char buffer_lexema[MAX_BUFFER_LEXICO];
static int idx_buffer = 0;
static int flag_warning_id = 0;
static int token_actual = -1;
static int en_comentario = 0;
static int linea_inicio_comentario = 1;

/* Estructura para búsqueda de palabras reservadas */
typedef struct {
    const char *palabra;
    int token;
} PalabraReservada;

static const PalabraReservada palabras_reservadas[] = {
    {"aEntero",   TK_TOINT},
    {"aReal",     TK_TOREAL},
    {"caso",      TK_CASE},
    {"defecto",   TK_DEFAULT},
    {"entero",    TK_INT},
    {"escribir",  TK_ESCR},
    {"funcion",   TK_FUNCTION},
    {"mientras",  TK_WHILE},
    {"o",         TK_OR},
    {"principal", TK_MAIN},
    {"real",      TK_REAL},
    {"retornar",  TK_RETURN},
    {"segun",     TK_SWITCH},
    {"y",         TK_AND}
};
static const int cant_palabras_reservadas = sizeof(palabras_reservadas) / sizeof(palabras_reservadas[0]);

static int buscar_palabra_reservada(const char *lexema) {
    for (int i = 0; i < cant_palabras_reservadas; i++) {
        if (strcmp(palabras_reservadas[i].palabra, lexema) == 0) {
            return palabras_reservadas[i].token;
        }
    }
    return -1;
}

void inicializar_lexico(FILE *archivo) {
    yyin = archivo ? archivo : stdin;
    yylineno = 1;
    idx_buffer = 0;
    flag_warning_id = 0;
    token_actual = -1;
    en_comentario = 0;
    linea_inicio_comentario = 1;
    buffer_lexema[0] = '\0';
    yytext[0] = '\0';
    inicializar_tabla_simbolos();
}

int get_evento(int c) {
    if (c == EOF) {
        return EV_FDA;
    }
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        return EV_LETRA;
    }
    if (c >= '0' && c <= '9') {
        return EV_DIGITO;
    }
    switch (c) {
        case '=':  return EV_IGUAL;
        case '+':  return EV_MAS;
        case '-':  return EV_MENOS;
        case '*':  return EV_POR;
        case '/':  return EV_BARRA;
        case '<':  return EV_MENOR;
        case '>':  return EV_MAYOR;
        case '!':  return EV_ADMIRACION;
        case '(':  return EV_ABRE_PAREN;
        case ')':  return EV_CIERRA_PAREN;
        case '{':  return EV_ABRE_LLAVE;
        case '}':  return EV_CIERRA_LLAVE;
        case ';':  return EV_PUNTO_COMA;
        case ':':  return EV_DOS_PUNTOS;
        case '.':  return EV_PUNTO;
        case ' ':
        case '\t':
        case '\r':
        case '\n': return EV_BLANCO;
        default:   return EV_OTRO;
    }
}

/* =========================================================================
 * ACCIONES SEMANTICAS
 * ========================================================================= */

static void f_nada(int c) {
    (void)c;
}

static void f_acumular_id(int c) {
    if (idx_buffer < LIMITE_LONGITUD_ID) {
        buffer_lexema[idx_buffer++] = (char)c;
        buffer_lexema[idx_buffer] = '\0';
    } else {
        flag_warning_id = 1;
    }
}

static void f_fin_id(int c) {
    (void)c;
    buffer_lexema[idx_buffer] = '\0';
    
    if (flag_warning_id) {
        fprintf(stderr, "Advertencia léxica en línea %d: Identificador supera el límite de 20 caracteres. Se truncará a '%s'.\n",
                yylineno, buffer_lexema);
        flag_warning_id = 0;
    }

    int tok_reservado = buscar_palabra_reservada(buffer_lexema);
    if (tok_reservado != -1) {
        token_actual = tok_reservado;
    } else {
        token_actual = TK_ID;
        insertar_simbolo(buffer_lexema, TK_ID, "identificador", NULL);
    }
}

static void f_acumular_num(int c) {
    if (idx_buffer < MAX_BUFFER_LEXICO - 1) {
        buffer_lexema[idx_buffer++] = (char)c;
        buffer_lexema[idx_buffer] = '\0';
    }
}

static void f_fin_num(int c) {
    (void)c;
    buffer_lexema[idx_buffer] = '\0';

    if (strchr(buffer_lexema, '.') != NULL) {
        token_actual = TK_NUM_REAL;
        insertar_simbolo(buffer_lexema, TK_NUM_REAL, "real", buffer_lexema);
    } else {
        char *endptr = NULL;
        errno = 0;
        long long val = strtoll(buffer_lexema, &endptr, 10);
        if (errno == ERANGE || val < -2147483648LL || val > 2147483647LL) {
            fprintf(stderr, "Error léxico en línea %d: Constante entera '%s' fuera de rango (32 bits con signo).\n",
                    yylineno, buffer_lexema);
        }
        token_actual = TK_NUM_INT;
        insertar_simbolo(buffer_lexema, TK_NUM_INT, "entero", buffer_lexema);
    }
}

static void f_acumular_op(int c) {
    if (idx_buffer < MAX_BUFFER_LEXICO - 1) {
        buffer_lexema[idx_buffer++] = (char)c;
        buffer_lexema[idx_buffer] = '\0';
    }
}

static void f_contar_linea(int c) {
    if (c == '\n') {
        yylineno++;
    }
}

static void f_inicio_coment(int c) {
    (void)c;
    en_comentario = 1;
    linea_inicio_comentario = yylineno;
    idx_buffer = 0;
    buffer_lexema[0] = '\0';
}

static void f_fin_coment(int c) {
    (void)c;
    en_comentario = 0;
    idx_buffer = 0;
    buffer_lexema[0] = '\0';
}

static void f_error_lexico(int c, int estado) {
    if (en_comentario && c == EOF) {
        fprintf(stderr, "Error léxico: Fin de archivo inesperado. Comentario abierto en línea %d no fue cerrado.\n",
                linea_inicio_comentario);
        return;
    }
    if (estado == EST_POSIBLE_DISTINTO) {
        fprintf(stderr, "Error léxico en línea %d: Carácter '!' inesperado. Se esperaba '=' para formar '!='.\n",
                yylineno);
        return;
    }
    if (c == '.') {
        fprintf(stderr, "Error léxico en línea %d: Punto decimal '.' aislado no válido sin parte entera previa.\n",
                yylineno);
        return;
    }
    if (c != EOF) {
        fprintf(stderr, "Error léxico en línea %d: Carácter '%c' no válido en el lenguaje.\n",
                yylineno, (char)c);
    }
}

static void ejecutar_accion(int accion, int c, int estado) {
    switch (accion) {
        case ACT_FN: f_nada(c); break;
        case ACT_F1: f_acumular_id(c); break;
        case ACT_F2: f_fin_id(c); break;
        case ACT_F3: f_acumular_num(c); break;
        case ACT_F4: f_fin_num(c); break;
        case ACT_F5: f_acumular_op(c); break;
        case ACT_F6: f_contar_linea(c); break;
        case ACT_F7: f_inicio_coment(c); break;
        case ACT_F8: f_fin_coment(c); break;
        case ACT_FE: f_error_lexico(c, estado); break;
        default:     f_nada(c); break;
    }
}

/* =========================================================================
 * BUCLE PRINCIPAL yylex()
 * ========================================================================= */

int yylex(void) {
    while (1) {
        int estado = EST_INICIAL;
        int estado_previo = EST_INICIAL;
        int ev_ultimo = EV_BLANCO;
        
        idx_buffer = 0;
        buffer_lexema[0] = '\0';
        flag_warning_id = 0;
        token_actual = -1;

        while (estado != EST_FINAL) {
            int c = fgetc(yyin);
            int ev = get_evento(c);
            ev_ultimo = ev;

            int prox_estado = nuevo_estado[estado][ev];
            int accion = matriz_acciones[estado][ev];

            /* 1. Ejecutar acción semántica */
            ejecutar_accion(accion, c, estado);

            /* 2. Evaluar Unread (devolver al flujo si corresponde) */
            if (matriz_unreads[estado][ev] && c != EOF) {
                ungetc(c, yyin);
            }

            /* Caso especial: EOF en estado inicial -> retornar 0 (YYEOF) */
            if (c == EOF && estado == EST_INICIAL && prox_estado == EST_FINAL) {
                yytext[0] = '\0';
                return 0;
            }

            estado_previo = estado;
            estado = prox_estado;
        }

        /* Determinar token a emitir */
        if (token_actual != -1) {
            strncpy(yytext, buffer_lexema, MAX_BUFFER_LEXICO - 1);
            yytext[MAX_BUFFER_LEXICO - 1] = '\0';
            return token_actual;
        }

        int tok_matriz = matriz_tokens[estado_previo][ev_ultimo];
        if (tok_matriz > 0) {
            strncpy(yytext, buffer_lexema, MAX_BUFFER_LEXICO - 1);
            yytext[MAX_BUFFER_LEXICO - 1] = '\0';
            return tok_matriz;
        }
        
        if (tok_matriz == 0) {
            yytext[0] = '\0';
            return 0;
        }

        /* Si fue un token descartable (espacio o comentario) continúa buscando el siguiente */
    }
}


