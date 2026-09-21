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
* **Problema a resolver**: Presentar las publicaciones al usuario permitiendo navegar hacia adelante (`[S] Siguiente`), hacia atrás (`[A] Anterior`), y eliminar tweets sin tener que recargar todas las publicaciones desde el archivo.
* **Alternativas consideradas**:
  1. *Lista simplemente enlazada*: No permite retroceder al tweet anterior sin reiniciar el recorrido desde el inicio de la lista.
  2. *Vector dinámico en memoria*: Moverse es eficiente, pero la eliminación de un tweet intermedio requiere desplazar todos los elementos posteriores en memoria. Esto implicaría una gran cantidad de copias en memoria, lo cual es ineficiente.
  3. *Lista doblemente enlazada*: Enlaces `sig` y `ant` por nodo.
* **Decisión y justificación**: Se eligió la **Lista Doblemente Enlazada (`tListaDoble`)**. Permite navegación en ambos sentidos, y ante la eliminación de cualquier posteo (primer tweet, último o uno intermedio), la desvinculación y liberación del nodo se realiza reajustando punteros sin mover informacion en memoria. Los posteos se presentan en orden cronológico inverso (el más reciente primero), replicando el comportamiento estándar de redes sociales.

---

## 3. Formato y Estrategia de Persistencia

Para garantizar que la informacion persista entre ejecuciones del programa y ademas cumplir con los requisitos generales de eficiencia se adoptó el siguiente esquema:

### 3.1. Archivos Binarios de Registros de Longitud Fija
* **`datos/usuarios.dat`**: Almacena estructuras `tUsuario`. Al tener tamaño fijo por registro (`sizeof(tUsuario)`), cualquier modificación (cambio de contraseña o baja lógica) se ejecuta de forma directa mediante `fseek(arch, offset, SEEK_SET)` y `fwrite`, sin reescribir el resto del archivo.
* **`datos/indice_usuarios.idx`**: Archivo de índices con estructuras `tIndiceUsuario`. Al iniciar el programa, Cwitter solo procesa este archivo para levantar en memoria la lista de índices y la pila de libres, logrando un arranque veloz.

### 3.2. Estrategia de Doble Archivo para Publicaciones
* **Buffer Temporal (`datos/posteos_temp.dat`)**: Cuando el usuario redacta una publicación, esta se persiste de inmediato en un archivo de lote temporal.
* **Consolidación (`combinarPosteos`)**: Se combinan las publicaciones temporales con el archivo histórico principal `datos/posteos.dat`. Para optimizar escrituras masivas y evitar contención o bloqueos en disco, los nuevos posteos se integran en bloque.
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
