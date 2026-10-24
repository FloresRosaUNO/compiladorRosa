#ifndef MATRIZ_TOKENS_H
#define MATRIZ_TOKENS_H

#ifndef ESTADOS
#define ESTADOS 24
#endif

#ifndef COLUMNAS
#define COLUMNAS 19
#endif

#define NO_DEVUELVE -1

/* Se consulta con [ultimo_estado][ultima_columna], es decir, la transicion que llevo a EF.
   NO_DEVUELVE = la transicion no cierra token.
   37 = error lexico (caracter '.' suelto, '!' sin '=', comentario sin cerrar).
   0  = fin de archivo en E0 (no hay mas tokens). */
static const int matriz_tokens[ESTADOS][COLUMNAS] = {
    /*     Let           Dig          =            +            -            *            /            <            >            !            (            )            {            }            ;            :            .            BLANCO       FDA */
    /* 0  */ { NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, 37,          NO_DEVUELVE, 0 },
    /* 1  */ { NO_DEVUELVE, NO_DEVUELVE, 14,          14,          14,          14,          14,          14,          14,          14,          14,          14,          14,          14,          14,          14,          14,          14,          14 },
    /* 2  */ { 15,          NO_DEVUELVE, 15,          15,          15,          15,          15,          15,          15,          15,          15,          15,          15,          15,          15,          15,          NO_DEVUELVE, 15,          15 },
    /* 3  */ { 16,          NO_DEVUELVE, 16,          16,          16,          16,          16,          16,          16,          16,          16,          16,          16,          16,          16,          16,          16,          16,          16 },
    /* 4  */ { 17,          17,          NO_DEVUELVE, 17,          17,          17,          17,          17,          17,          17,          17,          17,          17,          17,          17,          17,          17,          17,          17 },
    /* 5  */ { 18,          18,          18,          18,          18,          18,          18,          18,          18,          18,          18,          18,          18,          18,          18,          18,          18,          18,          18 },
    /* 6  */ { 19,          19,          19,          19,          19,          19,          19,          19,          19,          19,          19,          19,          19,          19,          19,          19,          19,          19,          19 },
    /* 7  */ { 20,          20,          20,          20,          20,          20,          20,          20,          20,          20,          20,          20,          20,          20,          20,          20,          20,          20,          20 },
    /* 8  */ { 21,          21,          21,          21,          21,          NO_DEVUELVE, 21,          21,          21,          21,          21,          21,          21,          21,          21,          21,          21,          21,          21 },
    /* 9  */ { 22,          22,          NO_DEVUELVE, 22,          22,          22,          22,          22,          22,          22,          22,          22,          22,          22,          22,          22,          22,          22,          22 },
    /* 10 */ { 23,          23,          NO_DEVUELVE, 23,          23,          23,          23,          23,          23,          23,          23,          23,          23,          23,          23,          23,          23,          23,          23 },
    /* 11 */ { 24,          24,          24,          24,          24,          24,          24,          24,          24,          24,          24,          24,          24,          24,          24,          24,          24,          24,          24 },
    /* 12 */ { 25,          25,          25,          25,          25,          25,          25,          25,          25,          25,          25,          25,          25,          25,          25,          25,          25,          25,          25 },
    /* 13 */ { 26,          26,          26,          26,          26,          26,          26,          26,          26,          26,          26,          26,          26,          26,          26,          26,          26,          26,          26 },
    /* 14 */ { 37,          37,          NO_DEVUELVE, 37,          37,          37,          37,          37,          37,          37,          37,          37,          37,          37,          37,          37,          37,          37,          37 },
    /* 15 */ { 27,          27,          27,          27,          27,          27,          27,          27,          27,          27,          27,          27,          27,          27,          27,          27,          27,          27,          27 },
    /* 16 */ { 28,          28,          28,          28,          28,          28,          28,          28,          28,          28,          28,          28,          28,          28,          28,          28,          28,          28,          28 },
    /* 17 */ { 29,          29,          29,          29,          29,          29,          29,          29,          29,          29,          29,          29,          29,          29,          29,          29,          29,          29,          29 },
    /* 18 */ { 30,          30,          30,          30,          30,          30,          30,          30,          30,          30,          30,          30,          30,          30,          30,          30,          30,          30,          30 },
    /* 19 */ { 31,          31,          31,          31,          31,          31,          31,          31,          31,          31,          31,          31,          31,          31,          31,          31,          31,          31,          31 },
    /* 20 */ { 32,          32,          32,          32,          32,          32,          32,          32,          32,          32,          32,          32,          32,          32,          32,          32,          32,          32,          32 },
    /* 21 */ { 34,          34,          34,          34,          34,          34,          34,          34,          34,          34,          34,          34,          34,          34,          34,          34,          34,          34,          34 },
    /* 22 */ { NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, 37 },
    /* 23 */ { NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, NO_DEVUELVE, 37 }
};

#endif
