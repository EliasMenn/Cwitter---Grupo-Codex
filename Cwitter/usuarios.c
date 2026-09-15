#include "usuarios.h"

#include <string.h>

int usuario_comparar( const void *datoA, const void *datoB)
{
    const tUsuario *usuarioA = (const tUsuario *)datoA;
    const tUsuario *usuarioB = (const tUsuario *)datoB;

    return strcmp(usuarioA->usuario, usuarioB->usuario);
}

int usuario_comparar_login(const void *datoA, const void *datoB)
{
    const tUsuario *usuarioA = (const tUsuario *)datoA;
    const tUsuario *usuarioB = (const tUsuario *)datoB;

    return strcmp(usuarioA->usuario, usuarioB->usuario) || strcmp(usuarioA->contrasenia, usuarioB->contrasenia);
}

eLoginRet usuario_logear(tLista *listaUsuarios,char *usuario,char *contrasenia,tUsuario *usuarioLogueado)
{
    tUsuario usuarioIngresado;
    int resultado;

    if(listaUsuarios == NULL || usuario == NULL || contrasenia == NULL || usuarioLogueado == NULL)
        return LOGIN_DATOS_INVALIDOS;

    if(strlen(usuario) >= MAX_USUARIO || strlen(contrasenia) >= MAX_CONTRASENIA)
        return LOGIN_CREDENCIALES_INCORRECTAS;

    strcpy(usuarioIngresado.usuario, usuario);
    strcpy(usuarioIngresado.contrasenia, contrasenia);

    resultado = lista_buscar(listaUsuarios, &usuarioIngresado, sizeof(tUsuario), usuario_comparar_login);

    if(resultado != LISTA_TODO_OK)
        return LOGIN_CREDENCIALES_INCORRECTAS;

    *usuarioLogueado = usuarioIngresado;

    return LOGIN_OK;
}


eUsuarioRet usuario_registrar( tLista *listaUsuarios, unsigned *ultimoId, const char *usuario, const char *contrasenia)
{
    tUsuario nuevoUsuario;
    int resultado;

    if( listaUsuarios == NULL || ultimoId == NULL || usuario == NULL || contrasenia == NULL)
        return USUARIO_DATOS_INVALIDOS;

    if(usuario[0] == '\0')
        return USUARIO_VACIO;

    if(strlen(usuario) > MAX_USUARIO-1)
        return USUARIO_DEMASIADO_LARGO;

    if(strlen(usuario) < MIN_USUARIO)
        return USUARIO_DEMASIADO_CORTO;

    if(contrasenia[0] == '\0')
        return CONTRASENIA_VACIA;

    if(strlen(contrasenia) > MAX_CONTRASENIA-1)
        return CONTRASENIA_DEMASIADO_LARGA;

    if(strlen(contrasenia) < MIN_CONTRASENIA)
        return CONTRASENIA_DEMASIADO_CORTA;

    nuevoUsuario.id = *ultimoId + 1;

    strcpy(nuevoUsuario.usuario, usuario);
    strcpy(nuevoUsuario.contrasenia, contrasenia);

    resultado = lista_insertar_en_orden( listaUsuarios, &nuevoUsuario, sizeof(tUsuario), INSERTAR_SIN_DUP, usuario_comparar );

    if(resultado == LISTA_DUPLICADO)
        return USUARIO_DUPLICADO;

    if(resultado == LISTA_SIN_MEM)
        return USUARIO_SIN_MEMORIA;

    *ultimoId = nuevoUsuario.id;

    return USUARIO_REGISTRO_OK;
}
