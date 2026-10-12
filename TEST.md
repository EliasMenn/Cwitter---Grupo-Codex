# Casos de Prueba
## Sprint 1

| # | Situación a probar | Pasos de la operacion | Salida Esperada | Estado |
| :---: | :--- | :--- | :--- | :---: |
| **01** | **Plataforma vacía al iniciar** | 1. Iniciar Cwitter por primera vez sin datos en disco (`datos/` vacía).<br>2. Registrar un usuario (`@franco`).<br>3. Iniciar sesión y entrar a `1. Ver Feed de publicaciones`. | El programa inicia en el menú principal; al ingresar al feed muestra `"No hay publicaciones en el feed todavia."`. | **OK** |
| **02** | **Usuario sin tweets consulta el feed** | 1. Iniciar sesión con un usuario que aún no tiene publicaciones.<br>2. En el menú de usuario, seleccionar la opción `1. Ver Feed de publicaciones`. | Muestra la pantalla del feed con `"No hay publicaciones en el feed todavia."` y vuelve al menú. | **OK** |
| **03** | **Búsqueda de un tweet que no existe** | 1. Con sesión activa, seleccionar `3. Buscar publicaciones`.<br>2. Ingresar un término o patrón sin coincidencias (ej: `*inexistente*`). | Muestra en consola `"No se encontraron publicaciones con '*inexistente*'."` y vuelve al menú. | **OK** |
| **04** | **Eliminación del primer tweet** | 1. Con tweets #1, #2 y #3 publicados, el autor selecciona `4. Eliminar una publicacion`.<br>2. Ingresar ID `1`.<br>3. Confirmar con `S`. | Muestra `"Tweet #1 eliminado exitosamente."`; el tweet #1 desaparece del feed y de disco. | **OK** |
| **05** | **Eliminación del último tweet** | 1. En el menú de usuario, seleccionar `4. Eliminar una publicacion`.<br>2. Ingresar el ID del último tweet publicado (ej: ID `3`).<br>3. Confirmar con `S`. | Muestra confirmación en pantalla y el tweet #3 queda removido del sistema. | **OK** |
| **06** | **Eliminación de un tweet intermedio** | 1. Con tweets #1, #2 y #3 en el feed, el autor selecciona `4. Eliminar una publicacion`.<br>2. Ingresar ID `2`.<br>3. Confirmar con `S`. | Muestra `"Tweet #2 eliminado exitosamente."` y el feed enlaza limpiamente el tweet #1 con el #3. | **OK** |
| **07** | **Intentar publicar sin estar logueado** | 1. En el menú principal (previo al login), intentar realizar una publicación. | El programa bloquea cualquier intento de publicar sin autenticarse previamente. | **OK** |
| **08** | **Intentar ver el feed sin estar logueado** | 1. En el menú principal, intentar acceder a ver el feed. | Acceso bloqueado; se requiere autenticación obligatoria con usuario y contraseña válidos. | **OK** |
| **09** | **Cerrar y reabrir el programa (Persistencia)** | 1. Registrar usuario `@anaana` y publicar `"Mi primer tweet"`.<br>2. Cerrar sesión (`0`) y salir del programa (`0`).<br>3. Reabrir el ejecutable, loguear con `@anaana` y entrar a `1. Ver Feed`. | Persistencia confirmada; tanto el usuario como el tweet sobreviven al cierre del programa. | **OK** |

---

## Casos de Prueba adicionales - Sprint 1

| # | Situación a probar | Pasos de la operacion | Salida esperada | Estado |
| :---: | :--- | :--- | :--- | :---: |
| **10** | **Intento de eliminar un tweet de otro usuario** | 1. Iniciar sesión con `@usuarioB`.<br>2. Seleccionar `4. Eliminar una publicacion`.<br>3. Ingresar el ID de un tweet publicado por `@usuarioA`.<br>4. Confirmar con `S`. | Se muestra en pantalla `"Error: No tiene permisos para eliminar este tweet."` y el tweet no es eliminado. | **OK** |
| **11** | **Registro con nombre de usuario duplicado** | 1. En el menú principal, seleccionar `1. Registrarse`.<br>2. Ingresar un nombre de usuario que ya está registrado (ej: `franco`) y una contraseña válida. | Muestra `"Error: Ya existe un usuario registrado con ese nombre."` sin generar duplicados. | **OK** |

## Sprint 2

| # | Situación a probar | Pasos de la operacion | Salida esperada | Estado |
| :---: | :--- | :--- | :--- | :---: |
| **12** | **Registro de usuario verificado** | 1. Registramos `@usuarioVer` a traves de la opcion 1 "Registrar Usuario", ingresamos nombre y contraseña.| Se muestra en pantalla `"[RESPUESTA FAMECHECK] HTTP 200: {"nombre": "@usuarioVer", "verificado":"true""` luego de esto el uso de la aplicacion es el normal de cualquier usuario. | **OK** |
| **13** | **Registro de usuario NO verificado** | 1. Registramos `@usuario` a traves de la opcion 1 "Registrar Usuario", ingresamos nombre y contraseña.| Se muestra en pantalla `"[RESPUESTA FAMECHECK] HTTP 200: {"nombre": "@usuario", "verificado":"false""` luego de esto el uso de la aplicacion es el normal de cualquier usuario. | **OK** |
| **14** | **Registro publicacion sin conexion a famecheck** | 1. Iniciamos sesion en la plataforma con `@usuarioVer`.<br>2. Elegimos la opcion para publicar, escribimos nuestro tweet y lo publicamos | La aplicacion genera correctamente el tweet, lo guarda con el tag de verificado y lo guarda en el orden correcto, no cuenta para la estadistica de mayor cantidad de posteos, se registra de la siguiente manera en el log de fallos .txt `"[2026-10-12 09:07:14] Operacion: POST /api/Cwitter/Cweet Detalle: HTTP 401 al reportar tweet de @usuarioVer"` | **OK** |
| **15** | **Top 5 con menos de 5 usuarios verificados** | 1. Iniciamos sesion en la plataforma.<br>2. Elegimos la opcion 8 "Ver top de usuarios" teniendo menos de 5 cuentas verificadas registradas en FameCheck (ej: 3 usuarios). | La aplicacion consulta a FameCheck y muestra en pantalla unicamente los usuarios disponibles ordenados de mayor a menor por cantidad de publicaciones, sin errores ni datos nulos. | **OK** |
| **16** | **Top 5 con algun empate** | 1. Iniciamos sesion en la plataforma.<br>2. Elegimos la opcion 8 "Ver top de usuarios" habiendo cuentas con la misma cantidad de tweets reportados (ej: dos cuentas con 4 publicaciones). | La aplicacion ordena la lista preservando los usuarios empatados (`INSERTAR_CON_DUP`) y presenta el top correctamente acotado al limite de 5 posiciones. | **OK** |
| **17** | **Feed con mezcla de verificados y no verificados** | 1. Publicamos tweets con cuentas no verificadas y con cuentas verificadas en distintos momentos.<br>2. Ingresamos a la opcion 1 "Ver Feed de publicaciones". | Los tweets de las cuentas verificadas se muestran al comienzo del feed (con el indicador `[S]`), y luego aparecen los tweets de cuentas no verificadas (con el indicador `[N]`), respetando el orden cronologico de publicacion. | **OK** |
| **18** | **Recorrer el feed hacia adelante y volver varias veces seguidas** | 1. Ingresamos a la opcion 1 "Ver Feed de publicaciones" con multiples tweets disponibles.<br>2. Presionamos `S` varias veces seguidas para avanzar.<br>3. Presionamos `A` reiteradas veces para retroceder a tweets anteriores.<br>4. Volvemos a presionar `S`. | La aplicacion recorre el feed de a una publicacion por vez en ambos sentidos usando la lista doblemente enlazada, permitiendo avanzar y retroceder sin reiniciar la lectura desde el inicio del feed. | **OK** |
| **19** | **Reiniciar el programa para confirmar que la verificacion no se pierde** | 1. Iniciamos sesion con `@usuarioVer` y comprobamos que figura como `[VERIFICADO]`.<br>2. Cerramos sesion (`0`) y salimos del programa (`0`).<br>3. Volvemos a abrir Cwitter e iniciamos sesion con `@usuarioVer`. | El usuario mantiene su estado de verificado leido desde disco (`usuarios.dat` e `indice_usuarios.idx`), confirmando que la verificacion no se pierde tras cerrar la aplicacion. | **OK** |

---

## Casos de Prueba adicionales - Sprint 2

| # | Situación a probar | Pasos de la operacion | Salida esperada | Estado |
| :---: | :--- | :--- | :--- | :---: |
| **20** | **Publicacion de usuario no verificado no se reporta a FameCheck** | 1. Iniciamos sesion con una cuenta no verificada (`@usuario`).<br>2. Publicamos un nuevo tweet.<br>3. Revisamos el archivo `datos/log_fallos.txt`. | El tweet se publica localmente con normalidad y aparece en el feed; la aplicacion no intenta enviarlo a FameCheck (solo se reportan usuarios verificados) y no se genera ningun registro en el log de fallos. | **OK** |
| **21** | **Modificacion de contraseña de usuario verificado mantiene verificacion** | 1. Iniciamos sesion con `@usuarioVer`.<br>2. Elegimos la opcion 6 "Modificar contrasenia" y asignamos una clave nueva.<br>3. Cerramos sesion e iniciamos sesion con la nueva clave. | La contraseña se actualiza correctamente en disco, el login es exitoso y la cuenta conserva su condicion de `[VERIFICADO]` tanto en memoria como en disco. | **OK** |
