#ifndef TABLA_SIMBOLOS_H
#define TABLA_SIMBOLOS_H

#define MAX_LEXEMA 128
#define MAX_SIMBOLOS 1024

typedef struct {
    char nombre[MAX_LEXEMA];
    int token;
    char tipo[32];          /* "entero", "real", etc. */
    char valor[MAX_LEXEMA]; /* Para constantes */
    int longitud;
} EntradaTS;

extern EntradaTS tabla_simbolos[MAX_SIMBOLOS];
extern int cant_simbolos;

void inicializar_tabla_simbolos(void);
int insertar_simbolo(const char *nombre, int token, const char *tipo, const char *valor);
int buscar_simbolo(const char *nombre);
void imprimir_tabla_simbolos(void);

#endif /* TABLA_SIMBOLOS_H */
