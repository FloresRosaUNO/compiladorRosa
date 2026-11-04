## Spec — 04-analizador-sintactico

simplememnte tiene que tener referencia a nuestra gramatica nuestro al es corta que use nuestro analizador sintactico nuestroy no el bison. 


### Objetivo
Reconocer un lenguaje de entrada aplicando las reglas gramaticales correspondientes definidas en la [gramática de DUOTIPO](specs/01-diseno/specDUOTIPO1.md#6-gramática). El analizador sintáctico procesa la entrada invocando bajo demanda al analizador léxico desarrollado por el grupo a través de la función `int yylex(void)` (sin utilizar Flex) y tomando como referencia la Tabla de Símbolos definida en `specs/02-tabla-simbolos/spec-tabladesimbolos-DUOTIPOv1.md`. ( linkear analizador lexico y el lexer )

 siguiente spesificacion 
.
 desde el accioma hasta poner todos los terminales
 pacinr ascendente desdde los terminales y subo uniendo las flechas hacia el no terminal
 en vez de descendente, cuando el parcing reduzca 
 agregar los saltos en comparaciones, saltos, bifurcaciones, ir por polaca porque es la mas sencilla

 
### Alcance
* Si el lenguaje es reconocido por la Gramática Independiente del Contexto (GIC), la aplicación devolverá `1` y emitirá por consola `Compilación exitosa`.
* En caso de una discrepancia sintáctica, se interrumpirá el análisis, lo notificará `yyerror()` por `stderr` y la aplicación devolverá `0` (`Compilación no exitosa`).
* **Delimitación de etapa:** En esta fase el parser se limita exclusivamente a validar la corrección sintáctica del código fuente. No se generan tercetos, árboles sintácticos ni código intermedio en las acciones semánticas de Bison.

### Entradas
* **Tira de Tokens:** Secuencia bajo demanda devuelta por `yylex()` con códigos numéricos $\ge 301$ ($301 \dots 335$), finalizando con `0` (`YYEOF`) para el fin de archivo.
* **Archivo fuente:** Código del lenguaje DUOTIPO abierto a través del descriptor de archivo (`yyin`).
* **Información de contexto léxico:** Variable global `yylineno` (mantenida por el analizador léxico) para reportar la línea exacta ante eventuales fallas sintácticas.

### Salidas
* **Código de retorno del proceso:** Entero devuelto por la aplicación (`1` si el programa fuente es sintácticamente válido según la GIC, `0` si contiene errores sintácticos o léxicos).
* **Mensajes de diagnóstico:** En caso de discrepancia sintáctica, notificación estándar por `stderr` con formato:
  ```text
  Error sintactico (P01): syntax error en linea <yylineno>
  ```
* **Lista de reducciones / reglas:** Derivación ascendente procesada internamente por el autómata de pila LALR(1) para habilitar las etapas posteriores del compilador. Mediante la directiva `%debug` y el flag `--debug` (`yydebug = 1`), se emitirá en consola la traza de desplazamientos (*shifts*) y reducciones (*reductions*) para auditar el proceso.

### Reglas y decisiones de diseño
* **Generador de parser:** Empleo de GNU Bison operando en modo LALR(1) con salida en lenguaje C.
* **Integración obligatoria del analizador léxico propio:** Bison invoca directamente la función `yylex()` implementada en C en `src/lexico/lexer.c` sin recurrir a generadores externos como Flex.
* **Coordinación de códigos de terminales:** Los identificadores de tokens en `%token` coinciden exactamente con los enteros asignados en la matriz de tokens del lexer y en el spec de diseño inicial ($301 \dots 335$).
* **Precedencia y asociatividad:** Declaración explícita de asociatividad a izquierda (`%left`) para operadores lógicos (`o`, `y`), relacionales y aritméticos (`+`, `-`, `*`, `/`).
* **Menos unario:** Se declara en `termino_n` empleando `%prec UMINUS` para asociar por derecha con máxima precedencia.
* **Limpieza de reglas:** Las reglas gramaticales no ejecutan generación de código intermedio ni llamadas a listas de tercetos; se limitan a validar la estructura sintáctica de DUOTIPO.
# generando lista de tercetos. en esta regla gramatical agregar mas menos. ejemplos.
### Criterios de aceptación
* **Caso Válido (Aceptación):** El archivo `tests/correctos/prueba1_valida.txt` debe ser procesado de principio a fin sin errores, retornando código `1` y emitiendo `Compilación exitosa`.
* **Caso Error por Delimitador:** La omisión de un punto y coma `;` (como en `tests/errores-lexicos-sintacticos/prueba_error_falta_semi.txt`) debe detener la ejecución, emitir `Error sintactico (P01): syntax error en linea <yylineno>` por `stderr` y retornar código `0`.
* **Caso Error por Bloque Desbalanceado:** Una estructura o bloque con llaves `{` sin cerrar (como en `tests/errores-lexicos-sintacticos/prueba_error_llave.txt`) debe ser interceptada por `yyerror()`, retornando código `0`.
* **Depuración:** La ejecución con `--debug` debe mostrar en pantalla la secuencia de lecturas de tokens y reducciones gramaticales de Bison.
lo que esta en las ppts este, que este especificamente este los de las ppts en lo de assembler.
