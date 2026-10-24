#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Header generado por Bison al compilar con: bison -d parser.y */
#include "parser.tab.h"

#define TAM_MAX_ID 20
#define TAM_MAX_NUM 30

/* Matrices en archivo externo */
#include "matriz_estados.h"
#include "matriz_tokens.h"
#include "matriz_funciones.h"

FILE *inputFile = NULL;
int error = 0;
int yylineno = 1;

char buffer[128];
int bufLen = 0;
int c = 0;
int primer_caracter_leido = 0;
int ultimo_estado = 0;
int ultima_columna = 0;
int dentro_comentario = 0; /* 1 mientras el automata esta dentro de un comentario (lo maneja f12/f13) */
int linea_comentario = 0;  /* linea donde empezo el comentario en curso */

/* Notifica un error lexico ocurrido en el archivo. */
int yyerror(const char *s) {
    fprintf(stderr, "\nError lexico (L01): %s en linea %d.\n", s, yylineno);
    error = 1;
    return -1;
}

/* Desambigua si una secuencia de letras corresponde a una palabra reservada o identificador */
int palabras_reservadas(const char *s) {
    if (strcmp(s, "entero") == 0)    return TK_INT;
    if (strcmp(s, "real") == 0)      return TK_REAL;
    if (strcmp(s, "funcion") == 0)   return TK_FUNCTION;
    if (strcmp(s, "principal") == 0) return TK_MAIN;
    if (strcmp(s, "retornar") == 0)  return TK_RETURN;
    if (strcmp(s, "segun") == 0)     return TK_SWITCH;
    if (strcmp(s, "caso") == 0)      return TK_CASE;
    if (strcmp(s, "defecto") == 0)   return TK_DEFAULT;
    if (strcmp(s, "mientras") == 0)  return TK_WHILE;
    if (strcmp(s, "aEntero") == 0)   return TK_TOINT;
    if (strcmp(s, "aReal") == 0)     return TK_TOREAL;
    if (strcmp(s, "y") == 0)         return TK_AND;
    if (strcmp(s, "o") == 0)         return TK_OR;
    return TK_ID;
}

/* Comprueba que el valor numerico no desborde un entero de 32 bits con signo */
int validar_rango_entero(const char *s) {
    int len = (int)strlen(s);
    if (len > 10) return 0;
    if (len == 10 && strcmp(s, "2147483647") > 0) return 0;
    return 1;
}

/* Caracter no valido en E0 */
int fe(char caracter) {
    char msg[64];
    snprintf(msg, sizeof(msg), "Caracter invalido '%c'", caracter);
    yyerror(msg);
    buffer[0] = '\0';
    return -1;
}

/* No hace nada */
int fn(char caracter) {
    (void)caracter;
    return 0;
}

/* Deslectura */
int f0(char caracter) {
    ungetc(caracter, inputFile);
    return 0;
}

/* Inicializa el buffer con la primera letra de un identificador desde E0 */
int f1(char caracter) {
    bufLen = 0;
    buffer[bufLen++] = caracter;
    buffer[bufLen] = '\0';
    return 0;
}

/* Inicializa el buffer con el primer digito de una constante numerica desde E0 */
int f2(char caracter) {
    bufLen = 0;
    buffer[bufLen++] = caracter;
    buffer[bufLen] = '\0';
    return 0;
}

/* Agrega caracteres subsiguientes (letras o digitos) al buffer del identificador */
int f3(char caracter) {
    if (bufLen < 120) {
        buffer[bufLen++] = caracter;
        buffer[bufLen] = '\0';
    }
    return 0;
}

/* Agrega digitos subsiguientes o el punto decimal a una constante numerica */
int f4(char caracter) {
    if (bufLen < 120) {
        buffer[bufLen++] = caracter;
        buffer[bufLen] = '\0';
    }
    return 0;
}

/* Cierre de constante numerica. Deslee el delimitador y valida el rango si es entero */
int f5(char caracter) {
    f0(caracter);

    if (strchr(buffer, '.') == NULL) {
        if (validar_rango_entero(buffer) == 0) {
            yyerror("Constante entera fuera de rango (-2147483648 a 2147483647)");
            buffer[0] = '\0';
        }
    }
    return 0;
}

/* Cierre de identificador. Deslee el delimitador y comprueba la longitud maxima */
int f6(char caracter) {
    f0(caracter);

    if (strlen(buffer) > TAM_MAX_ID) {
        yyerror("Identificador excede el largo maximo permitido (20 caracteres)");
        buffer[0] = '\0';
    }
    return 0;
}

/* Inicializa y almacena un token simple de un solo caracter desde E0 */
int f7(char caracter) {
    bufLen = 0;
    buffer[bufLen++] = caracter;
    buffer[bufLen] = '\0';
    return 0;
}

/* Concatena el segundo caracter a operadores compuestos (==, <=, >=, !=) */
int f8(char caracter) {
    buffer[bufLen++] = caracter;
    buffer[bufLen] = '\0';
    return 0;
}

/* Blanco consumido por el automata: solo cuenta saltos de linea */
int f9(char caracter) {
    if (caracter == '\n') {
        yylineno++;
    }
    return 0;
}

/* '!' que no forma '!=' */
int f10(char caracter) {
    f0(caracter);
    yyerror("Operador '!' incompleto (se esperaba '!=')");
    buffer[0] = '\0';
    return 0;
}

/* Fin de archivo dentro de un comentario que nunca se cerro */
int f11(char caracter) {
    int linea_actual = yylineno;
    (void)caracter;
    yylineno = linea_comentario;
    yyerror("Comentario sin cerrar");
    yylineno = linea_actual;
    buffer[0] = '\0';
    dentro_comentario = 0;
    return 0;
}

/* Inicio de comentario */
int f12(char caracter) {
    (void)caracter;
    dentro_comentario = 1;
    linea_comentario = yylineno;
    return 0;
}

/* Fin de comentario */
int f13(char caracter) {
    (void)caracter;
    dentro_comentario = 0;
    return 0;
}

/* Obtiene la columna del caracter segun la matriz del spec */
int get_evento(int caracter) {
    if (caracter == EOF) return COL_FDA;
    if (isalpha(caracter)) return 0;
    if (isdigit(caracter)) return 1;
    if (caracter == '=') return 2;
    if (caracter == '+') return 3;
    if (caracter == '-') return 4;
    if (caracter == '*') return 5;
    if (caracter == '/') return 6;
    if (caracter == '<') return 7;
    if (caracter == '>') return 8;
    if (caracter == '!') return 9;
    if (caracter == '(') return 10;
    if (caracter == ')') return 11;
    if (caracter == '{') return 12;
    if (caracter == '}') return 13;
    if (caracter == ';') return 14;
    if (caracter == ':') return 15;
    if (caracter == '.') return 16;
    if (isspace(caracter)) return COL_BLANCO;
    if (dentro_comentario) return COL_LETRA;

    char msg[64];
    snprintf(msg, sizeof(msg), "Caracter fuera del alfabeto '%c'", (char)caracter);
    yyerror(msg);
    c = ' ';
    return COL_BLANCO;
}

/* Traduce el codigo numerico de la matriz al token definido en Bison */
int mapear_token_bison(int tipotoken) {
    switch (tipotoken) {
        case 14: return palabras_reservadas(buffer);
        case 15: return TK_NUM_INT;
        case 16: return TK_NUM_REAL;
        case 17: return TK_ASSIGN;
        case 18: return TK_PLUS;
        case 19: return TK_MINUS;
        case 20: return TK_MULT;
        case 21: return TK_DIV;
        case 22: return TK_LT;
        case 23: return TK_GT;
        case 24: return TK_LE;
        case 25: return TK_GE;
        case 26: return TK_EQ;
        case 27: return TK_NEQ;
        case 28: return TK_LPAREN;
        case 29: return TK_RPAREN;
        case 30: return TK_LBRACE;
        case 31: return TK_RBRACE;
        case 32: return TK_SEMI;
        case 34: return TK_COLON;
        default: return -1;
    }
}

/* yylex invocado por Bison */
int yylex(void) {
    int estado = 0;
    int estado_final = -1;
    int columna;
    int tipotoken;

    /* Asegura la primera lectura del archivo cuando yyparse() toma el control */
    if (!primer_caracter_leido) {
        c = fgetc(inputFile);
        primer_caracter_leido = 1;
    }

    bufLen = 0;
    buffer[0] = '\0';

    while (estado != estado_final) {
        /* Si alcanzamos EOF y no estamos en medio de un token compuesto */
        if (c == EOF && estado == 0) {
            return 0; /* 0 indica fin de archivo a Bison */
        }

        columna = get_evento(c);

        matriz_funciones[estado][columna]((char)c);

        ultimo_estado = estado;
        ultima_columna = columna;

        estado = matriz_estados[estado][columna];

        c = fgetc(inputFile);
    }

    tipotoken = matriz_tokens[ultimo_estado][ultima_columna];
    return mapear_token_bison(tipotoken);
}

/* 
 * MAIN COMENTADO:
 * El punto de entrada ahora se encuentra en parser.y mediante yyparse().
 * Se deja comentado como referencia para pruebas independientes del lexer.
 */

/*
int main(int argc, char *argv[]) {
    int token;
    const char *nombre_archivo = "prueba.txt";

    if (argc > 1) {
        nombre_archivo = argv[1];
    }

    if ((inputFile = fopen(nombre_archivo, "r")) == NULL) {
        printf("Error: no se pudo abrir el archivo %s\n", nombre_archivo);
        return 1;
    }

    printf("TOKEN | LEXEMA\n");
    printf("-------------------\n");

    while ((token = yylex()) != 0) {
        if (token > 0 && buffer[0] != '\0') {
            printf("%d | %s\n", token, buffer);
        }
    }

    if (fclose(inputFile) != 0) {
        printf("Error al cerrar el archivo.\n");
    }

    if (error == 0) {
        printf("\n - Compilacion AL EXITOSA - \n");
    } else {
        printf("\n - Analisis Lexico completo con ERRORES - \n");
    }

    return error;
}
*/
