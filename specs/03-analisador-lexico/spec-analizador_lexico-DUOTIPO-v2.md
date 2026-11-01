### Spec — Analizador léxico de DUOTIPO
**Grupo:** E · **Lenguaje de implementación:** C · **Depende de:** specs/01-diseno/specDUOTIPO1.md · **Produce:** src/lexico/

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
| **L1** | **Palabras reservadas** | Se reconocen mediante el mismo autómata que los identificadores (camino a través del estado E1). Al cerrarse el lexema, la acción semántica `f_fin_id` consulta una tabla de búsqueda con las palabras reservadas del lenguaje (`entero`, `real`, `funcion`, `principal`, `retornar`, `segun`, `caso`, `defecto`, `mientras`, `aEntero`, `aReal`, `escribir`, `y`, `o`). Si coincide, devuelve el token reservado correspondiente; de lo contrario, devuelve `TK_ID` y lo incorpora a la Tabla de Símbolos. |
| **L2** | **Límite de longitud y truncado de ID al vuelo** | Longitud máxima de 20 caracteres. Se controla directamente en la función de acumulación `f_acumular_id`: si la longitud ya alcanzó 20 caracteres, **no acumula más** (trunca automáticamente) y enciende una bandera para emitir un *Warning Léxico* por stderr indicando la línea (`yylineno`). De este modo no se necesita una verificación posterior separada. |
| **L3** | **Constantes numéricas y rangos de 32 bits** | Las constantes enteras (`TK_NUM_INT`) se representan en complemento a dos de 32 bits (`-2147483648` a `2147483647`). Las constantes reales (`TK_NUM_REAL`) contienen punto decimal con formato `dígito.dígito`. La acumulación en `f_acumular_num` controla los caracteres leídos; al cerrar en `f_fin_num`, si una constante entera desborda el rango de 32 bits, se informa un *Error Léxico* y se almacena en la Tabla de Símbolos. |
| **L4** | **Comentarios y blancos** | Los espacios (`' '`), tabuladores (`'\t'`), retornos de carro (`'\r'`), saltos de línea (`'\n'`) y comentarios multilínea de bloque `/* ... */` (estados E22 y E23) son consumidos y descartados por el autómata retornando al estado E0 sin emitir ningún token hacia el sintáctico. Cada `\n` incrementa el contador global de líneas `yylineno`. |
| **L5** | **Retroceso de carácter (*Unread*)** | Se desacopla completamente de las acciones semánticas y se gobierna mediante la matriz `unreads[ESTADO][EVENTO]`. Si `unreads[e][ev] == 1`, el analizador léxico ejecuta `ungetc(c, yyin)` devolviendo el delimitador al flujo de entrada para no perder el carácter que delimitó el token actual. Esto permite que las acciones de fin de token (`f_fin_id`, `f_fin_num`) no tengan que gestionar lecturas ni deslecturas. |
| **L6** | **Manejo de errores léxicos en comentarios** | Si se alcanza el fin de archivo (`EOF`) mientras se encuentra abierto un comentario de bloque (estados E22 o E23), se emite un *Error Léxico* señalando la línea donde inició el comentario y no un error sintáctico. Si se lee un carácter `!` no seguido de `=`, se deslee el delimitador y se reporta *Error Léxico* ("Se esperaba '=' tras '!'"). Todo carácter ajeno al alfabeto emite *Error Léxico* y es descartado volviendo a E0. |

--------------------------------------------------------------------------------

#### 3. Eventos (columnas de las matrices)

La función auxiliar `get_evento(int c)` mapea el carácter leído del archivo fuente a un índice entero de columna (de 0 a 19):

| Col | Evento | Símbolo | Caracteres incluidos |
|:---:|:---|:---:|:---|
| **0** | Letra  | `L` | `a-z`, `A-Z`, `_` |
| **1** | Dígito | `D` | `0-9` |
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

#### 4. Estados del Autómata

| Estado | Tipo | Descripción |
|:---:|:---:|:---|
| **E0** | Inicial | Consume separadores o inicia el reconocimiento de lexemas. |
| **E1** | Palabra | Leyendo identificador o palabra reservada. |
| **E2** | Número | Leyendo constante entera. |
| **E3** | Real | Leyendo parte decimal de una constante real (tras leer `.`). |
| **E4** | Asignación | Leyó `=`; puede ser asignación `=` o pasar a igualdad `==`. |
| **E5** | Suma | Leyó `+`; cierre de token simple. |
| **E6** | Resta | Leyó `-`; cierre de token simple. |
| **E7** | Multiplicación | Leyó `*`; cierre de token simple. |
| **E8** | División | Leyó `/`; puede ser división `/` o inicio de comentario `/*`. |
| **E9** | Menor | Leyó `<`; puede ser `<` o menor igual `<=`. |
| **E10** | Mayor| Leyó `>`; puede ser `>` o mayor igual `>=`. |
| **E11** | Menor o igual | Leyó `<=` (operador relacional compuesto). |
| **E12** | Mayor o igual | Leyó `>=` (operador relacional compuesto). |
| **E13** | Asignación | Leyó `==` (operador relacional compuesto). |
| **E14** | Posible Distinto | Leyó `!`; espera obligatoriamente `=` para formar `!=`. |
| **E15** | Distinto | Leyó `!=` (operador relacional compuesto). |
| **E16** | Paréntesis apertura | Leyó `(`; delimitador simple. |
| **E17** | Paréntesis cierre| Leyó `)`; delimitador simple. |
| **E18** | Llave apertura | Leyó `{`; delimitador simple. |
| **E19** | Lave cierre | Leyó `}`; delimitador simple. |
| **E20** | Delimitador | Leyó `;`; delimitador simple. |
| **E21** | En proceso | Leyó `:`; delimitador simple. |
| **E22** | Comentario | Dentro del cuerpo de comentario de bloque `/* ...`. |
| **E23** | Posible cierre comentario | Leyó `*` dentro del comentario; esperando `/` para cerrar bloque. |
| **EF (-1)** | Final | Estado de aceptación. Determina el retorno de token hacia Bison. |

--------------------------------------------------------------------------------

#### 5. Acciones Semánticas (optimizadas y consolidadas)

Gracias a la integración del control de longitud durante la acumulación y al uso de la matriz `unreads`, la cantidad de funciones se reduce al mínimo esencial:

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

| Estado | Letra (0) | Digito (1) | = (2) | + (3) | - (4) | * (5) | / (6) | < (7) | > (8) | ! (9) | ( (10) | ) (11) | { (12) | } (13) | ; (14) | : (15) | . (16) | BL (17) | EOF (18) | OTRO (19) |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **E0** | 1 | 2 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 14 | 16 | 17 | 18 | 19 | 20 | 21 | -1 | 0 | -1 | 0 |
| **E1** | 1 | 1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E2** | -1 | 2 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | 3 | -1 | -1 | -1 |
| **E3** | -1 | 3 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E4** | -1 | -1 | 13 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E5** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E6** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E7** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E8** | -1 | -1 | -1 | -1 | -1 | 22 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E9** | -1 | -1 | 11 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E10** | -1 | -1 | 12 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E11** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E12** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E13** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E14** | -1 | -1 | 15 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E15** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E16** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E17** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E18** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E19** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E20** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E21** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| **E22** | 22 | 22 | 22 | 22 | 22 | 23 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | -1 | 22 |
| **E23** | 22 | 22 | 22 | 22 | 22 | 23 | 0 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | -1 | 22 |

--------------------------------------------------------------------------------

#### 7. Matriz de Acciones Semánticas (`acciones[24][20]`)

| Estado | Letra (0) | Dígito (1) | = (2) | + (3) | - (4) | * (5) | / (6) | < (7) | > (8) | ! (9) | ( (10) | ) (11) | { (12) | } (13) | ; (14) | : (15) | . (16) | BL (17) | EOF (18) | OTRO (19) |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **E0** | f1 | f3 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | fe | f6 | fn | fe |
| **E1** | f1 | f1 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 |
| **E2** | f4 | f3 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f3 | f4 | f4 | f4 |
| **E3** | f4 | f3 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 |
| **E4** | fn | fn | f5 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E5** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E6** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E7** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E8** | fn | fn | fn | fn | fn | f7 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E9** | fn | fn | f5 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E10** | fn | fn | f5 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E11** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E12** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E13** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E14** | fe | fe | f5 | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe |
| **E15** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E16** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E17** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E18** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E19** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E20** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E21** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| **E22** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | f6 | fe | fn |
| **E23** | fn | fn | fn | fn | fn | fn | f8 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | f6 | fe | fn |

--------------------------------------------------------------------------------

#### 8. Tabla de Unreads (`unreads[24][20]`)

Un valor `1` indica que se debe reinsertar el carácter leído en el buffer de entrada con `ungetc(c, yyin)`. Un valor `0` indica que el carácter pertenece al token o fue consumido normalmente.

| Estado | Letra (0) | Dígito (1) | = (2) | + (3) | - (4) | * (5) | / (6) | < (7) | > (8) | ! (9) | ( (10) | ) (11) | { (12) | } (13) | ; (14) | : (15) | . (16) | BL (17) | EOF (18) | OTRO (19) |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **E0** | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| **E1 (ID)** | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E2 (INT)** | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 | 0 | 1 |
| **E3 (REAL)**| 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E4 (=)** | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E5 (+)** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E6 (-)** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E7 (*)** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E8 (/)** | 1 | 1 | 1 | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E9 (<)** | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E10 (>)** | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E11 (<=)**| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E12 (>=)**| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E13 (==)**| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E14 (!)** | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E15 (!=)**| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E16 (()** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E17 ())** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E18 ({)** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E19 (})** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E20 (;)** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E21 (:)** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| **E22 (/*)**| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| **E23 (***)| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

--------------------------------------------------------------------------------

#### 9. Matriz / Mapeo de Tokens por Estado Final

Cuando el autómata alcanza un estado de aceptación `EF (-1)`, el token retornado hacia Bison depende del estado previo en el que se encontraba la máquina:

| Estado previo | Token retornado | Código / Constante | Descripción |
|:---:|:---:|:---:|:---|
| **E1** | `TK_ID` o palabra reservada | `314` o `301..313, 335` | Identificador o palabra reservada validada por `f2`. |
| **E2** | `TK_NUM_ENT` | `315` | Constante entera de 32 bits. |
| **E3** | `TK_NUM_REAL` | `316` | Constante real de 32 bits |
| **E4** | `TK_ASIGN` | `317` | Operador de asignación `=`. |
| **E5** | `TK_SUMA` | `318` | Operador aritmético suma `+`. |
| **E6** | `TK_RESTA` | `319` | Operador aritmético resta `-`. |
| **E7** | `TK_MULT` | `320` | Operador aritmético multiplicación `*`. |
| **E8** | `TK_DIV` | `321` | Operador aritmético división `/`. |
| **E9** | `TK_MENOR` | `322` | Operador de comparación menor `<`. |
| **E10** | `TK_MAYOR` | `323` | Operador de comparación mayor `>`. |
| **E11** | `TK_MENORIGUAL` | `324` | Operador relacional menor o igual `<=`. |
| **E12** | `TK_MAYORIGUAL` | `325` | Operador relacional mayor o igual `>=`. |
| **E13** | `TK_EQUIV` | `326` | Operador relacional asignación `==`. |
| **E15** | `TK_DISTIN` | `327` | Operador relacional distinto `!=`. |
| **E16** | `TK_ABREPAREN` | `328` | Delimitador `(`. |
| **E17** | `TK_CIERRPAREN` | `329` | Delimitador `)`. |
| **E18** | `TK_ABRELLAV` | `330` | Delimitador `{`. |
| **E19** | `TK_CIERRLLAV` | `331` | Delimitador `}`. |
| **E20** | `TK_FINSENTEN` | `332` | Delimitador de fin de sentencia `;`. |
| **E21** | `TK_SEPACASO` | `334` | Separador de caso o defecto `:`. |
| **E0 (con EOF)** | `0` (`YYEOF`) | `0` | Fin de archivo alcanzado correctamente en estado inicial. |

--------------------------------------------------------------------------------

#### 10. Errores que emite esta fase

| Código | Condición | Mensaje emitido | Recuperación |
|:---:|:---|:---|:---|
| **ERR_LEX_01** | Carácter no perteneciente al alfabeto de DUOTIPO (`OTRO` o `.` aislado en E0). | `Error léxico en línea %d: Carácter '%c' no válido en el lenguaje.` | Se ignora el carácter no válido y el autómata vuelve al estado E0. |
| **ERR_LEX_02** | Operador `!` seguido de un carácter distinto de `=`. | `Error léxico en línea %d: Carácter '!' inesperado. Se esperaba '=' para formar '!='.` | Se aplica unread sobre el delimitador, se reporta el error y vuelve a E0. |
| **ERR_LEX_03** | Fin de archivo inesperado (`EOF`) dentro del cuerpo de un comentario multilínea `/* ...`. | `Error léxico: Fin de archivo inesperado. Comentario abierto en línea %d no fue cerrado.` | Se interrumpe la ejecución del analizador léxico reportando código de error. |
| **ERR_LEX_04** | Constante entera fuera del rango representable de 32 bits (`[-2147483648, 2147483647]`). | `Error léxico en línea %d: Constante entera '%s' fuera de rango (32 bits con signo).` | Se almacena el valor saturado o truncado y se continúa el análisis. |
| **ATENCION_LEX_01** | Identificador que excede el límite de 20 caracteres. | `Atención léxico en línea %d: Identificador '%s' supera el límite de 20 caracteres. Se truncará a '%s'.` | El identificador se trunca en la acumulación a 20 caracteres y se almacena truncado en la Tabla de Simbolos. |

--------------------------------------------------------------------------------

#### 11. Trazas de verificación

##### Traza 1: Reconocimiento de identificador y operador `+` (`contador + 5`)
| Paso | Estado | Carácter leído | Evento | Acción | Nuevo estado | Unread | Token devuelto |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| 1 | 0 | `'c'` | `L` (0) | `f1` (acumular ID) | 1 | 0 | — |
| 2 | 1 | `'o'` | `L` (0) | `f1` (acumular ID) | 1 | 0 | — |
| ... | 1 | `'r'` | `L` (0) | `f1` (acumular ID) | 1 | 0 | — |
| 3 | 1 | `' '` | `BL` (17) | `f2` (fin ID) | -1 (EF) | 1 | `TK_ID` ("contador") |
| 4 | 0 | `' '` | `BL` (17) | `f6` (consumir BL) | 0 | 0 | — |
| 5 | 0 | `'+'` | `+` (3) | `f5` (acumular op) | 5 | 0 | — |
| 6 | 5 | `' '` | `BL` (17) | `fn` | -1 (EF) | 1 | `TK_PLUS` (`+`) |

##### Traza 2: Consumo silencioso de comentario multilínea (`/* comentario */`)
| Paso | Estado | Carácter leído | Evento | Acción | Nuevo estado | Unread | Token devuelto |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| 1 | 0 | `'/'` | `/` (6) | `f5` (acumular op) | 8 | 0 | — |
| 2 | 8 | `'*'` | `*` (5) | `f7` (inicio comentario) | 22 | 0 | — |
| 3 | 22 | `'c'` | `L` (0) | `fn` | 22 | 0 | — |
| 4 | 22 | `'*'` | `*` (5) | `fn` | 23 | 0 | — |
| 5 | 23 | `'/'` | `/` (6) | `f8` (fin comentario) | 0 | 0 | — (sigue en E0) |

--------------------------------------------------------------------------------

#### 12. Casos de prueba de esta fase

| Entrada | Salida esperada | Qué verifica |
|---|---|---|
| `entero var_1 = 45;` | `TK_INT`, `TK_ID("var_1")`, `TK_ASSIGN`, `TK_NUM_INT(45)`, `TK_SEMI` | Identificadores válidos, palabras reservadas y constantes enteras. |
| `identificador_demasiado_largo_para_el_lenguaje = 10;` | *Warning léxico en línea 1* (truncado a 20 caracteres) + `TK_ID("identificador_demas")`, `TK_ASSIGN`, `TK_NUM_INT(10)`, `TK_SEMI` | Límite máximo de longitud de identificadores y truncado al vuelo (L2). |
| `a <= b y c != d;` | `TK_ID("a")`, `TK_LE`, `TK_ID("b")`, `TK_AND`, `TK_ID("c")`, `TK_NEQ`, `TK_ID("d")`, `TK_SEMI` | Operadores relacionales compuestos y conectores lógicos reservadas. |
| `/* bloque sin cerrar` | *Error léxico: Fin de archivo inesperado en comentario de bloque abierto en línea 1.* | Manejo correcto de EOF dentro de comentarios sin pasar al parser sintáctico. |
| `!a` | *Error léxico en línea 1: Carácter '!' inesperado* + `TK_ID("a")` | Manejo de exclamación no seguida de igual. |
