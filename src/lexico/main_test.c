#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

static const char *obtener_nombre_token(int token) {
    switch (token) {
        case TK_INT:         return "TK_INT";
        case TK_REAL:        return "TK_REAL";
        case TK_FUNCTION:    return "TK_FUNCTION";
        case TK_MAIN:        return "TK_MAIN";
        case TK_RETURN:      return "TK_RETURN";
        case TK_SWITCH:      return "TK_SWITCH";
        case TK_CASE:        return "TK_CASE";
        case TK_DEFAULT:     return "TK_DEFAULT";
        case TK_WHILE:       return "TK_WHILE";
        case TK_TOINT:       return "TK_TOINT";
        case TK_TOREAL:      return "TK_TOREAL";
        case TK_AND:         return "TK_AND";
        case TK_OR:          return "TK_OR";
        case TK_ID:          return "TK_ID";
        case TK_NUM_INT:     return "TK_NUM_INT";
        case TK_NUM_REAL:    return "TK_NUM_REAL";
        case TK_ASSIGN:      return "TK_ASSIGN";
        case TK_PLUS:        return "TK_PLUS";
        case TK_MINUS:       return "TK_MINUS";
        case TK_MULT:        return "TK_MULT";
        case TK_DIV:         return "TK_DIV";
        case TK_LT:          return "TK_LT";
        case TK_GT:          return "TK_GT";
        case TK_LE:          return "TK_LE";
        case TK_GE:          return "TK_GE";
        case TK_EQ:          return "TK_EQ";
        case TK_NEQ:         return "TK_NEQ";
        case TK_LPAREN:      return "TK_LPAREN";
        case TK_RPAREN:      return "TK_RPAREN";
        case TK_LBRACE:      return "TK_LBRACE";
        case TK_RBRACE:      return "TK_RBRACE";
        case TK_SEMI:        return "TK_SEMI";
        case TK_COLON:       return "TK_COLON";
        case TK_ESCR:        return "TK_ESCR";
        case 0:              return "YYEOF";
        default:             return "TOKEN_DESCONOCIDO";
    }
}

int main(int argc, char *argv[]) {
    FILE *archivo = NULL;
    if (argc > 1) {
        archivo = fopen(argv[1], "r");
        if (!archivo) {
            fprintf(stderr, "Error al abrir archivo de prueba: %s\n", argv[1]);
            return 1;
        }
    } else {
        printf("Leyendo desde entrada estándar (o archivo no especificado)...\n");
    }

    inicializar_lexico(archivo);

    printf("Iniciando análisis léxico...\n\n");
    int token;
    while ((token = yylex()) != 0) {
        printf("[Línea %3d] Token: %-15s (Código %3d) | Lexema: '%s'\n",
               yylineno, obtener_nombre_token(token), token, yytext);
    }
    printf("\n[Fin de archivo alcanzado correctamente]\n");

    imprimir_tabla_simbolos();

    if (archivo) {
        fclose(archivo);
    }
    return 0;
}
