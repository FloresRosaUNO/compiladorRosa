#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "matriz_estados.h"
#include "matriz_tokens.h"
#include "matriz_acciones.h"
#include "matriz_unreads.h"
#include "/tabla_simbolos/tabla_simbolos.h"

#define MAX_BUFFER_LEXICO 1024
#define LIMITE_LONGITUD_ID 20

/* Variables globales del analizador léxico */
extern FILE *yyin;
extern int yylineno;
extern char yytext[MAX_BUFFER_LEXICO];

/* Funciones principales */
void inicializar_lexico(FILE *archivo);
int get_evento(int c);
int yylex(void);

#endif /* LEXER_H */
