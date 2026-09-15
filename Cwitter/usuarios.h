#ifndef USUARIOS_H_INCLUDED
#define USUARIOS_H_INCLUDED

#include "global/lista.h"

#define MIN_USUARIO 6
#define MAX_USUARIO 21

#define MIN_CONTRASENIA 6
#define MAX_CONTRASENIA 21

typedef struct
{
    unsigned id;
    char usuario[MAX_USUARIO];
    char contrasenia[MAX_CONTRASENIA];
} tUsuario;

typedef enum
{
    USUARIO_REGISTRO_OK,
    USUARIO_DATOS_INVALIDOS,
    USUARIO_VACIO,
    USUARIO_DEMASIADO_CORTO,
    USUARIO_DEMASIADO_LARGO,
    CONTRASENIA_VACIA,
    CONTRASENIA_DEMASIADO_CORTA,
    CONTRASENIA_DEMASIADO_LARGA,
    USUARIO_DUPLICADO,
    USUARIO_SIN_MEMORIA
} eUsuarioRet;

eUsuarioRet usuario_registrar(tLista *listaUsuarios, unsigned *ultimoId, const char *usuario, const char *contrasenia);
int usuario_comparar(const void *datoA, const void *datoB);
#endif
