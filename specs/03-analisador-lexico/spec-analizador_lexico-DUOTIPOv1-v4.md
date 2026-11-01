### Spec — Analizador léxico de DUOTIPO
**Grupo:** E · **Lenguaje de implementación:** C · **Depende de:** specs/01-diseno/specDUOTIPO1.md · **Produce:** src/lexico/ · escribe en la tabla de símbolos definida en specs/02-tabla-simbolos/spec.md

--------------------------------------------------------------------------------

#### 1. Alcance e interfaz
Función `yylex()` invocada por el analizador sintáctico (GNU Bison). Devuelve **un token por llamada**, como entero representativo del tipo de token (constantes `TK_*` o códigos definidos por Bison). No genera una pasada previa que devuelva una lista de tokens; el flujo es guiado por demanda.

```c
int yylex(void);
```

--------------------------------------------------------------------------------

#### 2. Decisiones propias de esta fase

| # | Decisión | Valor |
|---|---|---|
| **L1** | **Palabras reservadas** | Se reconocen mediante el mismo autómata que los identificadores (camino a través del estado E1). Al cerrarse el lexema, la acción semántica `f2` (`f_fin_id`) consulta una tabla de búsqueda (hash o búsqueda binaria) con las palabras reservadas del lenguaje (`entero`, `real`, `funcion`, `principal`, `retornar`, `segun`, `caso`, `defecto`, `mientras`, `aEntero`, `aReal`, `escribir`, `y`, `o`). Si coincide, devuelve el token reservado correspondiente; de lo contrario, devuelve `TK_ID` y lo incorpora a la Tabla de Símbolos. |
| **L2** | **Límite de longitud de ID** | Longitud máxima de 20 caracteres. Si el lexema supera los 20 caracteres, la acción `f1` trunca automáticamente el lexema en memoria (no acumula a partir del carácter 21) y activa una bandera. Al cerrarse el token, `f2` emite un *Advertencia Léxica* por stderr indicando la línea (`yylineno`), y se continúa con el token truncado. |
| **L3** | **Constantes numéricas y rangos** | Las constantes enteras (`TK_NUM_INT`) se representan en complemento a dos de 32 bits (`-2147483648` a `2147483647`). Las constantes reales (`TK_NUM_REAL`) contienen punto decimal con dígitos obligatorios antes y después (formato `dígito.dígito`). Si una constante entera excede el rango de 32 bits, se informa un *Error Léxico* indicando desbordamiento de rango. La acumulación se realiza con `f3` y el cierre y validación de rango con `f4`. |
| **L4** | **Comentarios y blancos** | Los espacios (`' '`), tabuladores (`'\t'`), retornos de carro (`'\r'`), saltos de línea (`'\n'`) y comentarios multilínea de bloque `/* ... */` (estados E22 y E23) son consumidos y descartados por el autómata retornando al estado E0 sin emitir ningún token hacia el sintáctico. Cada `\n` incrementa el contador global de líneas `yylineno` mediante `f6`. |
| **L5** | **Retroceso de carácter (*Unread*)** | Se desacopla completamente de las acciones semánticas y se gobierna mediante la matriz `unreads[ESTADO][EVENTO]`. Si `unreads[e][ev] == 1`, el analizador léxico ejecuta `ungetc(c, yyin)` devolviendo el delimitador al flujo de entrada para no perder el carácter que delimitó el token actual. |
| **L6** | **Manejo de errores léxicos en comentarios** | Si se alcanza el fin de archivo (`EOF`) mientras se encuentra abierto un comentario de bloque (estados E22 o E23), se emite un *Error Léxico* señalando la línea donde inició el comentario y no un error sintáctico. Si se lee un carácter `!` no seguido de `=`, se deslee el siguiente carácter y se reporta *Error Léxico* ("Se esperaba '=' tras '!'"). Todo carácter ajeno al alfabeto emite *Error Léxico* y es descartado volviendo a E0. |

--------------------------------------------------------------------------------

#### 3. Eventos (columnas de las matrices)

La función auxiliar `get_evento(int c)` mapea el carácter leído del archivo fuente a un índice entero de columna (de 0 a 19):

| Col | Evento | Símbolo | Caracteres incluidos |
|:---:|:---|:---:|:---|
| **0** | Letra / Guion bajo | `letra` | `a-z`, `A-Z`, `_` |
| **1** | Dígito | `digito` | `0-9` |
| **2** | Asignación / Igual | `=` | `=` |
| **3** | Suma | `+` | `+` |
| **4** | Resta | `-` | `-` |
| **5** | Multiplicación | `*` | `*` |
| **6** | Barra división | `/` | `/` |
| **7** | Menor | `<` | `<` |
| **8** | Mayor | `>` | `>` |
| **9** | Exclamación | `!` | `!` |
| **10** | Paréntesis izquierdo | `(` | `(` |
| **11** | Paréntesis derecho | `)` | `)` |
| **12** | Llave izquierda | `{` | `{` |
| **13** | Llave derecha | `}` | `}` |
| **14** | Punto y coma | `;` | `;` |
| **15** | Dos puntos | `:` | `:` |
| **16** | Punto decimal | `.` | `.` |
| **17** | Blanco / Separador | `BL` | Espacio (`' '`), tabulación (`'\t'`), `\r`, `\n` |
| **18** | Fin de archivo | `EOF` | Carácter `EOF` (`-1`) |
| **19** | No reconocido | `OTRO` | Cualquier otro carácter fuera del alfabeto de DUOTIPO |

--------------------------------------------------------------------------------

#### 4. Estados y Diagrama

| Estado | Nombre / Mnemónico en `.h` | Significado / Descripción |
|:---:|:---|:---|
| **E0** | `EST_INICIAL` | estado inicial |
| **E1** | `EST_ID` | palabra (identificador o palabra reservada) |
| **E2** | `EST_ENTERO` | entero (parte entera) |
| **E3** | `EST_REAL` | real (parte decimal tras leer punto) |
| **E4** | `EST_ASIG` | asignacion (=) |
| **E5** | `EST_SUMA` | suma (+) |
| **E6** | `EST_RESTA` | resta (-) |
| **E7** | `EST_MULT` | multiplicación (*) |
| **E8** | `EST_DIV` | división (/) |
| **E9** | `EST_MENOR` | menor (<) |
| **E10** | `EST_MAYOR` | mayor (>) |
| **E11** | `EST_MENOR_IGUAL` | menor o igual (<=) |
| **E12** | `EST_MAYOR_IGUAL` | mayor o igual (>=) |
| **E13** | `EST_IGUALDAD` | igualdad (==) |
| **E14** | `EST_POSIBLE_DIST` | posible distinto (!) |
| **E15** | `EST_DISTINTO` | distinto (!=) |
| **E16** | `EST_PAREN_ABRE` | abre paréntesis (() |
| **E17** | `EST_PAREN_CIERRA` | cierra paréntesis ()) |
| **E18** | `EST_LLAVE_ABRE` | llave apertura ({) |
| **E19** | `EST_LLAVE_CIERRA` | llave cierre (}) |
| **E20** | `EST_FIN_SENT` | fin sentencia (;) |
| **E21** | `EST_SEPARADOR` | separador (:) |
| **E22** | `EST_COMENTARIO` | comentario (/* cuerpo) |
| **E23** | `EST_POSIBLE_FIN_COMENT` | posible fin comentario (*) |
| **EF (-1)** | `EST_FINAL` | estado final de aceptación (retorna token hacia Bison) |

--------------------------------------------------------------------------------

#### 5. Acciones semánticas

| Acción | Nombre de función | Qué hace |
|:---:|:---:|:---|
| **fn** | `f_nada` | No realiza ninguna operación (ignora o descarta el carácter actual). |
| **f1** | `f_acumular_id` | **Inicializa o acumula en ID**: si el buffer está vacío, lo resetea e inserta el primer carácter. Si ya tiene contenido, verifica: si `longitud < 20` almacena el carácter; si `longitud >= 20` **trunca** (no agrega más) y activa la bandera de warning léxico. |
| **f2** | `f_fin_id` | **Cierre de ID**: Agrega `\0` al buffer. Si la bandera de warning fue activada, emite el warning léxico con el número de línea. Busca en la tabla de palabras reservadas (devuelve `TK_INT`, `TK_WHILE`, etc.); si no es reservada, inserta/recupera de la Tabla de Símbolos y prepara `TK_ID`. |
| **f3** | `f_acumular_num` | **Inicializa o acumula en Constante Numérica**: si el buffer está vacío, lo limpia e ingresa el primer dígito. En lecturas subsiguientes concatena dígitos o el punto decimal (`.`), validando longitud del buffer. |
| **f4** | `f_fin_num` | **Cierre de Constante**: Agrega `\0`. Si es entera, valida rango de 32 bits (`[-2147483648, 2147483647]`); si desborda, emite error léxico. Inserta la constante en la Tabla de Símbolos y prepara `TK_NUM_INT` o `TK_NUM_REAL`. |
| **f5** | `f_acumular_op` | **Almacena operador o delimitador**: Almacena el primer carácter en E0, o concatena el segundo carácter para operadores dobles (`==`, `<=`, `>=`, `!=`). |
| **f6** | `f_contar_linea` | Si el carácter leído es `\n`, incrementa el contador global de línea `yylineno`. |
| **f7** | `f_inicio_coment` | Registra la línea de inicio del comentario de bloque (para reportar en caso de EOF prematuro). |
| **f8** | `f_fin_coment` | Concluye el comentario de bloque y limpia banderas de comentario. |
| **fe** | `f_error_lexico` | Registra y emite un mensaje de error léxico con número de línea actual y descripción del motivo. |

--------------------------------------------------------------------------------

#### 6. Matriz de Nuevos Estados (`nuevo_estado[24][20]`)

Valores: `0` a `23` para estados intermedios, `-1` para Estado Final de aceptación (`EF`).

| N° | Nombre de Estado | letra (0) | digito (1) | = (2) | + (3) | - (4) | * (5) | / (6) | < (7) | > (8) | ! (9) | ( (10) | ) (11) | { (12) | } (13) | ; (14) | : (15) | . (16) | BL (17) | EOF (18) | OTRO (19) |
|:---:|:---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **0** | `estado inicial` | 1 | 2 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 14 | 16 | 17 | 18 | 19 | 20 | 21 | -1 | 0 | -1 | 0 |
| **1** | `palabra` | 1 | 1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **2** | `entero` | -1 | 2 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | 3 | -1 | -1 | -1 |
| **3** | `real` | -1 | 3 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **4** | `asigancion (=)` | -1 | -1 | 13 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **5** | `suma (+)` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **6** | `resta (-)` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **7** | `multiplicación (*)` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **8** | `división (/)` | -1 | -1 | -1 | -1 | -1 | 22 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **9** | `menor (< )` | -1 | -1 | 11 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **10** | `mayor (>)` | -1 | -1 | 12 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **11** | `menor o igual (<=)` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **12** | `mayor o igual (>=)` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **13** | `igualdad (==)` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **14** | `posible distinto` | -1 | -1 | 15 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **15** | `distinto ( !=)` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **16** | `abre paréntesis` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **17** | `cierra paréntesis` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **18** | `llave apertura` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **19** | `llave cierre` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **20** | `fin sentencia (;)` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **21** | `separador (:)` | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **22** | `comentario (/* )` | 22 | 22 | 22 | 22 | 22 | 23 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | -1 | 22 |
| **23** | `posible fin comentario` | 22 | 22 | 22 | 22 | 22 | 23 | 0 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | -1 | 22 |

--------------------------------------------------------------------------------

#### 7. Matriz de Acciones Semánticas (`acciones[24][20]`)

| N° | Nombre de Estado | letra (0) | digito (1) | = (2) | + (3) | - (4) | * (5) | / (6) | < (7) | > (8) | ! (9) | ( (10) | ) (11) | { (12) | } (13) | ; (14) | : (15) | . (16) | BL (17) | EOF (18) | OTRO (19) |
|:---:|:---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **0** | `estado inicial` | f1 | f3 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | fe | f6 | fn | fe |
| **1** | `palabra` | f1 | f1 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 |
| **2** | `entero` | f4 | f3 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f3 | f4 | f4 | f4 |
| **3** | `real` | f4 | f3 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 |
| **4** | `asigancion (=)` | fn | fn | f5 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **5** | `suma (+)` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **6** | `resta (-)` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **7** | `multiplicación (*)` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **8** | `división (/)` | fn | fn | fn | fn | fn | f7 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **9** | `menor (< )` | fn | fn | f5 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **10** | `mayor (>)` | fn | fn | f5 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **11** | `menor o igual (<=)` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **12** | `mayor o igual (>=)` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **13** | `igualdad (==)` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **14** | `posible distinto` | fe | fe | f5 | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe |
| **15** | `distinto ( !=)` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **16** | `abre paréntesis` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **17** | `cierra paréntesis` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **18** | `llave apertura` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **19** | `llave cierre` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **20** | `fin sentencia (;)` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **21** | `separador (:)` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **22** | `comentario (/* )` | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | f6 | fe | fn |
| **23** | `posible fin comentario` | fn | fn | fn | fn | fn | fn | f8 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | f6 | fe | fn |

--------------------------------------------------------------------------------

#### 8. Tabla de Unreads (`unreads[24][20]`)

Un valor `1` indica que se debe reinsertar el carácter leído en el buffer de entrada con `ungetc(c, yyin)`. Un valor `0` indica que el carácter pertenece al token o fue consumido normalmente.

| N° | Nombre de Estado | letra (0) | digito (1) | = (2) | + (3) | - (4) | * (5) | / (6) | < (7) | > (8) | ! (9) | ( (10) | ) (11) | { (12) | } (13) | ; (14) | : (15) | . (16) | BL (17) | EOF (18) | OTRO (19) |
|:---:|:---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **0** | `estado inicial` | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| **1** | `palabra` | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **2** | `entero` | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 | 0 | 1 |
| **3** | `real` | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **4** | `asigancion (=)` | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **5** | `suma (+)` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **6** | `resta (-)` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **7** | `multiplicación (*)` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **8** | `división (/)` | 1 | 1 | 1 | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **9** | `menor (< )` | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **10** | `mayor (>)` | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **11** | `menor o igual (<=)` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **12** | `mayor o igual (>=)` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **13** | `igualdad (==)` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **14** | `posible distinto` | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **15** | `distinto ( !=)` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **16** | `abre paréntesis` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **17** | `cierra paréntesis` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **18** | `llave apertura` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **19** | `llave cierre` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **20** | `fin sentencia (;)` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **21** | `separador (:)` | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **22** | `comentario (/* )` | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| **23** | `posible fin comentario` | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

--------------------------------------------------------------------------------

#### 9. Errores que emite esta fase

| Código | Condición | Mensaje emitido | Recuperación |
|:---:|:---|:---|:---|
| **ERR_LEX_01** | Carácter no perteneciente al alfabeto de DUOTIPO (`OTRO` o `.` aislado en E0). | `Error léxico en línea %d: Carácter '%c' no válido en el lenguaje.` | Se ignora el carácter no válido y el autómata vuelve al estado E0. |
| **ERR_LEX_02** | Operador `!` seguido de un carácter distinto de `=`. | `Error léxico en línea %d: Carácter '!' inesperado. Se esperaba '=' para formar '!='.` | Se aplica unread sobre el delimitador, se reporta el error y vuelve a E0. |
| **ERR_LEX_03** | Fin de archivo inesperado (`EOF`) dentro del cuerpo de un comentario multilínea `/* ...`. | `Error léxico: Fin de archivo inesperado. Comentario abierto en línea %d no fue cerrado.` | Se interrumpe la ejecución del analizador léxico reportando código de error. |
| **ERR_LEX_04** | Constante entera fuera del rango representable de 32 bits (`[-2147483648, 2147483647]`). | `Error léxico en línea %d: Constante entera '%s' fuera de rango (32 bits con signo).` | Se almacena el valor saturado o truncado y se continúa el análisis. |
| **WARN_LEX_01** | Identificador que excede el límite de 20 caracteres. | `Warning léxico en línea %d: Identificador '%s' supera el límite de 20 caracteres. Se truncará a '%s'.` | El identificador se trunca a 20 caracteres y se almacena truncado en la TS. |

--------------------------------------------------------------------------------

#### 10. Traza de verificación

##### Traza 1: Reconocimiento de identificador y operador `+` (`contador + 5`)
| Paso | N° | Estado | Carácter leído | Evento | Acción | Nuevo estado | Unread | Token devuelto |
|:---:|:---:|:---|:---:|:---:|:---:|:---:|:---:|:---:|
| 1 | 0 | `estado inicial` | `'c'` | `letra` (0) | `f1` (acumular ID) | 1 (`palabra`) | 0 | — |
| 2 | 1 | `palabra` | `'o'` | `letra` (0) | `f1` (acumular ID) | 1 (`palabra`) | 0 | — |
| ... | 1 | `palabra` | `'r'` | `letra` (0) | `f1` (acumular ID) | 1 (`palabra`) | 0 | — |
| 3 | 1 | `palabra` | `' '` | `BL` (17) | `f2` (fin ID) | -1 (EF) | 1 | `TK_ID` ("contador") |
| 4 | 0 | `estado inicial` | `' '` | `BL` (17) | `f6` (contar/blanco) | 0 (`estado inicial`) | 0 | — |
| 5 | 0 | `estado inicial` | `'+'` | `+` (3) | `f5` (acumular op) | 5 (`suma (+)`) | 0 | — |
| 6 | 5 | `suma (+)` | `' '` | `BL` (17) | `fn` | -1 (EF) | 1 | `TK_PLUS` (`+`) |

##### Traza 2: Consumo silencioso de comentario multilínea (`/* comentario */`)
| Paso | N° | Estado | Carácter leído | Evento | Acción | Nuevo estado | Unread | Token devuelto |
|:---:|:---:|:---|:---:|:---:|:---:|:---:|:---:|:---:|
| 1 | 0 | `estado inicial` | `'/'` | `/` (6) | `f5` (acumular op) | 8 (`división (/)`) | 0 | — |
| 2 | 8 | `división (/)` | `'*'` | `*` (5) | `f7` (inicio comentario) | 22 (`comentario (/* )`) | 0 | — |
| 3 | 22 | `comentario (/* )` | `'c'` | `letra` (0) | `fn` | 22 (`comentario (/* )`) | 0 | — |
| 4 | 22 | `comentario (/* )` | `'*'` | `*` (5) | `fn` | 23 (`posible fin comentario`) | 0 | — |
| 5 | 23 | `posible fin comentario`| `'/'` | `/` (6) | `f8` (fin comentario) | 0 (`estado inicial`) | 0 | — (sigue en E0) |

--------------------------------------------------------------------------------

#### 11. Casos de prueba de esta fase

| Entrada | Salida esperada | Qué verifica |
|---|---|---|
| `entero var_1 = 45;` | `TK_INT`, `TK_ID("var_1")`, `TK_ASSIGN`, `TK_NUM_INT(45)`, `TK_SEMI` | Identificadores válidos, palabras reservadas y constantes enteras. |
| `identificador_demasiado_largo_para_el_lenguaje = 10;` | *Warning léxico en línea 1* (truncado a 20 caracteres) + `TK_ID("identificador_demas")`, `TK_ASSIGN`, `TK_NUM_INT(10)`, `TK_SEMI` | Límite máximo de longitud de identificadores (L2). |
| `a <= b y c != d;` | `TK_ID("a")`, `TK_LE`, `TK_ID("b")`, `TK_AND`, `TK_ID("c")`, `TK_NEQ`, `TK_ID("d")`, `TK_SEMI` | Operadores relacionales compuestos y conectores lógicos reservados. |
| `/* bloque sin cerrar` | *Error léxico: Fin de archivo inesperado. Comentario abierto en línea 1 no fue cerrado.* | Manejo correcto de EOF dentro de comentarios sin pasar al parser sintáctico. |
| `!a` | *Error léxico en línea 1: Carácter '!' inesperado* + `TK_ID("a")` | Manejo de exclamación no seguida de igual. |
