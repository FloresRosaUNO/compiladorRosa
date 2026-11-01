### Spec — Analizador léxico de DUOTIPO
**Grupo:** E · **Lenguaje de implementación:** C · **Depende de:** specs/01-diseno/specDUOTIPO1.md · **Produce:** src/lexer.c/ · escribe en la tabla de símbolos definida en specs/02-tabla-simbolos/spec.md

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
| **L2** | **Límite de longitud de ID** | Longitud máxima de 20 caracteres. Si el lexema supera los 20 caracteres, la acción `f1` trunca automáticamente el lexema en memoria (no acumula a partir del carácter 21) y activa una bandera. Al cerrarse el token, `f2` emite un *Advertencia Léxica* por stderr indicando número de línea (`yylineno`), y se continúa con el token truncado. |
| **L3** | **Constantes numéricas y rangos** | Las constantes enteras (`TK_NUM_INT`) se representan en complemento a dos de 32 bits (`-2147483648` a `2147483647`). Las constantes reales (`TK_NUM_REAL`) contienen punto decimal con dígitos obligatorios antes y después (formato `dígito.dígito`). Si una constante entera excede el rango de 32 bits, se informa un *Error Léxico* indicando desbordamiento de rango. La acumulación se realiza con `f3` y el cierre y validación de rango con `f4`. |
| **L4** | **Comentarios y blancos** | Los espacios (`' '`), tabuladores (`'\t'`), retornos de carro (`'\r'`), saltos de línea (`'\n'`) y comentarios multilínea de bloque `/* ... */` (estados E22 y E23) son consumidos y descartados por el autómata retornando al estado E0 sin emitir ningún token hacia el sintáctico. Cada `\n` incrementa el contador global de líneas `yylineno` mediante `f6`. |
| **L5** | **Retroceso de carácter (*Unread*)** | Se desacopla completamente de las acciones semánticas y se gobierna mediante la matriz `unreads[ESTADO][EVENTO]`. Si `unreads[e][ev] == 1`, el analizador léxico ejecuta `ungetc(c, yyin)` devolviendo el delimitador al flujo de entrada para no perder el carácter que delimitó el token actual. |
| **L6** | **Manejo de errores léxicos en comentarios** | Si se alcanza el fin de archivo (`EOF`) mientras se encuentra abierto un comentario de bloque (estados E22 o E23), se emite un *Error Léxico* señalando la línea donde inició el comentario y no un error sintáctico. Si se lee un carácter `!` no seguido de `=`, se deslee el siguiente carácter y se reporta *Error Léxico* ("Se esperaba '=' tras '!'"). Todo carácter ajeno al alfabeto emite *Error Léxico* y es descartado volviendo a E0. |

--------------------------------------------------------------------------------

#### 3. Eventos (columnas de las matrices)

La función auxiliar `get_evento(int c)` mapea el carácter leído del archivo fuente a un índice entero de columna (de 0 a 19):

| Col | Evento | Símbolo | Caracteres incluidos |
|:---:|:---|:---:|:---|
| **0** | Letra | `LETRA` | `a-z`, `A-Z` |
| **1** | Dígito | `DÍGITO` | `0-9` |
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
| **17** | Blanco / Separador | `BLANCO` | Espacio (`' '`), tabulación (`'\t'`), `\r`, `\n` |
| **18** | Fin de archivo | `FDA` | Carácter `FDA` (`-1`) |
| **19** | No reconocido | `OTRO` | Cualquier otro carácter fuera del alfabeto de DUOTIPO |

--------------------------------------------------------------------------------

#### 4. Estados y Diagrama

## 4. Estados y Diagrama

<img width="2691" height="1493" alt="AUTOMATA_COMPILADORES-DuoTipo v 4 drawio" src="https://github.com/user-attachments/assets/c8984f60-dc39-49d6-b46b-d777bca5dc32" />

---
<center>
  
| Estado | Significado |
|---|---|
| **E0** | Estado inicial; consume blancos o inicia el reconocimiento de tokens |
| **E1** | Reconociendo Identificador o Palabra Reservada |
| **E2** | Reconociendo Constante Entera |
| **E3** | Reconociendo Constante Real (tras leer parte entera y `.`) |
| **E4** | Tras leer `=`; puede ser asignación `=` o pasar a `==` |
| **E5** | Operador aritmético suma `+` |
| **E6** | Operador aritmético resta `-` |
| **E7** | Operador aritmético multiplicación `*` |
| **E8** | Tras leer `/`; puede ser división `/` o inicio de comentario `/*` |
| **E9** | Tras leer `<`; puede ser menor `<` o menor o igual `<=` |
| **E10** | Tras leer `>`; puede ser mayor `>` o mayor o igual `>=` |
| **E11** | Operador relacional menor o igual `<=` |
| **E12** | Operador relacional mayor o igual `>=` |
| **E13** | Operador relacional igualdad `==` |
| **E14** | Tras leer `!`; espera `=` para formar `!=` |
| **E15** | Operador relacional desigualdad `!=` |
| **E16** | Delimitador paréntesis izquierdo `(` |
| **E17** | Delimitador paréntesis derecho `)` |
| **E18** | Delimitador llave izquierda `{` |
| **E19** | Delimitador llave derecha `}` |
| **E20** | Delimitador fin de sentencia `;` |
| **E21** | Delimitador dos puntos `:` |
| **E22** | Dentro de cuerpo de comentario multilínea `/* ...` |
| **E23** | Posible fin de comentario (leyó `*` dentro de comentario, esperando `/`) |
| **EF (-1)** | Estado final; cierre de token y consulta a matriz de tokens |
  
</center>
---

--------------------------------------------------------------------------------

#### 5. Matriz de Nuevos Estados (`nuevo_estado[24][20]`)

Valores: `0` a `23` para estados intermedios, `-1` para Estado Final de aceptación (`EF`).

| Nombre de Estado | N° | LETRA (0) | DÍGITO (1) | = (2) | + (3) | - (4) | * (5) | / (6) | < (7) | > (8) | ! (9) | ( (10) | ) (11) | { (12) | } (13) | ; (14) | : (15) | . (16) | BLANCO (17) | FDA (18) | OTRO (19) |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| `Estado inicial` | **E0** | 1 | 2 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 14 | 16 | 17 | 18 | 19 | 20 | 21 | -1 | 0 | -1 | 0 |
| `palabra` | **E1** | 1 | 1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `entero` | **E2** | -1 | 2 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | 3 | -1 | -1 | -1 |
| `real` | **E3** | -1 | 3 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `asigancion (=)` | **E4** | -1 | -1 | 13 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `suma (+)` | **E5** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `resta (-)` | **E6** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `multiplicación (*)` | **E7** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `división (/)` | **E8** | -1 | -1 | -1 | -1 | -1 | 22 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `menor (< )` | **E9** | -1 | -1 | 11 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `mayor (>)` | **E10** | -1 | -1 | 12 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `menor o igual (<=)` | **E11** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `mayor o igual (>=)` | **E12** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `igualdad (==)` | **E13** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `posible distinto` | **E14** | -1 | -1 | 15 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `distinto ( !=)` | **E15** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `abre paréntesis` | **E16** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `cierra paréntesis` | **E17** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `llave apertura` | **E18** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `llave cierre` | **E19** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `fin sentencia (;)` | **E20** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `separador (:)` | **E21** | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 | -1 |
| `comentario (/* )` | **E22** | 22 | 22 | 22 | 22 | 22 | 23 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | -1 | 22 |
| `posible fin comentario` | **E23** | 22 | 22 | 22 | 22 | 22 | 23 | 0 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | 22 | -1 | 22 |

---
#### 6. Acciones semánticas

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


#### 7. Matriz de Acciones Semánticas (`acciones[24][20]`)

| Nombre de Estado | N° | LETRA (0) | DÍGITO (1) | = (2) | + (3) | - (4) | * (5) | / (6) | < (7) | > (8) | ! (9) | ( (10) | ) (11) | { (12) | } (13) | ; (14) | : (15) | . (16) | BLANCO (17) | FDA (18) | OTRO (19) |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| `estado inicial` | **E0** | f1 | f3 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | f5 | fe | f6 | fn | fe |
| `palabra` | **E1** | f1 | f1 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 | f2 |
| `entero` | **E2** | f4 | f3 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f3 | f4 | f4 | f4 |
| `real` | **E3** | f4 | f3 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 | f4 |
| `asigancion (=)` | **E4** | fn | fn | f5 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `suma (+)` | **E5** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `resta (-)` | **E6** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `multiplicación (*)` | **E7** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `división (/)` | **E8** | fn | fn | fn | fn | fn | f7 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `menor (< )` | **E9** | fn | fn | f5 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `mayor (>)` | **E10** | fn | fn | f5 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `menor o igual (<=)` | **E11** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `mayor o igual (>=)` | **E12** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `igualdad (==)` | **E13** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `posible distinto` | **E14** | fe | fe | f5 | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe | fe |
| `distinto ( !=)` | **E15** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `abre paréntesis` | **E16** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `cierra paréntesis` | **E17** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `llave apertura` | **E18** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `llave cierre` | **E19** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `fin sentencia (;)` | **E20** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `separador (:)` | **E21** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn |
| `comentario (/* )` | **E22** | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | f6 | fe | fn |
| `posible fin comentario` | **E23** | fn | fn | fn | fn | fn | fn | f8 | fn | fn | fn | fn | fn | fn | fn | fn | fn | fn | f6 | fe | fn |

---

#### 8. Tabla de Unreads (`unreads[24][20]`)

Un valor `1` indica que se debe reinsertar el carácter leído en el buffer de entrada con `ungetc(c, yyin)`. Un valor `0` indica que el carácter pertenece al token o fue consumido normalmente.

| Nombre de Estado | N° | LETRA (0) | DÍGITO (1) | = (2) | + (3) | - (4) | * (5) | / (6) | < (7) | > (8) | ! (9) | ( (10) | ) (11) | { (12) | } (13) | ; (14) | : (15) | . (16) | BLANCO (17) | FDA (18) | OTRO (19) |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| `estado inicial` | **E0** | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `palabra` | **1** | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `entero` | **E2** | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 | 0 | 1 |
| `real` | **E3** | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `asigancion (=)` | **E4** | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `suma (+)` | **E5** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `resta (-)` | **E6** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `multiplicación (*)` | **E7** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `división (/)` | **E8** | 1 | 1 | 1 | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `menor (< )` | **E9** | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `mayor (>)` | **E10** | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `menor o igual (<=)` | **E11** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `mayor o igual (>=)` | **E12** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `igualdad (==)` | **E13** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `posible distinto` | **E14** | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `distinto ( !=)` | **E15** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `abre paréntesis` | **E16** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `cierra paréntesis` | **E17** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `llave apertura` | **E18** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `llave cierre` | **E19** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `fin sentencia (;)` | **E20** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `separador (:)` | **E21** | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
| `comentario (/* )` | **E22** | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| `posible fin comentario` | **E23** | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

--------------------------------------------------------------------------------
Tabla de Tokens:
https://github.com/martinzwaryczz/compiladores-2026/blob/main/specs/01-
diseno/spec.md#4-tabla-de-tokens

#### 9.Matriz de Tokens


#### 10. Errores que emite esta fase

| Código | Condición | Mensaje emitido | Recuperación |
|:---:|:---|:---|:---|
| **ERR_LEX_01** | Carácter no perteneciente al alfabeto de DUOTIPO (`OTRO` o `.` aislado en E0). | `Error léxico en línea %d: Carácter '%c' no válido en el lenguaje.` | Se ignora el carácter no válido y el autómata vuelve al estado E0. |
| **ERR_LEX_02** | Operador `!` seguido de un carácter distinto de `=`. | `Error léxico en línea %d: Carácter '!' inesperado. Se esperaba '=' para formar '!='.` | Se aplica unread sobre el delimitador, se reporta el error y vuelve a E0. |
| **ERR_LEX_03** | Fin de archivo inesperado (`EOF`) dentro del cuerpo de un comentario multilínea `/* ...`. | `Error léxico: Fin de archivo inesperado. Comentario abierto en línea %d no fue cerrado.` | Se interrumpe la ejecución del analizador léxico reportando código de error. |
| **ERR_LEX_04** | Constante entera fuera del rango representable de 32 bits (`[-2147483648, 2147483647]`). | `Error léxico en línea %d: Constante entera '%s' fuera de rango (32 bits con signo).` | Se almacena el valor saturado o truncado y se continúa el análisis. |
| **ADVER_LEX_01** | Identificador que excede el límite de 20 caracteres. | `Advertencia léxica en línea %d: Identificador '%s' supera el límite de 20 caracteres. Se truncará a '%s'.` | El identificador se trunca a 20 caracteres y se almacena truncado en la Tabla de Simbolos. |

--------------------------------------------------------------------------------

#### 11. Traza de verificación

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

#### 12. Casos de prueba de esta fase

| Entrada | Salida esperada | Qué verifica |
|---|---|---|
| `entero var_1 = 45;` | `TK_INT`, `TK_ID("var_1")`, `TK_ASSIGN`, `TK_NUM_INT(45)`, `TK_SEMI` | Identificadores válidos, palabras reservadas y constantes enteras. |
| `identificador_demasiado_largo_para_el_lenguaje = 10;` | *Warning léxico en línea 1* (truncado a 20 caracteres) + `TK_ID("identificador_demas")`, `TK_ASSIGN`, `TK_NUM_INT(10)`, `TK_SEMI` | Límite máximo de longitud de identificadores (L2). |
| `a <= b y c != d;` | `TK_ID("a")`, `TK_LE`, `TK_ID("b")`, `TK_AND`, `TK_ID("c")`, `TK_NEQ`, `TK_ID("d")`, `TK_SEMI` | Operadores relacionales compuestos y conectores lógicos reservados. |
| `/* bloque sin cerrar` | *Error léxico: Fin de archivo inesperado. Comentario abierto en línea 1 no fue cerrado.* | Manejo correcto de EOF dentro de comentarios sin pasar al parser sintáctico. |
| `!a` | *Error léxico en línea 1: Carácter '!' inesperado* + `TK_ID("a")` | Manejo de exclamación no seguida de igual. |
