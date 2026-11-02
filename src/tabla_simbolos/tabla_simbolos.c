#include <stdio.h>
#include <string.h>
#include "tabla_simbolos.h"

EntradaTS tabla_simbolos[MAX_SIMBOLOS];
int cant_simbolos = 0;

void inicializar_tabla_simbolos(void) {
    cant_simbolos = 0;
}

int buscar_simbolo(const char *nombre) {
    for (int i = 0; i < cant_simbolos; i++) {
        if (strcmp(tabla_simbolos[i].nombre, nombre) == 0) {
            return i;
        }
    }
    return -1;
}

int insertar_simbolo(const char *nombre, int token, const char *tipo, const char *valor) {
    int idx = buscar_simbolo(nombre);
    if (idx != -1) {
        return idx;
    }
    if (cant_simbolos >= MAX_SIMBOLOS) {
        fprintf(stderr, "Error: Tabla de Símbolos llena.\n");
        return -1;
    }

    strncpy(tabla_simbolos[cant_simbolos].nombre, nombre, MAX_LEXEMA - 1);
    tabla_simbolos[cant_simbolos].nombre[MAX_LEXEMA - 1] = '\0';
    tabla_simbolos[cant_simbolos].token = token;
    
    if (tipo) {
        strncpy(tabla_simbolos[cant_simbolos].tipo, tipo, 31);
        tabla_simbolos[cant_simbolos].tipo[31] = '\0';
    } else {
        tabla_simbolos[cant_simbolos].tipo[0] = '\0';
    }

    if (valor) {
        strncpy(tabla_simbolos[cant_simbolos].valor, valor, MAX_LEXEMA - 1);
        tabla_simbolos[cant_simbolos].valor[MAX_LEXEMA - 1] = '\0';
    } else {
        tabla_simbolos[cant_simbolos].valor[0] = '\0';
    }

    tabla_simbolos[cant_simbolos].longitud = strlen(nombre);
    return cant_simbolos++;
}

void imprimir_tabla_simbolos(void) {
    printf("\n================================ TABLA DE SIMBOLOS ================================\n");
    printf("%-4s | %-25s | %-12s | %-10s | %-15s\n", "Pos", "Nombre / Lexema", "Token", "Tipo", "Valor");
    printf("-----------------------------------------------------------------------------------\n");
    for (int i = 0; i < cant_simbolos; i++) {
        printf("%-4d | %-25s | %-12d | %-10s | %-15s\n",
               i,
               tabla_simbolos[i].nombre,
               tabla_simbolos[i].token,
               tabla_simbolos[i].tipo,
               tabla_simbolos[i].valor);
    }
    printf("===================================================================================\n\n");
}
