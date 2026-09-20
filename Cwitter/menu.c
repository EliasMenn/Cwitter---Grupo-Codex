#include "menu.h"
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
        printf("\n>> Usuario registrado con exito!\n");
        break;
    case USUARIO_VACIO:
        printf("\n>> Error: El nombre de usuario no puede estar vacio.\n");
        break;
    case USUARIO_DEMASIADO_CORTO:
        printf("\n>> Error: El nombre de usuario debe tener al menos %d caracteres.\n", MIN_USUARIO);
        break;
    case USUARIO_DEMASIADO_LARGO:
        printf("\n>> Error: El nombre de usuario supera el limite permitido (%d caracteres).\n", MAX_USUARIO - 1);
        break;
    case CONTRASENIA_VACIA:
        printf("\n>> Error: La contrasenia no puede estar vacia.\n");
        break;
    case CONTRASENIA_DEMASIADO_CORTA:
        printf("\n>> Error: La contrasenia debe tener al menos %d caracteres.\n", MIN_CONTRASENIA);
        break;
    case CONTRASENIA_DEMASIADO_LARGA:
        printf("\n>> Error: La contrasenia supera el limite permitido (%d caracteres).\n", MAX_CONTRASENIA - 1);
        break;
    case USUARIO_DUPLICADO:
        printf("\n>> Error: Ya existe un usuario registrado con ese nombre.\n");
        break;
    case USUARIO_SIN_MEMORIA:
        printf("\n>> Error: No hay memoria suficiente en el sistema.\n");
        break;
    default:
        printf("\n>> Error: Datos invalidos recibidos.\n");
        break;
    }
}

static void mostrar_error_login(eLoginRet res)
{
    switch(res)
    {
    case LOGIN_OK:
        printf("\n>> Inicio de sesion correcto.\n");
        break;
    case LOGIN_CREDENCIALES_INCORRECTAS:
        printf("\n>> Error: Usuario o contrasenia incorrectos (o cuenta dada de baja).\n");
        break;
    case LOGIN_USUARIO_NO_ENCONTRADO:
        printf("\n>> Error: El usuario ingresado no existe.\n");
        break;
    default:
        printf("\n>> Error: Datos de inicio de sesion invalidos.\n");
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
        limpiar_pantalla();
        printf("+========================================+\n");
        printf("|      << CWITTER - MENU USUARIO >>      |\n");
        printf("+========================================+\n");
        printf("| Sesion activa: @%-22s |\n", usuarioLogueado->usuario);
        printf("+========================================+\n");
        printf("|  1. Ver Feed de publicaciones          |\n");
        printf("|  2. Nueva publicacion                  |\n");
        printf("|  3. Modificar contrasenia              |\n");
        printf("|  4. Dar de baja mi cuenta              |\n");
        printf("|  0. Cerrar sesion                      |\n");
        printf("+========================================+\n");
        printf("Opcion: ");

        if(scanf("%d", &opcion) != 1)
        {
            limpiar_buffer();
            opcion = -1;
            printf("\n>> Opcion invalida. Intente de nuevo.\n");
            pausar();
            continue;
        }
        limpiar_buffer();

        switch(opcion)
        {
        case 1:
            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|       << FEED DE PUBLICACIONES >>      |\n");
            printf("+========================================+\n");
            printf("\n[AVISO] Agregar funcion\n");
            pausar();
            break;

        case 2:
            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|         << NUEVA PUBLICACION >>        |\n");
            printf("+========================================+\n");
            printf("\n[AVISO] Agregar funcion\n");
            pausar();
            break;

        case 3:
            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|        << MODIFICAR CONTRASENIA >>     |\n");
            printf("+========================================+\n\n");
            leer_cadena("Ingrese su nueva contrasenia: ", nuevaClave, MAX_CONTRASENIA);

            if(strlen(nuevaClave) < MIN_CONTRASENIA)
            {
                printf("\n>> Error: La nueva contrasenia debe tener al menos %d caracteres.\n", MIN_CONTRASENIA);
            }
            else
            {
                if(usuario_modificar_contrasenia(indicesUsuarios, usuarioLogueado->usuario, nuevaClave))
                {
                    strcpy(usuarioLogueado->contrasenia, nuevaClave);
                    printf("\n>> Contrasenia modificada exitosamente en disco.\n");
                }
                else
                {
                    printf("\n>> Error al actualizar la contrasenia en almacenamiento.\n");
                }
            }
            pausar();
            break;

        case 4:
            limpiar_pantalla();
            printf("+========================================+\n");
            printf("|          << DAR DE BAJA CUENTA >>      |\n");
            printf("+========================================+\n\n");
            leer_cadena("¿Esta seguro que desea dar de baja su cuenta? (S/N): ", confirmacion, sizeof(confirmacion));

            if(confirmacion[0] == 'S' || confirmacion[0] == 's')
            {
                if(usuario_dar_baja(indicesUsuarios, pilaLibres, usuarioLogueado->usuario))
                {
                    printf("\n>> Su cuenta ha sido dada de baja exitosamente.\n");
                    printf(">> Cerrando sesion automatica...\n");
                    pausar();
                    return; // Retornar al menu principal
                }
                else
                {
                    printf("\n>> Error al procesar la baja de usuario.\n");
                    pausar();
                }
            }
            else
            {
                printf("\n>> Operacion de baja cancelada.\n");
                pausar();
            }
            break;

        case 0:
            printf("\n>> Cerrando sesion de @%s...\n", usuarioLogueado->usuario);
            pausar();
            break;

        default:
            printf("\n>> Opcion no valida.\n");
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
            printf("\n>> Opcion invalida. Intente de nuevo.\n");
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
            printf("\n>> Opcion no valida.\n");
            pausar();
            break;
        }

    } while(opcion != 0);
}
