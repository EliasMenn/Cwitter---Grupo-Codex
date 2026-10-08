#include "menu.h"
#include "publicaciones.h"
#include "almacenamiento.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void limpiar_pantalla(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void limpiar_buffer(void)
{
    int c;
    while((c = getchar()) != '\n' && c != EOF)
    {
        // Descartar caracteres restantes
    }
}

static void pausar(void)
{
    printf("\nPresione ENTER para continuar...");
    getchar();
}

static void leer_cadena(const char *mensaje, char *destino, int maxTam)
{
    size_t len;

    printf("%s", mensaje);
    if(fgets(destino, maxTam, stdin) != NULL)
    {
        len = strlen(destino);
        if(len > 0 && destino[len - 1] == '\n')
        {
            destino[len - 1] = '\0';
        }
        else
        {
            limpiar_buffer();
        }
    }
    else
    {
        destino[0] = '\0';
    }
}

static void mostrar_error_registro(eUsuarioRet res)
{
    switch(res)
    {
    case USUARIO_REGISTRO_OK:
        printf("\nUsuario registrado con exito\n");
        break;
    case USUARIO_VACIO:
        printf("\nError: El nombre de usuario no puede estar vacio.\n");
        break;
    case USUARIO_DEMASIADO_CORTO:
        printf("\nError: El nombre de usuario debe tener al menos %d caracteres.\n", MIN_USUARIO);
        break;
    case USUARIO_DEMASIADO_LARGO:
        printf("\nError: El nombre de usuario supera el limite permitido (%d caracteres).\n", MAX_USUARIO - 1);
        break;
    case CONTRASENIA_VACIA:
        printf("\nError: La contrasenia no puede estar vacia.\n");
        break;
    case CONTRASENIA_DEMASIADO_CORTA:
        printf("\nError: La contrasenia debe tener al menos %d caracteres.\n", MIN_CONTRASENIA);
        break;
    case CONTRASENIA_DEMASIADO_LARGA:
        printf("\nError: La contrasenia supera el limite permitido (%d caracteres).\n", MAX_CONTRASENIA - 1);
        break;
    case USUARIO_DUPLICADO:
        printf("\nError: Ya existe un usuario registrado con ese nombre.\n");
        break;
    case USUARIO_SIN_MEMORIA:
        printf("\nError: No hay memoria suficiente en el sistema.\n");
        break;
    default:
        printf("\nError: Datos invalidos recibidos.\n");
        break;
    }
}

static void mostrar_error_login(eLoginRet res)
{
    switch(res)
    {
    case LOGIN_OK:
        printf("\n Inicio de sesion correcto.\n");
        break;
    case LOGIN_CREDENCIALES_INCORRECTAS:
        printf("\nError: Usuario o contrasenia incorrectos (o cuenta dada de baja).\n");
        break;
    case LOGIN_USUARIO_NO_ENCONTRADO:
        printf("\nError: El usuario ingresado no existe.\n");
        break;
    default:
        printf("\nError: Datos de inicio de sesion invalidos.\n");
        break;
    }
}

static void menu_usuario_autenticado(tLista *indicesUsuarios, tPila *pilaLibres, tUsuario *usuarioLogueado)
{
    int opcion = -1;
    char nuevaClave[MAX_CONTRASENIA];
    char confirmacion[10];

    do
    {
        combinarPosteos();
        limpiar_pantalla();
        printf("+========================================+\n");
        printf("|      << CWITTER - MENU USUARIO >>      |\n");
        printf("+========================================+\n");
        printf("| Sesion activa: @%-15s %s |\n",
       usuarioLogueado->usuario,
       usuarioLogueado->verificado == 'S' ? "[VERIFICADO]" : "            ");
        printf("+========================================+\n");
        printf("|  1. Ver Feed de publicaciones          |\n");
        printf("|  2. Nueva publicacion                  |\n");
        printf("|  3. Buscar publicaciones               |\n");
        printf("|  4. Editar una publicacion             |\n");
        printf("|  5. Eliminar una publicacion           |\n");
        printf("|  6. Modificar contrasenia              |\n");
        printf("|  7. Dar de baja mi cuenta              |\n");
        printf("|  0. Cerrar sesion                      |\n");
        printf("+========================================+\n");
        printf("Opcion: ");

        if(scanf("%d", &opcion) != 1)
        {
            limpiar_buffer();
            opcion = -1;
            printf("\nOpcion invalida. Intente de nuevo.\n");
            pausar();
            continue;
        }
        limpiar_buffer();

        switch(opcion)
        {
        case 1:
        {
            tFeed feed;
            char tecla;
            iniciarFeed(&feed);

            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|       << FEED DE PUBLICACIONES >>      |\n");
            printf("+========================================+\n\n");

            if(feed.p == NULL)
            {
                printf("No hay publicaciones en el feed todavia.\n\n");
                pausar();
            }
            else
            {
                printf("--- Publicacion actual ---\n");
                siguientePosteo(&feed);

                do
                {
                    printf("\n------------------------------------------\n");
                    printf("[S] Siguiente | [A] Anterior | [0] Volver\nOpcion: ");
                    if(scanf(" %c", &tecla) != 1)
                    {
                        limpiar_buffer();
                        break;
                    }
                    limpiar_buffer();

                    if(tecla == 'S' || tecla == 's')
                    {
                        limpiar_pantalla();
                        printf("+========================================+\n");
                        printf("|       << FEED DE PUBLICACIONES >>      |\n");
                        printf("+========================================+\n\n");
                        printf("--- Siguiente publicacion ---\n");
                        siguientePosteo(&feed);
                    }
                    else if(tecla == 'A' || tecla == 'a')
                    {
                        limpiar_pantalla();
                        printf("+========================================+\n");
                        printf("|       << FEED DE PUBLICACIONES >>      |\n");
                        printf("+========================================+\n\n");
                        printf("--- Publicacion anterior ---\n");
                        posteoAnterior(&feed);
                    }
                } while(tecla != '0');
            }

            vaciarLista(&feed.p);
            break;
        }

        case 2:
            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|         << NUEVA PUBLICACION >>        |\n");
            printf("+========================================+\n\n");
            printf("Escribe tu tweet (Max 140 caracteres):\n> ");
            if(guardarPosteo(*usuarioLogueado) == 0)
            {
                printf("\nPublicacion guardada exitosamente.\n");
            }
            else
            {
                printf("\nError al guardar la publicacion.\n");
            }
            pausar();
            break;

        case 3:
        {
            tFeed feedFiltro;
            tPosteo patronFiltro;
            char termino[LARGO_MAX];
            char tecla;

            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|       << BUSCAR PUBLICACIONES >>       |\n");
            printf("+========================================+\n\n");
            printf("Ingrese texto o patron a buscar (ej: '*palabra*'):\n> ");
            leer_cadena("", termino, sizeof(termino));

            if(strlen(termino) == 0)
            {
                printf("\nBusqueda cancelada (termino vacio).\n");
                pausar();
                break;
            }

            strcpy(patronFiltro.publicacion, termino);
            cargarPostsFiltrados(&feedFiltro, cmpTexto, &patronFiltro);

            if(feedFiltro.p == NULL)
            {
                printf("\nNo se encontraron publicaciones con '%s'.\n\n", termino);
                pausar();
            }
            else
            {
                limpiar_pantalla();
                printf("+========================================+\n");
                printf("|     << RESULTADOS DE LA BUSQUEDA >>    |\n");
                printf("+========================================+\n");
                printf("Filtro: \"%s\"\n\n", termino);
                feedFiltro.posteo_actual = 0;
                siguientePosteoFiltrados(&feedFiltro);

                do
                {
                    printf("\n------------------------------------------\n");
                    printf("[S] Siguiente | [A] Anterior | [0] Volver\nOpcion: ");
                    if(scanf(" %c", &tecla) != 1)
                    {
                        limpiar_buffer();
                        break;
                    }
                    limpiar_buffer();

                    if(tecla == 'S' || tecla == 's')
                    {
                        limpiar_pantalla();
                        printf("+========================================+\n");
                        printf("|     << RESULTADOS DE LA BUSQUEDA >>    |\n");
                        printf("+========================================+\n");
                        printf("Filtro: \"%s\"\n\n", termino);
                        siguientePosteoFiltrados(&feedFiltro);
                    }
                    else if(tecla == 'A' || tecla == 'a')
                    {
                        limpiar_pantalla();
                        printf("+========================================+\n");
                        printf("|     << RESULTADOS DE LA BUSQUEDA >>    |\n");
                        printf("+========================================+\n");
                        printf("Filtro: \"%s\"\n\n", termino);
                        posteoAnteriorFiltrado(&feedFiltro);
                    }
                } while(tecla != '0');

                vaciarLista(&feedFiltro.p);
            }
            break;
        }

        case 4:
        {
            tFeed feedFiltro;
            tPosteo patronFiltro;
            char termino[LARGO_MAX];
            char tecla;
            char temp[LARGO_MAX + 4];// para evitar agregar los asteriscos manualmente le agregue esos 4 bits que ocuparian los astericos y la barra cero, basicamente que no se trunque
            int i;

            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|       << EDITAR UNA PUBLICACION >>     |\n");
            printf("+========================================+\n\n");
            printf("Ingrese una palabra clave o parte del tweet a editar:\n> ");
            leer_cadena("", termino, sizeof(termino));

            if(strlen(termino) == 0)
            {
                printf("\nBusqueda cancelada (termino vacio).\n");
                pausar();
                break;
            }

            for(i = 0; termino[i] != '\0'; i++)
            {
                if(termino[i] >= 'A' && termino[i] <= 'Z')
                {
                    termino[i] = termino[i] + 32;
                }
            }

            if(strchr(termino, '*') == NULL)
            {
                snprintf(temp, sizeof(temp), "*%s*", termino);
                strcpy(patronFiltro.publicacion, temp);
            }
            else
            {
                strcpy(patronFiltro.publicacion, termino);
            }

            strcpy(patronFiltro.nombreUsuario, usuarioLogueado->usuario);
            cargarPostsFiltrados(&feedFiltro, cmpTexto, &patronFiltro);

            if(feedFiltro.p == NULL)
            {
                printf("\nNo se encontraron publicaciones tuyas con '%s'.\n\n", termino);
                pausar();
            }
            else
            {
                limpiar_pantalla();
                printf("+========================================+\n");
                printf("|     << RESULTADOS DE LA BUSQUEDA >>    |\n");
                printf("+========================================+\n");
                printf("Filtro: \"%s\"\n\n", termino);
                feedFiltro.posteo_actual = 0;
                siguientePosteoFiltrados(&feedFiltro);

                do
                {
                    printf("\n------------------------------------------\n");
                    printf("[S] Siguiente | [A] Anterior | [E] Proceder a Editar | [0] Volver\nOpcion: ");
                    if(scanf(" %c", &tecla) != 1)
                    {
                        limpiar_buffer();
                        break;
                    }
                    limpiar_buffer();

                    if(tecla == 'S' || tecla == 's')
                    {
                        limpiar_pantalla();
                        printf("+========================================+\n");
                        printf("|     << RESULTADOS DE LA BUSQUEDA >>    |\n");
                        printf("+========================================+\n");
                        printf("Filtro: \"%s\"\n\n", termino);
                        siguientePosteoFiltrados(&feedFiltro);
                    }
                    else if(tecla == 'A' || tecla == 'a')
                    {
                        limpiar_pantalla();
                        printf("+========================================+\n");
                        printf("|     << RESULTADOS DE LA BUSQUEDA >>    |\n");
                        printf("+========================================+\n");
                        printf("Filtro: \"%s\"\n\n", termino);
                        posteoAnteriorFiltrado(&feedFiltro);
                    }
                    else if(tecla == 'E' || tecla == 'e')
                    {
                        unsigned idAEditar;
                        char nuevoTexto[LARGO_MAX];
                        char confirmacionEdicion[10];
                        tPosteo posteoActual;
                        int tienePosteoActual;
                        int resEdicion;

                        idAEditar = 0;
                        tienePosteoActual = 0;

                        if(feedFiltro.posteo_actual > 0)
                        {
                            tienePosteoActual = (obtenerPosicionN(&feedFiltro.p, &posteoActual, sizeof(tPosteo), feedFiltro.posteo_actual - 1) == 0);
                        }

                        vaciarLista(&feedFiltro.p);

                        limpiar_pantalla();
                        printf("+========================================+\n");
                        printf("|       << EDITAR UNA PUBLICACION >>     |\n");
                        printf("+========================================+\n\n");

                        if(tienePosteoActual)
                        {
                            printf("Tweet seleccionado para editar:\n");
                            mostrarPosteo(posteoActual);
                            printf("\n----------------------------------------\n\n");
                        }

                        printf("Ingrese el ID del tweet a editar: ");

                        if(scanf("%u", &idAEditar) != 1)
                        {
                            limpiar_buffer();
                            printf("\nID invalido.\n");
                            pausar();
                            break;
                        }
                        limpiar_buffer();

                        leer_cadena("Ingrese el nuevo texto del tweet (Max 140 caracteres):\n> ", nuevoTexto, sizeof(nuevoTexto));

                        if(strlen(nuevoTexto) == 0)
                        {
                            printf("\nError: El texto no puede estar vacio.\n");
                            pausar();
                            break;
                        }

                        leer_cadena("¿Confirmar edicion del tweet? (S/N): ", confirmacionEdicion, sizeof(confirmacionEdicion));
                        if(confirmacionEdicion[0] == 'S' || confirmacionEdicion[0] == 's')
                        {
                            resEdicion = almacenamiento_editar_posteo(idAEditar, usuarioLogueado->usuario, nuevoTexto);
                            if(resEdicion == 1)
                            {
                                printf("\nTweet #%u editado exitosamente.\n", idAEditar);
                            }
                            else if(resEdicion == -1)
                            {
                                printf("\nError: No tiene permisos para editar este tweet.\n");
                            }
                            else if(resEdicion == 0)
                            {
                                printf("\nError: No se encontro ningun tweet tuyo con el ID #%u.\n", idAEditar);
                            }
                            else
                            {
                                printf("\nError al procesar la edicion.\n");
                            }
                        }
                        else
                        {
                            printf("\nOperacion de edicion cancelada.\n");
                        }
                        pausar();
                        break;
                    }
                } while(tecla != '0');

                vaciarLista(&feedFiltro.p);
            }
            break;
        }

        case 5:
        {
            tFeed feedFiltro;
            tPosteo patronFiltro;
            char termino[LARGO_MAX];
            char tecla;
            char temp[LARGO_MAX + 4];
            int i;

            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|      << ELIMINAR UNA PUBLICACION >>    |\n");
            printf("+========================================+\n\n");
            printf("Ingrese una palabra clave o parte del tweet:\n> ");
            leer_cadena("", termino, sizeof(termino));

            if(strlen(termino) == 0)
            {
                printf("\nBusqueda cancelada (termino vacio).\n");
                pausar();
                break;
            }

            for(i = 0; termino[i] != '\0'; i++)
            {
                if(termino[i] >= 'A' && termino[i] <= 'Z')
                {
                    termino[i] = termino[i] + 32;
                }
            }

            if(strchr(termino, '*') == NULL)
            {
                snprintf(temp, sizeof(temp), "*%s*", termino);
                strcpy(patronFiltro.publicacion, temp);
            }
            else
            {
                strcpy(patronFiltro.publicacion, termino);
            }

            cargarPostsFiltrados(&feedFiltro, cmpTexto, &patronFiltro);

            if(feedFiltro.p == NULL)
            {
                printf("\nNo se encontraron publicaciones con '%s'.\n\n", termino);
                pausar();
            }
            else
            {
                limpiar_pantalla();
                printf("+========================================+\n");
                printf("|     << RESULTADOS DE LA BUSQUEDA >>    |\n");
                printf("+========================================+\n");
                printf("Filtro: \"%s\"\n\n", termino);
                feedFiltro.posteo_actual = 0;
                siguientePosteoFiltrados(&feedFiltro);

                do
                {
                    printf("\n------------------------------------------\n");
                    printf("[S] Siguiente | [A] Anterior | [D] Proceder a Eliminar | [0] Volver\nOpcion: ");
                    if(scanf(" %c", &tecla) != 1)
                    {
                        limpiar_buffer();
                        break;
                    }
                    limpiar_buffer();

                    if(tecla == 'S' || tecla == 's')
                    {
                        limpiar_pantalla();
                        printf("+========================================+\n");
                        printf("|     << RESULTADOS DE LA BUSQUEDA >>    |\n");
                        printf("+========================================+\n");
                        printf("Filtro: \"%s\"\n\n", termino);
                        siguientePosteoFiltrados(&feedFiltro);
                    }
                    else if(tecla == 'A' || tecla == 'a')
                    {
                        limpiar_pantalla();
                        printf("+========================================+\n");
                        printf("|     << RESULTADOS DE LA BUSQUEDA >>    |\n");
                        printf("+========================================+\n");
                        printf("Filtro: \"%s\"\n\n", termino);
                        posteoAnteriorFiltrado(&feedFiltro);
                    }
                    else if(tecla == 'D' || tecla == 'd')
                    {
                        unsigned idABorrar;
                        char confirmacionBorrado[10];
                        tPosteo posteoActual;
                        int tienePosteoActual;
                        int resElim;

                        idABorrar = 0;
                        tienePosteoActual = 0;

                        if(feedFiltro.posteo_actual > 0)
                        {
                            tienePosteoActual = (obtenerPosicionN(&feedFiltro.p, &posteoActual, sizeof(tPosteo), feedFiltro.posteo_actual - 1) == 0);
                        }

                        vaciarLista(&feedFiltro.p);

                        limpiar_pantalla();
                        printf("+========================================+\n");
                        printf("|      << ELIMINAR UNA PUBLICACION >>    |\n");
                        printf("+========================================+\n\n");

                        if(tienePosteoActual)
                        {
                            printf("Tweet seleccionado para revisar:\n");
                            mostrarPosteo(posteoActual);
                            printf("\n----------------------------------------\n\n");
                        }

                        printf("Ingrese el ID del tweet a eliminar: ");

                        if(scanf("%u", &idABorrar) != 1)
                        {
                            limpiar_buffer();
                            printf("\nID invalido.\n");
                            pausar();
                            break;
                        }
                        limpiar_buffer();

                        leer_cadena("¿Continuar con la eliminacion del tweet? (S/N): ", confirmacionBorrado, sizeof(confirmacionBorrado));
                        if(confirmacionBorrado[0] == 'S' || confirmacionBorrado[0] == 's')
                        {
                            resElim = almacenamiento_eliminar_posteo(idABorrar, usuarioLogueado->usuario);
                            if(resElim == 1)
                            {
                                printf("\nTweet #%u eliminado exitosamente.\n", idABorrar);
                            }
                            else if(resElim == -1)
                            {
                                printf("\nError: No tiene permisos para eliminar este tweet.\n");
                            }
                            else if(resElim == 0)
                            {
                                printf("\nError: No se encontro ningun tweet con el ID #%u.\n", idABorrar);
                            }
                            else
                            {
                                printf("\nError al procesar la eliminacion.\n");
                            }
                        }
                        else
                        {
                            printf("\nOperacion de eliminacion cancelada.\n");
                        }
                        pausar();
                        break;
                    }
                } while(tecla != '0');

                vaciarLista(&feedFiltro.p);
            }
            break;
        }

        case 6:
            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|        << MODIFICAR CONTRASENIA >>     |\n");
            printf("+========================================+\n\n");
            leer_cadena("Ingrese su nueva contrasenia: ", nuevaClave, MAX_CONTRASENIA);

            if(strlen(nuevaClave) < MIN_CONTRASENIA)
            {
                printf("\nError: La nueva contrasenia debe tener al menos %d caracteres.\n", MIN_CONTRASENIA);
            }
            else
            {
                if(usuario_modificar_contrasenia(indicesUsuarios, usuarioLogueado->usuario, nuevaClave))
                {
                    strcpy(usuarioLogueado->contrasenia, nuevaClave);
                    printf("\nContrasenia modificada exitosamente.\n");
                }
                else
                {
                    printf("\nError al actualizar la contrasenia.\n");
                }
            }
            pausar();
            break;

        case 7:
            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|          << DAR DE BAJA CUENTA >>      |\n");
            printf("+========================================+\n\n");
            leer_cadena("¿Esta seguro que desea dar de baja su cuenta? (S/N): ", confirmacion, sizeof(confirmacion));

            if(confirmacion[0] == 'S' || confirmacion[0] == 's')
            {
                if(usuario_dar_baja(indicesUsuarios, pilaLibres, usuarioLogueado->usuario))
                {
                    printf("\nSu cuenta ha sido dada de baja exitosamente.\n");
                    printf("Cerrando sesion...\n");
                    pausar();
                    return; // Retornar al menu principal
                }
                else
                {
                    printf("\nError al procesar la baja de usuario.\n");
                    pausar();
                }
            }
            else
            {
                printf("\nOperacion de baja cancelada.\n");
                pausar();
            }
            break;

        case 0:
            printf("\nCerrando sesion de @%s...\n", usuarioLogueado->usuario);
            pausar();
            combinarPosteos();
            break;

        default:
            printf("\nOpcion invalida.\n");
            pausar();
            break;
        }

    } while(opcion != 0);
}

void menu_iniciar(tLista *indicesUsuarios, tPila *pilaLibres, unsigned *ultimoId)
{
    int opcion = -1;
    char usuario[MAX_USUARIO];
    char contrasenia[MAX_CONTRASENIA];
    tUsuario usuarioLogueado;
    eLoginRet retLogin;
    eUsuarioRet retRegistro;

    do
    {
        limpiar_pantalla();
        printf("+================================+\n");
        printf("|         << CWITTER >>          |\n");
        printf("+================================+\n");
        printf("|  1. Registrarse                |\n");
        printf("|  2. Iniciar sesion             |\n");
        printf("|  0. Salir                      |\n");
        printf("+================================+\n");
        printf("Opcion: ");

        if(scanf("%d", &opcion) != 1)
        {
            limpiar_buffer();
            opcion = -1;
            printf("\nOpcion invalida. Intente de nuevo.\n");
            pausar();
            continue;
        }
        limpiar_buffer();

        switch(opcion)
        {
        case 1:
            limpiar_pantalla();
            printf("+================================+\n");
            printf("|     << REGISTRO USUARIO >>     |\n");
            printf("+================================+\n\n");
            leer_cadena("Elija un nombre de usuario: ", usuario, MAX_USUARIO);
            leer_cadena("Elija una contrasenia: ", contrasenia, MAX_CONTRASENIA);

            retRegistro = usuario_registrar(indicesUsuarios, pilaLibres, ultimoId, usuario, contrasenia);
            mostrar_error_registro(retRegistro);
            pausar();
            break;

        case 2:

            limpiar_pantalla();
            printf("+================================+\n");
            printf("|      << INICIAR SESION >>      |\n");
            printf("+================================+\n\n");
            leer_cadena("Usuario: ", usuario, MAX_USUARIO);
            leer_cadena("Contrasenia: ", contrasenia, MAX_CONTRASENIA);

            retLogin = usuario_logear(indicesUsuarios, usuario, contrasenia, &usuarioLogueado);
            mostrar_error_login(retLogin);

            if(retLogin == LOGIN_OK)
            {
                pausar();
                menu_usuario_autenticado(indicesUsuarios, pilaLibres, &usuarioLogueado);
            }
            else
            {
                pausar();
            }
            break;

        case 0:
            limpiar_pantalla();
            break;

        default:
            printf("\nOpcion invalida.\n");
            pausar();
            break;
        }

    } while(opcion != 0);
}
