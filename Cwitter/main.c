#include <stdio.h>
#include <stdlib.h>
#include "usuarios.h"
#include "publicaciones.h"
#include "almacenamiento.h"

void mostrar_indice(void *dato, void *extra)
{
    tIndiceUsuario *indice = (tIndiceUsuario *)dato;

    (void)extra;

    printf(
        "ID: %u | Usuario: %s | Estado: %c | OffsetDat: %ld | OffsetIdx: %ld\n",
        indice->id,
        indice->usuario,
        indice->estado,
        indice->offsetDat,
        indice->offsetIdx
    );
}

void mostrar_resultado(eUsuarioRet resultado)
{
    switch(resultado)
    {
    case USUARIO_REGISTRO_OK:
        printf("Usuario registrado correctamente.\n");
        break;

    case USUARIO_DATOS_INVALIDOS:
        printf("Los datos recibidos son invalidos.\n");
        break;

    case USUARIO_VACIO:
        printf("El nombre de usuario esta vacio.\n");
        break;

    case USUARIO_DEMASIADO_LARGO:
        printf("El nombre de usuario es demasiado largo.\n");
        break;

    case CONTRASENIA_VACIA:
        printf("La contrasenia esta vacia.\n");
        break;

    case CONTRASENIA_DEMASIADO_LARGA:
        printf("La contrasenia es demasiado larga.\n");
        break;

    case USUARIO_DUPLICADO:
        printf("El nombre de usuario ya existe.\n");
        break;

    case USUARIO_DEMASIADO_CORTO:
        printf("El nombre de usuario debe tener al menos 6 caracteres.\n");
        break;

    case CONTRASENIA_DEMASIADO_CORTA:
        printf("La contrasenia debe tener al menos 6 caracteres.\n");
        break;

    case USUARIO_SIN_MEMORIA:
        printf("No hay memoria suficiente.\n");
        break;
    }
}
int main()
{
    tLista indicesUsuarios;
    tPila pilaLibres;
    unsigned ultimoId = 0;
    eUsuarioRet resultado;

    tFeed feed;
    iniciarFeed(&feed);

    lista_crear(&indicesUsuarios);
    crearPila(&pilaLibres);

    // Cargar indices desde disco (si existen) y armar pila de libres
    printf("Cargando base de usuarios...\n");
    almacenamiento_cargar_indices(&indicesUsuarios, &pilaLibres, &ultimoId);
    printf("Ultimo ID encontrado en disco: %u\n", ultimoId);

    printf("\n--- Probando Registros ---\n");
    printf("Caso 1: registro correcto franco\n");
    resultado = usuario_registrar(&indicesUsuarios, &pilaLibres, &ultimoId, "franco", "clave123");
    mostrar_resultado(resultado);

    printf("\nCaso 2: segundo usuario correcto anaana\n");
    resultado = usuario_registrar(&indicesUsuarios, &pilaLibres, &ultimoId, "anaana", "prueba456");
    mostrar_resultado(resultado);

    printf("\nCaso 3: usuario duplicado\n");
    resultado = usuario_registrar(&indicesUsuarios, &pilaLibres, &ultimoId, "franco", "otraClave");
    mostrar_resultado(resultado);

    printf("\nIndices de usuarios en memoria:\n");
    lista_recorrer(&indicesUsuarios, mostrar_indice, NULL);

    tUsuario usuarioLogueado;
    eLoginRet retLogin;

    printf("\n--- Probando Modificaciones ---\n");
    printf("Modificando contraseña de 'anaana' a 'nuevaclave123'...\n");
    if(usuario_modificar_contrasenia(&indicesUsuarios, "anaana", "nuevaclave123"))
    {
        printf("Contraseña modificada en el disco.\n");
    }

    printf("Intentando loguear con 'anaana' y la clave VIEJA ('prueba456')...\n");
    retLogin = usuario_logear(&indicesUsuarios, "anaana", "prueba456", &usuarioLogueado);
    if(retLogin == LOGIN_OK)
    {
        printf("Login OK! Bienvenido %s\n", usuarioLogueado.usuario);
    }
    else
    {
        printf("Login fallido correctamente. Codigo: %d\n", retLogin);
    }

    printf("Intentando loguear con 'anaana' y la clave NUEVA ('nuevaclave123')...\n");
    retLogin = usuario_logear(&indicesUsuarios, "anaana", "nuevaclave123", &usuarioLogueado);
    if(retLogin == LOGIN_OK)
    {
        printf("Login OK! Bienvenido %s\n", usuarioLogueado.usuario);
    }
    else
    {
        printf("Login fallido. Codigo: %d\n", retLogin);
    }

    printf("\n--- Probando Bajas y Reutilizacion ---\n");
    printf("Dando de baja a 'franco'...\n");
    usuario_dar_baja(&indicesUsuarios, &pilaLibres, "franco");

    printf("\nIndices de usuarios en memoria tras la baja:\n");
    lista_recorrer(&indicesUsuarios, mostrar_indice, NULL);

    printf("\nIntentando loguear con 'franco' dado de baja...\n");
    retLogin = usuario_logear(&indicesUsuarios, "franco", "clave123", &usuarioLogueado);
    if(retLogin == LOGIN_OK)
    {
        printf("Login OK! Bienvenido %s\n", usuarioLogueado.usuario);
    }
    else
    {
        printf("Login fallido. Codigo: %d\n", retLogin);
    }

    printf("\nCaso 4: registrando 'nuevoUser' para verificar reciclaje de espacio...\n");
    resultado = usuario_registrar(&indicesUsuarios, &pilaLibres, &ultimoId, "nuevoUser", "nueva123");
    mostrar_resultado(resultado);

    guardarPosteo(usuarioLogueado);

    siguientePosteo(&feed);
    siguientePosteo(&feed);

    printf("\nIndices de usuarios tras registrar al nuevo:\n");
    lista_recorrer(&indicesUsuarios, mostrar_indice, NULL);

    printf("\nUltimo ID utilizado total: %u\n", ultimoId);

    vaciarLista(&feed.p);
    lista_vaciar(&indicesUsuarios);
    vaciarPila(&pilaLibres);

    return 0;
}
