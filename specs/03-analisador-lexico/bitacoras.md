# Bitácora — 03-analizador-lexico

| Fecha | Quién | Qué se hizo / decidió | Notas |
|-------|-------|-----------------------|-------|
|       |       |                       |       |

# Bitácora de Trabajo — Analizador Léxico DUOTIPO
**Fecha:** 1 de noviembre de 2026  
**Fase:** Analizador Léxico y Estructura Base del Compilador  
**Grupo:** E  

---

### 1. Revisión y actualización de especificaciones (`specs/`)
* **Alineación con las correcciones de cátedra:** Se adaptó la especificación técnica del analizador léxico a la plantilla ejemplo, formalizando el alcance bajo demanda de la función `int yylex(void)` conectada con GNU Bison.
* **Consolidación de decisiones de diseño (L1 a L6):**
(ya estaban tomadas en cuenta , lo que se hizo fue agregarlas a la especificacin en el espacio correspondiente )
  * Reconocimiento unificado de identificadores y palabras reservadas por el mismo camino del autómata (`E1`).
  * Truncado automático de identificadores a 20 caracteres y emisión de **Advertencia léxica** por `stderr`.
  * Verificación de constantes enteras con signo en complemento a dos de 32 bits (`[-2147483648, 2147483647]`) y números reales.
  * Descarte transparente de blancos y comentarios de bloque multilínea `/* ... */`.
  * Desacople total del retroceso de caracteres mediante la **matriz de unreads** (`ungetc`), simplificando drásticamente las funciones semánticas.
  * Detección y reporte estricto de fin de archivo prematuro (`EOF`) en comentarios abiertos indicando la línea de inicio.

---

### 2. Optimización del diseño del autómata y matrices
* **Unificación de acciones semánticas:** Se fusionaron las funciones de acumulación y control de longitud en una sola rutina (`f1` para identificadores y `f3` para números) que verifica el límite de caracteres al vuelo mientras lee, reduciendo el catálogo a solo 10 acciones elementales (`fn`, `f1` a `f8`, `fe`).
* **Estandarización de encabezados y nombres en español:**
  * Se definieron los encabezados de eventos con nombres claros: `LETRA (0)`, `DÍGITO (1)`, operadores, delimitadores, `BLANCO (17)`, `FDA (18)` y `OTRO (19)`.
  * Se incorporó en cada matriz la columna descriptiva del nombre del estado (`estado inicial`, `palabra`, `entero`, `real`, `asignacion (=)`, `comentario (/* )`, etc.).
* **RE-Generación de las 4 matrices fundamentales:**
  1. `nuevo_estado`: Matriz de transiciones de estados (0 a 23 y -1 para aceptación).
  2. `matriz_unreads`: Matriz de control de retroceso (`1` para reinsertar carácter, `0` para consumir).
  3. `matriz_tokens`: Mapeo exacto de los tokens y códigos a emitir hacia el parser al alcanzar el estado final.
  4. `matriz_acciones`: Matriz de llamadas a funciones semánticas por par `(estado, evento)`.

---

### 3. Implementación de código fuente (`src/`)
Se programaron y compilaron todos los componentes en C requeridos para la fase léxica:
* **`matriz_estados.h`**, **`matriz_unreads.h`**, **`matriz_tokens.h`** y **`matriz_acciones.h`**: Cabeceras con los enums y las matrices bidimensionales completamente comentadas fila por fila con el nombre del estado correspondiente.
* **`tabla_simbolos.h` y `tabla_simbolos.c`**: Implementación de la Tabla de Símbolos estática para almacenar identificadores (con truncado) y constantes numéricas, incluyendo la función para exportar la tabla a archivo.
* **`lexer.h` y `lexer.c`**: 
  * Función `get_evento(int c)` con evaluación segura de `EOF` y clasificación en las 20 columnas.
  * Implementación de las acciones semánticas optimizadas y emisión de mensajes en español (*«Advertencia léxica en línea X...»* y *«Error léxico en línea X...»*).
  * Bucle principal de `yylex()` gobernado 100% por matrices, devolviendo tokens uno a uno.
* **`main_test.c` y `Makefile`**: Driver de ejecución para pruebas independientes del analizador léxico y script automatizado de compilación con `gcc`.

---

### 4. Construcción de nueva batería de pruebas (`tests/`)
Se diseñaron 5 archivos de prueba abarcando todas las categorías exigidas por la cátedra:
* **`prueba1_valida.txt` (`tests/correctos/`):** Caso completo con funciones, conversiones de tipo (`aReal`), bucles `mientras`, sentencias `segun`/`caso` y comentarios descartados correctamente.
* **`prueba2_advertencia_id.txt` (`tests/errores-lexicos-sintacticos/`):** Identificador de más de 20 caracteres que dispara la advertencia léxica y se almacena truncado.
* **`prueba3_error_comentario.txt` (`tests/errores-lexicos-sintacticos/`):** Comentario multilínea sin cerrar que alcanza `EOF`, reportando la línea exacta donde inició.
* **`prueba4_error_caracteres.txt` (`tests/errores-lexicos-sintacticos/`):** Operador `!` sin su correspondiente `=` y símbolos inválidos fuera del alfabeto (`@`).
* **`prueba5_error_rango_entero.txt` (`tests/errores-lexicos-sintacticos/`):** Constante entera que excede el límite de 32 bits con signo.

---

### 5. Definición de la arquitectura del repositorio
Se mapeó formalmente la distribución de carpetas para los próximos entregables:
* `docs/`: Entregables formales y bitácoras.
* `specs/`: Especificaciones de diseño, tabla de símbolos, léxico y el futuro sintáctico.
* `src/`: Código fuente del compilador (`.c` y `.h`).
* `tests/`: Casos organizados en `correctos/`, `errores-lexicos-sintacticos/` y `errores-semanticos/`.
* `out/`: Salidas generadas (`ts.txt`, tercetos, `.asm`).
