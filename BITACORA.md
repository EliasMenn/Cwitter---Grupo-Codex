# Bitácora de Diseño — Cwitter (Sprint 1)
**Grupo: Codex**  

---

## 1. Entidades Identificadas

Despues de realizar un analisis de los requisitos planteados, identificamos las siguientes entidades principales:

### 1.1. Usuario (`tUsuario`)
Representa la cuenta de una persona en la red social.
* **`id` (`unsigned`)**: Identificador único, asignado al usuario al momento de registrarse.
* **`usuario` (`char[21]`)**: Nombre de usuario único.
* **`contrasenia` (`char[21]`)**: Clave de acceso.
* **`estado` (`char`)**: Indica el estado del usuario: `'A'` (Activo) o `'B'` (Baja lógica).

### 1.2. Índice de Usuario en Memoria (`tIndiceUsuario`)
Estructura diseñada para residir en memoria y utilizada para las operaciones de búsqueda y autenticación sin cargar los datos completos de los usuarios.
* **`id` (`unsigned`)**: ID del usuario.
* **`usuario` (`char[21]`)**: Clave alfanumérica ordenada.
* **`estado` (`char`)**: Permite verificar si la cuenta está activa sin leer el disco.
* **`offsetDat` (`long`)**: Posición en bytes del registro completo dentro del archivo `usuarios.dat`.
* **`offsetIdx` (`long`)**: Posición en bytes dentro del archivo de índices `indice_usuarios.idx`.

### 1.3. Publicación / Tweet (`tPosteo`)
Representa un mensaje publicado por un usuario en la plataforma.
* **`id` (`unsigned`)**: Identificador único del posteo.
* **`nombreUsuario` (`char[21]`)**: Autor de la publicación.
* **`publicacion` (`char[141]`)**: Contenido del mensaje.

### 1.4. Espacio Libre / Hueco (`tEspacioLibre`)
Representa una posición física en los archivos `.dat` e `.idx` correspondiente a una cuenta dada de baja.
* **`offsetDat` (`long`)**: Desplazamiento libre en `usuarios.dat`.
* **`offsetIdx` (`long`)**: Desplazamiento libre en `indice_usuarios.idx`.

### 1.5. Feed de Publicaciones (`tFeed`)
Estructura en memoria representada como una lista doblemente enlazada de posteos, permitiendo la visualización secuencial de los mismos.
* **`tListaDoble p`**: Lista doblemente enlazada que contiene los posteos.
* **`int posteo_actual`**: Índice del posteo actual en la lista.
* **`long offset`**: Offset del posteo actual en el archivo de publicaciones.
* **`long inicio`**: Offset del primer posteo en el archivo de publicaciones.

---

## 2. Estructuras de Datos Elegidas y Justificación

### 2.1. Gestión de Usuarios en Memoria: Lista Simplemente Enlazada Ordenada (`tLista`)
* **Problema a resolver**: Administrar el conjunto de usuarios activos, garantizar la unicidad del nombre de usuario en los registros y autenticar logins rápidamente.
* **Alternativas consideradas**:
  1. *Vector dinámico (`realloc`)*: Requiere redimensionamiento en memoria que puede fallar por fragmentación cuando la plataforma escale a miles de usuarios.
  3. *Lista simplemente enlazada ordenada*: Permite crecimiento dinámico nodo a nodo según demanda, sin memoria desperdiciada.
* **Decisión y justificación**: Se implementó una **Lista Simplemente Enlazada Ordenada** por `usuario` realizando inserción ordenada sin duplicados. Además, en la lista de memoria solo se almacenan los índices `sizeof(tIndiceUsuario)`, manteniendo un consumo de memoria mínimo independientemente del volumen total en disco.

### 2.2. Reciclaje de Registros Dados de Baja: Pila (`tPila`)
* **Problema a resolver**: Al dar de baja una cuenta, no se debe dejar espacio muerto indefinidamente en disco ni realizar una reescritura masiva de todo el archivo para compactarlo.
* **Alternativas consideradas**:
  1. *Compactación total del archivo en cada baja*: Rechazada por ser sumamente ineficiente en disco.
  2. *Búsqueda secuencial de registros libres al registrar*: Ineficiente al momento de crear cuentas nuevas. Esto implicaria recorrer todo el archivo de indices en el peor de los casos.
  3. *Pila de huecos disponibles en memoria (LIFO)*: Almacena los offsets liberados y los entrega de manera eficiente para ser reutilizados ante un nuevo registro.
* **Decisión y justificación**: Se utilizó una **Pila (`tPila`)** para guardar las posiciones libres (`tEspacioLibre`). Ante un alta, se consulta primero la pila: si hay huecos disponibles, se reutiliza inmediatamente el último offset desapilado, sobrescribiendo la posición en disco. Si la pila está vacía, se escribe al final del archivo. Al reiniciar el programa, la pila se reconstruye leyendo los registros con `estado == 'B'` desde el archivo de índices.

### 2.3. Feed de Publicaciones en Memoria: Lista Doblemente Enlazada (`tListaDoble`)
* **Problema a resolver**: Presentar las publicaciones al usuario permitiendo navegar hacia adelante (`[S] Siguiente`), hacia atrás (`[A] Anterior`), asegurar que no se exceda la memoria disponible del sistema
    y eliminar tweets sin tener que recargar todas las publicaciones desde el archivo.
* **Alternativas consideradas**:
  1. *Lista simplemente enlazada*: No permite retroceder al tweet anterior sin reiniciar el recorrido desde el inicio de la lista.
  2. *Vector dinámico en memoria*: Moverse es eficiente, pero la eliminación de un tweet intermedio requiere desplazar todos los elementos posteriores en memoria. Esto implicaría una gran cantidad de copias en memoria, lo cual es ineficiente.
  3. *Lista doblemente enlazada*: Enlaces `sig` y `ant` por nodo.
* **Decisión y justificación**: Se eligió la **Lista Doblemente Enlazada (`tListaDoble`)**. Permite navegación en ambos sentidos, y ante la eliminación de cualquier posteo (primer tweet, último o uno intermedio), la desvinculación y liberación del nodo se realiza reajustando punteros sin mover informacion en memoria. Los posteos se presentan en orden cronológico inverso (el más reciente primero), replicando el comportamiento estándar de redes sociales.
* **Metodo de implementacion**: Debido a que no sabemos la cantidad real de publicaciones no podemos optar por cargar el archivo entero y realizar una carga cada vez que el usuario quiera ver un tweet nuevo es un desproposito. por eso optamos por una implementacion de `visor` donde cargamos una cantidad M, al llegar al ultimo de estos posteos, quitamos de memoria los N primeros y agregamos la misma cantidad al final. 
Si el usuario volviera hasta el comienzo (y no estemos al comienzo de archivo) quitamos los N ultimos registros del final de nuestra lista y agregamos la misma cantidad al comienzo.
---

## 3. Formato y Estrategia de Persistencia

Para garantizar que la informacion persista entre ejecuciones del programa y ademas cumplir con los requisitos generales de eficiencia se adoptó el siguiente esquema:

### 3.1. Archivos Binarios de Registros de Longitud Fija
* **`datos/usuarios.dat`**: Almacena estructuras `tUsuario`. Al tener tamaño fijo por registro (`sizeof(tUsuario)`), cualquier modificación (cambio de contraseña o baja lógica) se ejecuta de forma directa mediante `fseek(arch, offset, SEEK_SET)` y `fwrite`, sin reescribir el resto del archivo.
* **`datos/indice_usuarios.idx`**: Archivo de índices con estructuras `tIndiceUsuario`. Al iniciar el programa, Cwitter solo procesa este archivo para levantar en memoria la lista de índices y la pila de libres, logrando un arranque veloz.

### 3.2. Estrategia de Doble Archivo para Publicaciones
* **Buffer Temporal (`datos/posteos_temp.dat`)**: Cuando el usuario redacta una publicación, esta se persiste de inmediato en un archivo de lote temporal.
* **Consolidación (`combinarPosteos`)**: Se combinan las publicaciones temporales con el archivo histórico principal `datos/posteos.dat`. Para optimizar escrituras masivas y evitar contención o bloqueos en disco, los nuevos posteos se integran al archivo temporal/buffer y cuando el usuario cierra su sesion
    se combinan con los posteos anteriores, asegurando de invertir el orden del archivo temporal antes de insertarlo al comienzo de todo (asegurando que el orden sea de la publicacion mas nueva a la mas vieja) esta funcion tambien se ejecuta al comienzo por si el programa fuese cerrado de manera abrupta 
    no haya perdida de ninguna publicacion. no se ejecuta tras cada publicacion por el alto coste de reescribir todo el archivo
* **Eliminación Segura**: Al borrar una publicación en disco, se utiliza la técnica estándar de archivo nuevo temporal (`posteos_nue.dat`) copiando únicamente las publicaciones activas y renombrando con `rename()`. Esto evita dejar registros corruptos y asegura la integridad ante caídas inesperadas del sistema.

---

## 4. Operaciones Futuras Probables

Pensando en el crecimiento de Cwitter y la escalabilidad de la aplicacion, podrian :

1. **Sistema de Seguidores**:
   * *Por qué*: Las redes sociales requieren feeds personalizados según las cuentas que cada usuario sigue.
   * *Impacto en diseño*: Implicará modelar relaciones muchos-a-muchos (red o grafo), implementable mediante listas o alguna estructura de datos.
2. **Likes, Reacciones**:
   * *Por qué*: Medir la popularidad de cada tweet.
   * *Impacto en diseño*: Se requerirá asociar contadores al posteo.
3. **Hilos y Respuestas**:
   * *Por qué*: Permitir conversaciones entre usuarios.
   * *Impacto en diseño*: Se deberia implementar una relacion entre distintos posteos, por ejemplo con un campo para el ID del posteo padre.

---

## 5. Registro de Uso de Inteligencia Artificial (IA)

En cumplimiento con la modalidad de trabajo de la cátedra, se detallan las consultas realizadas a herramientas de IA, la evaluación crítica del equipo y las soluciones implementadas:

### Interacción 1: Estructura de Memoria para Navegación del Feed
* **Prompt utilizado**:
  > *"¿Qué estructura de datos dinámica en C es más recomendable para implementar el feed de una red social que permita avanzar al siguiente tweet, retroceder al anterior y borrar tweets?"*
* **Resumen de la respuesta obtenida**:
  La IA propuso dos alternativas: un búfer circular en un array dinámico con `realloc` o una lista doblemente enlazada. Destacó que el búfer circular ofrece mejor localidad temporal de caché, pero que la lista doblemente enlazada facilita la eliminación de elementos intermedios sin huecos ni desplazamientos.
* **Evaluación y decisión del equipo**:
  Analizamos que en un entorno con restricciones de memoria limitada, el costo de redimensionar memoria con `realloc` y desplazar bloques continuos al borrar un tweet representaba un riesgo innecesario. Decidimos adoptar la **Lista Doblemente Enlazada** porque resuelve el avance, retroceso y borrado de nodos intermedios manipulando los punteros `ant` y `sig`.

---

### Interacción 2: Reciclaje de Espacio de Cuentas Dadas de Baja
* **Prompt utilizado**:
  > *"¿Cómo conviene manejar la baja de usuarios en archivos binarios de registros de longitud fija en C para no desperdiciar espacio en disco sin compactar todo el archivo en cada borrado?"*
* **Resumen de la respuesta obtenida**:
  Nos sugirió implementar bajas lógicas cambiando un campo de estado a `'B'` y almacenar los desplazamientos (`offsets`) disponibles en una pila LIFO o cola FIFO en memoria, para luego sobreescribir dichos offsets en las siguientes altas.
* **Evaluación y decisión del equipo**:
  Decidimos que una estructura tipo **Pila (`tPila`)** era la solución óptima: Con su implementación como TDA dinámico, permite consultar si hay espacio disponible antes de hacer un `fseek` al final del archivo, y se puede reapilar fácilmente al leer los índices en el arranque del programa.

# Sprint 2

## 1 Modificaciones necesarias:

Tras Realizar analisis de los requisitos nuevos, optamos por modificar los siguientes campos y funciones:

### 1.1 Usuario:
* Se le agrego un campo de verificado al usuario, permitiendo la persistencia de la verificacion a traves de sesiones.

### 1.2 Publicaciones:
* Se le agrego un campo de verificado a las publicaciones, permitiendo la identificacion de las mismas, a su vez facilitando la implementancion del orden solicitado
* Se le agrego un campo de horario para permitir la visualizacion del orden (El orden ya existia pero no se almacenaba fecha y hora)

### 1.3 Funcion de Combinar Posteos:
* Se modifico la funcion para verificar el campo nuevo que agregamos a publicaciones, si coincide con el char 'S' insertamos esas publicaciones primero, sino se respeta el orden de publicacion.

## 2 Implementaciones nuevas:

### 2.1 UsuarioVerificado:
* Se creo esta estructura para permitir identificar a los usuarios verificados y su cantidad de publicaciones, evitando asi bajar a memoria toda la informacion correspondiente a los mismos cuando nos solicitan el top 5 con mas publicaciones.

### 2.2 Funciones de API
* Se generaron multiples funciones relacionadas a la comunicacion con el API proporcionado, entre ellas una funcion de POST, GET y las funciones que procesan las respuestas segun que solicitud hagamos

### 2.3 Estructuras de datos
* Todas las funcionalidades implementadas en este sprint fueron implementadas bajo estructuras de datos ya presentes en nuestras librerias, principalmente la estructura de lista simplemente enlazada para los usuarios verificados y procesamiento del API. frente a el feed de publicaciones este ya se encontraba implementado bajo una estructura de lista doblemente enlazada.

## 3. Registro de Uso de Inteligencia Artificial (IA)

* **Prompt Utilizado:**
  > *"Como podria implementar un GET request para obtener una lista de datos en formato json desde un API"*
* **Resumen de la respuesta:**
  > *"La IA (claude en este caso), propuso una funcion para obtener el request y procesar el json, haciendo ajustes menores segun lo que fuimos probando en la conversacion y cambiando el formato de devolucion"*
* **Decision del equipo:**
  > *"Tras probar la solucion propuesta e ir ajustando el resultado final (tnato consultando a la IA como ajustando a mano y probando), obtenemos una funcion que realiza el GET request y nos devuelve la informacion que necesitamos de manera tal que podamos procesarla sin mayores inconvenientes"*

---

# Sprint 3

## 1. Modificaciones Realizadas sobre Entidades y Límites

### 1.1. Límite de Caracteres en Publicaciones (`MAX_CHARS` y `LARGO_MAX`)
* **Requisito del cliente**: Aumentar la longitud permitida de las publicaciones de 140 a 280 caracteres.
* **Ajuste técnico**:
  * Se redefinió `MAX_CHARS` a `280` en `publicaciones.h`.
  * Se redefinió `LARGO_MAX` como `(MAX_CHARS + 1)` (281 bytes), asegurando el espacio para el terminador nulo (`'\0'`) en la estructura `tPosteo` y en los buffers de lectura.
  * Se actualizaron los textos de la interfaz en `menu.c` y las validaciones de entrada en `publicaciones.c`.

### 1.2. Hilos de Conversación y Respuestas Anidadas (`id_padre`)
* **Requisito del cliente**: Permitir responder a cualquier tweet y que esas respuestas puedan recibir a su vez otras respuestas, sin límite de profundidad.
* **Ajuste técnico**:
  * Se incorporó el campo `unsigned id_padre` en la estructura `tPosteo`.
  * Un valor de `id_padre == 0` representa un posteo raíz (independiente).
  * Un valor de `id_padre > 0` vincula la publicación como respuesta al tweet con dicho identificador.

---

## 2. Estudio de Escalabilidad y Benchmark de Búsqueda por ID

Siguiendo las recomendaciones del cliente antes de modificar el diseño original, se procedió a medir empíricamente el comportamiento de la búsqueda de tweets ante volúmenes crecientes de datos (100, 1.000, 10.000 y 100.000 registros).

### 2.1. Metodología de Medición
Se desarrolló una herramienta automatizada de benchmark (`benchmark.c`) utilizando `QueryPerformanceCounter` en Windows para garantizar precisión de microsegundos. Se evaluó el **peor caso** (búsqueda de un ID inexistente al final de la colección) comparando la búsqueda secuencial del Sprint 1 frente a la búsqueda indexada en memoria del Sprint 3.

### 2.2. Tabla Comparativa de Rendimiento (Peor Caso)

| Cantidad de Tweets | Búsqueda Secuencial en Disco (Sprint 1) | Comparaciones / Lecturas (Sprint 1) | Búsqueda Indexada AVL (Sprint 3) | Comparaciones en Árbol (Sprint 3) |
| :---: | :---: | :---: | :---: | :---: |
| **100** | 0.022 ms | 100 | 0.00004 ms | **14** |
| **1.000** | 0.186 ms | 1.000 | 0.00005 ms | **20** |
| **10.000** | 3.310 ms | 10.000 | 0.00010 ms | **28** |
| **100.000** | **26.465 ms** | **100.000** | **0.00010 ms** | **34** |

### 2.3. Análisis de Resultados y Justificación del Reemplazo
* **Búsqueda del Sprint 1 (Lineal en Disco)**:
  * Presenta una complejidad temporal $O(N)$ tanto en tiempo como en accesos a disco. Al pasar de 10.000 a 100.000 tweets, el tiempo saltó de 3.31 ms a 26.46 ms, y requirió escanear 32.8 MB de archivo de principio a fin. Esto confirmó que la búsqueda secuencial original no era sostenible para una plataforma en crecimiento masivo.
* **Búsqueda del Sprint 3 (Índice en Memoria con Árbol AVL)**:
  * Presenta una complejidad logarítmica estricta $O(\log N)$. Para 100.000 tweets, el peor caso requirió apenas **34 comparaciones en memoria RAM**, resolviéndose en una fracción insignificante de milisegundo (0.0001 ms).
  * Si se necesita leer el registro completo desde disco, el índice entrega el `offset` exacto en bytes, reduciendo el acceso físico a **1 solo `fseek` y 1 solo `fread`**.

---

## 3. Estructuras de Datos Elegidas y Justificación

### 3.1. Índices en Memoria: Árboles Balanceados AVL (`tArbol`)
* **Problema a resolver**: Localizar rápidamente cualquier tweet por su ID para permitir respuestas, ediciones o eliminaciones, y además mantener agrupadas las respuestas de cada conversación.
* **Alternativas consideradas**:
  1. *Vector dinámico ordenado con búsqueda binaria*: Si bien la búsqueda es $O(\log N)$, la inserción de nuevos tweets exige redimensionar con `realloc` y desplazar bloques masivos de memoria ($O(N)$), resultando ineficiente y propensa a fallos de fragmentación.
  2. *Tabla Hash*: Ofrece búsqueda promedio $O(1)$, pero no mantiene los elementos ordenados, lo que dificulta el recorrido secuencial de respuestas y la prioridad de usuarios verificados.
  3. *Árbol AVL (Árbol Binario de Búsqueda Autobalanceado)*: Mantiene balanceada su altura mediante rotaciones simples y dobles cuando el factor de equilibrio excede $[-1, 1]$, garantizando altura máxima $\le 1.44 \log_2(N)$.
* **Decisión y justificación**: Se adoptó el **Árbol AVL** (`global/arbol.c` y `global/arbol.h`). Se instancian dos índices en memoria:
  * `idxPosteos`: Indexa todos los posteos ordenados por verificación (`verificado` primero) y por ID descendente.
  * `idxRespuestas`: Indexa únicamente las respuestas, ordenadas primariamente por `id_padre`, luego por `verificado` y por orden cronológico.

### 3.2. Representación y Visualización de los Hilos de Conversación
* **Problema a resolver**: Mostrar las conversaciones jerárquicas con distintos niveles de profundidad dentro del feed general, respetando la paginación de ventana móvil (3 tweets por pantalla) de la lista doblemente enlazada.
* **Decisión y justificación**:
  * **Modelo lógico (Árbol N-ario)**: Un tweet raíz puede tener $N$ respuestas directas, y cada una de ellas sub-respuestas. Este árbol N-ario se recorre en profundidad (**DFS - Depth First Search**) combinando el avance del feed con el índice `idxRespuestas`.
  * **Cálculo dinámico de profundidad**: En lugar de sobrecargar el archivo de disco con campos redundantes, la función `obtenerNivelProfundidad(id_padre)` consulta el índice AVL en memoria subiendo por los padres hasta la raíz en tiempo $O(\text{nivel} \cdot \log N)$, lo cual es instantáneo.
  * **Sangría visual escalonada**: En el feed general, cada tweet evalúa su nivel: el nivel 0 se muestra al margen, mientras que los niveles $\ge 1$ se muestran sangrados con espacios proporcionales y el indicador explícito `(en respuesta a #padre)`, permitiendo al usuario identificar con total claridad qué responde a qué sin abandonar el feed.

---

## 4. Formato y Estrategia de Persistencia Modificada

Para asegurar que las relaciones jerárquicas entre tweets sobrevivan al cierre del programa:

1. **Estructura en Disco (`datos/posteos.dat`)**:
   * Se expandió el registro de longitud fija `tPosteo` incorporando `unsigned id_padre` (4 bytes).
   * Al ser de tamaño fijo uniforme (328 bytes por tweet con el texto de 280 caracteres), se conserva la capacidad de acceso aleatorio directo vía `fseek(arch, offset, SEEK_SET)`.
2. **Reconstrucción al Iniciar (`almacenamiento_reconstruir_indices`)**:
   * Al abrir la aplicación, se lee secuencialmente `datos/posteos.dat` una única vez y se pueblan los árboles AVL `idxPosteos` e `idxRespuestas` con sus respectivas posiciones (`offset`).
   * Al cerrar sesión o publicar, la función `combinarPosteos()` consolida los tweets temporales en el archivo histórico y regenera los índices en memoria para mantener sincronizados los offsets.

---

## 5. Registro de Uso de Inteligencia Artificial (IA)

En cumplimiento con la modalidad de trabajo de la cátedra, se detallan las consultas y asistencias de IA durante este sprint:

### Interacción 1: Medición de Rendimiento y Benchmark de Búsqueda
* **Prompt utilizado**:
  > *"¿Cómo podemos diseñar un benchmark en C para medir el peor caso de búsqueda secuencial en archivo binario vs. búsqueda en árbol AVL en memoria para 100, 1.000, 10.000 y 100.000 registros en Windows?"*
* **Resumen de la respuesta**:
  La IA propuso utilizar la API de temporización de alta resolución de Windows (`QueryPerformanceCounter` y `QueryPerformanceFrequency`), instrumentar un contador de comparaciones dentro de la función de comparación del árbol AVL y evaluar el caso de búsqueda de claves inexistentes para garantizar la medición del peor caso real.
* **Evaluación y decisión del equipo**:
  Implementamos el script `benchmark.c` con dicha metodología. Obtuvimos mediciones reales en nuestro hardware que demostraron el pasaje de $O(N)$ a $O(\log N)$, las cuales volcamos en la tabla comparativa de esta bitácora.

### Interacción 2: Integración de Jerarquías en la Ventana Móvil del Feed
* **Prompt utilizado**:
  > *"¿Cómo podemos representar visualmente los niveles de profundidad de las respuestas dentro del visor de feed existente (que pagina de a 3 tweets) sin romper la lista doblemente enlazada ni crear un menú aparte?"*
* **Resumen de la respuesta**:
  Se planteó calcular la profundidad de cada tweet en tiempo de visualización subiendo por la cadena de `id_padre` a través del índice en memoria, e imprimir una sangría proporcional junto con la referencia al posteo padre.
* **Evaluación y decisión del equipo**:
  Adoptamos la solución creando `obtenerNivelProfundidad()` y adaptando `mostrarPosteo()`. El feed conservó su navegación fluida de 3 tweets por página, agregando claridad visual inmediata sobre la jerarquía de las conversaciones.

