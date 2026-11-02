
%{
/*
 * ============================================================================
 * Analizador Sintáctico de DUOTIPO (GNU Bison)
 * ============================================================================
 * Implementación de la Gramática Independiente del Contexto (LALR(1)).
 * Invoca bajo demanda al analizador léxico 'yylex()' desarrollado en C.
 * En esta fase el objetivo exclusivo es validar la corrección sintáctica
 * retornando 1 (éxito) o 0 (falla), sin generar código intermedio ni tercetos.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "/lexico/lexer.h"
#include "/tabla_simbolos/tabla_simbolos.h"

extern int yylineno;
extern FILE *yyin;

/* Prototipos de funciones locales */
int yylex(void);
void yyerror(const char *s);

/* Banderas de estado */
static int hubo_error_sintactico = 0;

%}

/* Habilitar depuración en Bison para trazar desplazamientos y reducciones */
%debug

/* -------------------------------------------------------------------------
 * Declaración de Tokens y códigos canónicos de DUOTIPO (>= 301)
 * ------------------------------------------------------------------------- */
%token TK_INT        301 /* entero */
%token TK_REAL       302 /* real */
%token TK_FUNCTION   303 /* funcion */
%token TK_MAIN       304 /* principal */
%token TK_RETURN     305 /* retornar */
%token TK_SWITCH     306 /* segun */
%token TK_CASE       307 /* caso */
%token TK_DEFAULT    308 /* defecto */
%token TK_WHILE      309 /* mientras */
%token TK_TOINT      310 /* aEntero */
%token TK_TOREAL     311 /* aReal */
%token TK_AND        312 /* y */
%token TK_OR         313 /* o */
%token TK_ID         314 /* identificador */
%token TK_NUM_INT    315 /* constante entera */
%token TK_NUM_REAL   316 /* constante real */
%token TK_ASSIGN     317 /* = */
%token TK_PLUS       318 /* + */
%token TK_MINUS      319 /* - */
%token TK_MULT       320 /* * */
%token TK_DIV        321 /* / */
%token TK_LT         322 /* < */
%token TK_GT         323 /* > */
%token TK_LE         324 /* <= */
%token TK_GE         325 /* >= */
%token TK_EQ         326 /* == */
%token TK_NEQ        327 /* != */
%token TK_LPAREN     328 /* ( */
%token TK_RPAREN     329 /* ) */
%token TK_LBRACE     330 /* { */
%token TK_RBRACE     331 /* } */
%token TK_SEMI       332 /* ; */
%token TK_COLON      334 /* : */
%token TK_ESCR       335 /* escribir */
%token TK_OTRO       337 /* error lexico */

/* -------------------------------------------------------------------------
 * Precedencia y Asociatividad
 * ------------------------------------------------------------------------- */
%left TK_OR
%left TK_AND
%left TK_EQ TK_NEQ TK_LT TK_LE TK_GT TK_GE
%left TK_PLUS TK_MINUS
%left TK_MULT TK_DIV
%right UMINUS

/* Axioma inicial */
%start programa

%%

/* =========================================================================
 * 1. Estructura Global del Programa
 * ========================================================================= */
programa:
      declaraciones_iniciales TK_MAIN TK_LBRACE bloque_principal TK_RBRACE
    | TK_MAIN TK_LBRACE bloque_principal TK_RBRACE
    ;

declaraciones_iniciales:
      decl_globales
    | decl_funciones
    | decl_globales decl_funciones
    ;

bloque_principal:
      decl_locales sentencias
    | sentencias
    ;

/* =========================================================================
 * 2. Declaraciones de Variables y Tipos
 * ========================================================================= */
decl_globales:
      decl_globales decl_var
    | decl_var
    ;

decl_locales:
      decl_locales decl_var
    | decl_var
    ;

decl_var:
      tipo TK_ID TK_SEMI
    ;

tipo:
      TK_INT
    | TK_REAL
    ;

/* =========================================================================
 * 3. Declaraciones de Funciones
 * ========================================================================= */
decl_funciones:
      decl_funciones decl_funcion
    | decl_funcion
    ;

decl_funcion:
      TK_FUNCTION tipo TK_ID TK_LPAREN TK_RPAREN TK_LBRACE bloque_funcion TK_RBRACE
    ;

bloque_funcion:
      decl_locales sentencias
    | sentencias
    ;

/* =========================================================================
 * 4. Sentencias y Estructuras de Control
 * ========================================================================= */
sentencias:
      sentencias sentencia
    | sentencia
    ;

sentencia:
      asignacion
    | segun
    | mientras
    | retorno
    | llamada_funcion TK_SEMI
    | llamada_escribir TK_SEMI
    ;

asignacion:
      TK_ID TK_ASSIGN expresion TK_SEMI
    ;

retorno:
      TK_RETURN expresion TK_SEMI
    ;

llamada_funcion:
      TK_ID TK_LPAREN TK_RPAREN
    ;

llamada_escribir:
      TK_ESCR TK_LPAREN expresion TK_RPAREN
    ;

segun:
      TK_SWITCH TK_LPAREN TK_ID TK_RPAREN TK_LBRACE casos defecto TK_RBRACE
    | TK_SWITCH TK_LPAREN TK_ID TK_RPAREN TK_LBRACE defecto TK_RBRACE
    ;

casos:
      casos caso
    | caso
    ;

caso:
      TK_CASE TK_NUM_INT TK_COLON sentencias
    ;

defecto:
      TK_DEFAULT TK_COLON sentencias
    ;

mientras:
      TK_WHILE TK_LPAREN condicion TK_RPAREN TK_LBRACE sentencias TK_RBRACE
    ;

/* =========================================================================
 * 5. Condiciones y Comparaciones
 * ========================================================================= */
condicion:
      condicion TK_OR cond_y
    | cond_y
    ;

cond_y:
      cond_y TK_AND cond_prim
    | cond_prim
    ;

cond_prim:
      TK_LPAREN condicion TK_RPAREN
    | comparacion
    ;

comparacion:
      expresion op_rel expresion
    ;

op_rel:
      TK_LT
    | TK_GT
    | TK_LE
    | TK_GE
    | TK_EQ
    | TK_NEQ
    ;

/* =========================================================================
 * 6. Expresiones Aritméticas y Términos
 * ========================================================================= */
expresion:
      expresion TK_PLUS termino
    | expresion TK_MINUS termino
    | termino
    ;

termino:
      termino TK_MULT termino_n
    | termino TK_DIV termino_n
    | termino_n
    ;

termino_n:
      TK_MINUS termino_n %prec UMINUS
    | TK_ID
    | TK_NUM_INT
    | TK_NUM_REAL
    | TK_TOINT TK_LPAREN expresion TK_RPAREN
    | TK_TOREAL TK_LPAREN expresion TK_RPAREN
    | llamada_funcion
    | TK_LPAREN expresion TK_RPAREN
    ;

%%

/* =========================================================================
 * Rutinas de Soporte en C
 * ========================================================================= */

void yyerror(const char *s) {
    hubo_error_sintactico = 1;
    fprintf(stderr, "Error sintactico (P01): %s en linea %d\n", s, yylineno);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo_fuente> [--debug]\n", argv[0]);
        return 0;
    }

    FILE *archivo = fopen(argv[1], "r");
    if (!archivo) {
        fprintf(stderr, "Error: No se pudo abrir el archivo '%s'\n", argv[1]);
        return 0;
    }

    yyin = archivo;
    yylineno = 1;
    hubo_error_sintactico = 0;

    /* Activar modo debug si se solicita por parámetro */
    if (argc >= 3 && strcmp(argv[2], "--debug") == 0) {
        #if YYDEBUG
        yydebug = 1;
        printf("--- Modo debug de Bison activado ---\n");
        #endif
    }

    /* Ejecutar el análisis sintáctico */
    int resultado = yyparse();
    fclose(archivo);

    if (resultado == 0 && hubo_error_sintactico == 0) {
        printf("Compilacion exitosa: El programa cumple la gramatica de DUOTIPO.\n");
        /* Exportar la tabla de símbolos a out/ts.txt */
        exportar_tabla_simbolos("out/ts.txt");
        return 1;
    } else {
        printf("Compilacion no exitosa: Se encontraron errores durante el analisis.\n");
        return 0;
    }
}
