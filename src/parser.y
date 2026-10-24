%{
#include <stdio.h>
#include <stdlib.h>

extern int yylex(void);
extern int yylineno;
extern FILE *inputFile;
extern int error;

void yyerror(const char *s);
%}

/* Declaracion explicita de los tokens terminales segun el spec */
%token TK_INT            /* entero */
%token TK_REAL           /* real */
%token TK_FUNCTION       /* funcion */
%token TK_MAIN           /* principal */
%token TK_RETURN         /* retornar */
%token TK_SWITCH         /* segun */
%token TK_CASE           /* caso */
%token TK_DEFAULT        /* defecto */
%token TK_WHILE          /* mientras */
%token TK_TOINT          /* aEntero */
%token TK_TOREAL         /* aReal */
%token TK_AND            /* y */
%token TK_OR             /* o */

%token TK_ID             /* identificador */
%token TK_NUM_INT        /* constante entera */
%token TK_NUM_REAL       /* constante real */

%token TK_ASSIGN         /* = */
%token TK_PLUS           /* + */
%token TK_MINUS          /* - */
%token TK_MULT           /* * */
%token TK_DIV            /* / */

%token TK_LT             /* < */
%token TK_GT             /* > */
%token TK_LE             /* <= */
%token TK_GE             /* >= */
%token TK_EQ             /* == */
%token TK_NEQ            /* != */

%token TK_LPAREN         /* ( */
%token TK_RPAREN         /* ) */
%token TK_LBRACE         /* { */
%token TK_RBRACE         /* } */
%token TK_SEMI           /* ; */
%token TK_COLON          /* : */

/* Precedencia y asociatividad para resolver ambiguedades de expresiones */
%left TK_OR
%left TK_AND
%left TK_EQ TK_NEQ
%left TK_LT TK_GT TK_LE TK_GE
%left TK_PLUS TK_MINUS
%left TK_MULT TK_DIV
%right UMINUS

%start programa

%%

/* -------------------------------------------------------------
 * 6. Gramatica formal DUOTIPO
 * ------------------------------------------------------------- */

programa
    : declaraciones_iniciales TK_MAIN TK_LBRACE bloque_principal TK_RBRACE
    | TK_MAIN TK_LBRACE bloque_principal TK_RBRACE
    ;

declaraciones_iniciales
    : decl_globales
    | decl_funciones
    | decl_globales decl_funciones
    ;

bloque_principal
    : decl_locales sentencias
    | sentencias
    ;

decl_globales
    : decl_globales decl_var
    | decl_var
    ;

decl_locales
    : decl_locales decl_var
    | decl_var
    ;

decl_var
    : tipo TK_ID TK_SEMI
    ;

tipo
    : TK_INT
    | TK_REAL
    ;

decl_funciones
    : decl_funciones decl_funcion
    | decl_funcion
    ;

decl_funcion
    : TK_FUNCTION tipo TK_ID TK_LPAREN TK_RPAREN TK_LBRACE bloque_funcion TK_RBRACE
    ;

bloque_funcion
    : decl_locales sentencias
    | sentencias
    ;

sentencias
    : sentencias sentencia
    | sentencia
    ;

sentencia
    : asignacion
    | segun
    | mientras
    | retorno
    | llamada_funcion TK_SEMI
    ;

asignacion
    : TK_ID TK_ASSIGN expresion TK_SEMI
    ;

retorno
    : TK_RETURN expresion TK_SEMI
    ;

llamada_funcion
    : TK_ID TK_LPAREN TK_RPAREN
    ;

segun
    : TK_SWITCH TK_LPAREN TK_ID TK_RPAREN TK_LBRACE casos defecto TK_RBRACE
    | TK_SWITCH TK_LPAREN TK_ID TK_RPAREN TK_LBRACE defecto TK_RBRACE
    ;

casos
    : casos caso
    | caso
    ;

caso
    : TK_CASE TK_NUM_INT TK_COLON sentencias
    ;

defecto
    : TK_DEFAULT TK_COLON sentencias
    ;

mientras
    : TK_WHILE TK_LPAREN condicion TK_RPAREN TK_LBRACE sentencias TK_RBRACE
    ;

condicion
    : condicion TK_OR cond_y
    | cond_y
    ;

cond_y
    : cond_y TK_AND cond_prim
    | cond_prim
    ;

cond_prim
    : TK_LPAREN condicion TK_RPAREN
    | comparacion
    ;

comparacion
    : expresion op_rel expresion
    ;

op_rel
    : TK_LT
    | TK_GT
    | TK_LE
    | TK_GE
    | TK_EQ
    | TK_NEQ
    ;

expresion
    : expresion TK_PLUS termino
    | expresion TK_MINUS termino
    | termino
    ;

termino
    : termino TK_MULT termino_n
    | termino TK_DIV termino_n
    | termino_n
    ;

termino_n
    : TK_MINUS termino_n %prec UMINUS
    | TK_ID
    | TK_NUM_INT
    | TK_NUM_REAL
    | TK_TOINT TK_LPAREN expresion TK_RPAREN
    | TK_TOREAL TK_LPAREN expresion TK_RPAREN
    | TK_LPAREN expresion TK_RPAREN
    | llamada_funcion
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error sintactico (P01): %s en linea %d\n", s, yylineno);
    error = 1;
}

int main(int argc, char *argv[]) {
    const char *nombre_archivo = "prueba.txt";

    if (argc > 1) {
        nombre_archivo = argv[1];
    }

    if ((inputFile = fopen(nombre_archivo, "r")) == NULL) {
        fprintf(stderr, "Error al abrir el archivo de entrada: %s\n", nombre_archivo);
        return 1;
    }

    /* Inicia el analisis sintactico LALR(1) */
    int resultado = yyparse();

    fclose(inputFile);

    if (resultado == 0 && error == 0) {
        printf("\n- Analisis Sintactico y Lexico EXITOSO -\n");
        return 0;
    } else {
        printf("\n- Se encontraron ERRORES durante la compilacion -\n");
        return 1;
    }
}