#ifndef USUARIOS_H_INCLUDED
#define USUARIOS_H_INCLUDED

#include "global/lista.h"
#include "global/pila.h"

#define MIN_USUARIO 6
#define MAX_USUARIO 21

#define MIN_CONTRASENIA 6
#define MAX_CONTRASENIA 21

typedef struct
{
    unsigned id;
    char usuario[MAX_USUARIO];
    char contrasenia[MAX_CONTRASENIA];
    char estado; // 'A' activo, 'B' baja logica
} tUsuario;

typedef struct
{
    unsigned id;
    char usuario[MAX_USUARIO];
    long offsetDat; // Posicion en usuarios.dat
    long offsetIdx; // Posicion en indice_usuarios.idx (para la baja logica)
    char estado;
} tIndiceUsuario;

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

typedef enum
{
    LOGIN_OK,
    LOGIN_DATOS_INVALIDOS,
    LOGIN_CREDENCIALES_INCORRECTAS,
    LOGIN_USUARIO_NO_ENCONTRADO
} eLoginRet;



eLoginRet usuario_logear(tLista *listaIndices, char *usuario, char *contrasenia, tUsuario *usuarioLogueado);
eUsuarioRet usuario_registrar(tLista *listaIndices, tPila *pilaLibres, unsigned *ultimoId, const char *usuario, const char *contrasenia);
int usuario_dar_baja(tLista *listaIndices, tPila *pilaLibres, const char *usuario);
int usuario_modificar_contrasenia(tLista *listaIndices, const char *usuario, const char *nuevaContrasenia);
int usuario_comparar(const void *datoA, const void *datoB);
int usuario_comparar_login(const void *datoA, const void *datoB);

#endif