## Spec — Analizador Sintáctico de DUOTIPO
**Grupo:** E · **Lenguaje de implementación:** C · **Depende de:** `specs/01-diseno/spec.md`, `src/lexico/lexer.h`, `src/tabla_simbolos/tabla_simbolos.h` · **Produce:** `src/sintactico/parser.y` (y código generado `parser.tab.c`, `parser.tab.h`)

--------------------------------------------------------------------------------

### 1. Alcance e interfaz
El analizador sintáctico tiene como objetivo exclusivo en esta etapa **determinar si el programa fuente de entrada es sintácticamente válido o no**, contrastándolo contra la Gramática Independiente del Contexto (GIC) de DUOTIPO.

Es generado a partir de una especificación de **GNU Bison** y expone como interfaz principal la función `yyparse()`:

```c
int yyparse(void);
```

* **Flujo guiado por demanda:** Bison **no** utiliza un analizador léxico generado por Flex. Invoca en forma directa y bajo demanda la función `yylex()` implementada en C nuestro (`src/lexico/lexer.c`), recibiendo **un token por llamada**.
* **Resultado del análisis:**
  * Si la entrada pertenece al lenguaje definido por la GIC: la compilación concluye con éxito, informando por consola y retornando código `1`.
  * Si la entrada contiene una discrepancia gramatical: se interrumpe el análisis, se notifica el error sintáctico con número de línea mediante `yyerror()` y el proceso retorna código `0`.
* **Delimitación de alcance:** En esta etapa **no se generan tercetos, árboles sintácticos ni código intermedio**. Las acciones semánticas asociadas a las reducciones se reservan para la siguiente fase de compilación.

--------------------------------------------------------------------------------

### 2. Decisiones de diseño de esta fase

| # | Decisión | Valor y Justificación |
|---|---|---|
| **S1** | **Herramienta generadora** | **GNU Bison** configurado para generar un autómata de pila ascendente **LALR(1)** en lenguaje C. |
| **S2** | **Alineación de tokens** | Los códigos numéricos declarados mediante `%token` coinciden exactamente con los asignados en `matriz_tokens.h` y en el diseño del lenguaje (valores `301` a `335`, y `0` para fin de archivo `YYEOF`). |
| **S3** | **Precedencia y asociatividad** | Se declaran explícitamente en Bison para evitar ambigüedades en expresiones aritméticas y lógicas: asociatividad a izquierda (`%left`) para conectores lógicos (`o`, `y`), operadores relacionales, y operadores aritméticos (`+`, `-`, `*`, `/`). |
| **S4** | **Menos unario** | El operador menos unario (por ejemplo `-5` o `-a`) se resuelve con la directiva `%prec UMINUS`, otorgándole la máxima precedencia sobre la resta binaria. |
| **S5** | **Depuración y trazabilidad** | Se habilita la directiva `%debug` en Bison y la variable `yydebug = 1` en el ejecutable para poder auditar los desplazamientos (*shift*) y reducciones (*reduce*) en la consola. |
| **S6** | **Manejo de errores sintácticos** | La función `yyerror()` captura cualquier falla gramatical reportando la línea exacta tomada de la variable global `yylineno` mantenida por el analizador léxico. |

--------------------------------------------------------------------------------

### 3. Terminales (Tokens reconocidos)

Todos los terminales son provistos por `yylex()` con los siguientes identificadores y códigos:

| Categoría | Directiva Bison | Código | Lexema / Sintaxis |
|---|---|:---:|---|
| **Palabras reservadas** | `%token TK_INT` | 301 | `entero` |
| | `%token TK_REAL` | 302 | `real` |
| | `%token TK_FUNCTION` | 303 | `funcion` |
| | `%token TK_MAIN` | 304 | `principal` |
| | `%token TK_RETURN` | 305 | `retornar` |
| | `%token TK_SWITCH` | 306 | `segun` |
| | `%token TK_CASE` | 307 | `caso` |
| | `%token TK_DEFAULT` | 308 | `defecto` |
| | `%token TK_WHILE` | 309 | `mientras` |
| | `%token TK_TOINT` | 310 | `aEntero` |
| | `%token TK_TOREAL` | 311 | `aReal` |
| | `%token TK_ESCR` | 335 | `escribir` |
| **Conectores lógicos** | `%token TK_AND` | 312 | `y` |
| | `%token TK_OR` | 313 | `o` |
| **Identificadores y literales**| `%token TK_ID` | 314 | Identificador (máx 20 caracteres) |
| | `%token TK_NUM_INT` | 315 | Constante entera (32 bits con signo) |
| | `%token TK_NUM_REAL` | 316 | Constante real (`digito.digito`) |
| **Operadores** | `%token TK_ASSIGN` | 317 | `=` |
| | `%token TK_PLUS` | 318 | `+` |
| | `%token TK_MINUS` | 319 | `-` |
| | `%token TK_MULT` | 320 | `*` |
| | `%token TK_DIV` | 321 | `/` |
| | `%token TK_LT` | 322 | `<` |
| | `%token TK_GT` | 323 | `>` |
| | `%token TK_LE` | 324 | `<=` |
| | `%token TK_GE` | 325 | `>=` |
| | `%token TK_EQ` | 326 | `==` |
| | `%token TK_NEQ` | 327 | `!=` |
| **Delimitadores y puntuación**| `%token TK_LPAREN` | 328 | `(` |
| | `%token TK_RPAREN` | 329 | `)` |
| | `%token TK_LBRACE` | 330 | `{` |
| | `%token TK_RBRACE` | 331 | `}` |
| | `%token TK_SEMI` | 332 | `;` |
| | `%token TK_COLON` | 334 | `:` |
| **Fin de archivo** | `YYEOF` | 0 | Retorno de fin de entrada |

--------------------------------------------------------------------------------

### 4. Precedencia y Asociatividad en Bison

Para resolver ambigüedades sin alterar la estructura gramatical, se declaran los siguientes niveles de precedencia (de menor a mayor):

```yacc
%left TK_OR
%left TK_AND
%left TK_EQ TK_NEQ TK_LT TK_LE TK_GT TK_GE
%left TK_PLUS TK_MINUS
%left TK_MULT TK_DIV
%right UMINUS
```

--------------------------------------------------------------------------------

### 5. Gramática Formal de DUOTIPO

#### 5.1 Axioma y Estructura Global
```yacc
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
```

#### 5.2 Declaraciones de Variables y Tipos
```yacc
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
```

#### 5.3 Declaración de Funciones
*DUOTIPO prohíbe recursión y no admite parámetros en funciones; las funciones poseen tipo y retorno obligatorio.*

```yacc
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
```

#### 5.4 Sentencias y Estructuras de Control
```yacc
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
```

#### 5.5 Condiciones y Expresiones Lógicas
```yacc
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
```

#### 5.6 Expresiones Aritméticas y Conversión
```yacc
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
```

--------------------------------------------------------------------------------

### 6. Errores y Diagnóstico de la Fase

| Código | Condición | Mensaje emitido por `stderr` | Acción del compilador |
|:---:|:---|:---|:---|
| **ERR_SIN_01** | Discrepancia sintáctica respecto a las reglas de la GIC (tokens fuera de orden, falta de delimitadores, etc.). | `Error sintactico (P01): syntax error en linea %d` | Detiene el análisis, emite el diagnóstico y finaliza con código `0`. |

--------------------------------------------------------------------------------

### 7. Criterios de Aceptación y Casos de Prueba

| Caso de prueba | Ubicación | Entrada de prueba | Salida y comportamiento esperado |
|---|---|---|---|
| **CP_SIN_01** | `tests/correctos/prueba1_valida.txt` | Código con declaraciones globales, función, bloque `principal`, `mientras`, `segun` y expresiones válidas. | **Éxito (código 1)**. Imprime mensaje de compilación exitosa y genera `out/ts.txt`. |
| **CP_SIN_02** | `tests/errores-lexicos-sintacticos/prueba_error_falta_semi.txt` | Omisión de `;` al final de una asignación o declaración: `entero a = 10` | **Falla (código 0)**. Emite por `stderr`: `Error sintactico (P01): syntax error en linea X`. |
| **CP_SIN_03** | `tests/errores-lexicos-sintacticos/prueba_error_llave.txt` | Bloque `principal` o `mientras` con llave desbalanceada `{ ...` sin cerrar `}`. | **Falla (código 0)**. Emite diagnóstico de error sintáctico indicando línea del token inesperado o EOF. |
| **CP_SIN_04** | `tests/errores-lexicos-sintacticos/prueba_error_estructura.txt` | Declaraciones de variables después de sentencias ejecutables en el bloque principal. | **Falla (código 0)**. Falla sintáctica al violar el orden canónico `<decl_locales> <sentencias>`. |
