#include "usuarios.h"
#include "almacenamiento.h"
#include <string.h>

int usuario_comparar( const void *datoA, const void *datoB)
{
    const tIndiceUsuario *usuarioA = (const tIndiceUsuario *)datoA;
    const tIndiceUsuario *usuarioB = (const tIndiceUsuario *)datoB;

    return strcmp(usuarioA->usuario, usuarioB->usuario);
}

eLoginRet usuario_logear(tLista *listaIndices, char *usuario, char *contrasenia, tUsuario *usuarioLogueado)
{
    tIndiceUsuario indiceBuscado;
    int resultado;

    if(listaIndices == NULL || usuario == NULL || contrasenia == NULL || usuarioLogueado == NULL)
    {
        return LOGIN_DATOS_INVALIDOS;
    }

    if(strlen(usuario) >= MAX_USUARIO || strlen(contrasenia) >= MAX_CONTRASENIA)
    {
        return LOGIN_CREDENCIALES_INCORRECTAS;
    }

    strcpy(indiceBuscado.usuario, usuario);

    resultado = lista_buscar(listaIndices, &indiceBuscado, sizeof(tIndiceUsuario), usuario_comparar);

    if(resultado != LISTA_TODO_OK)
    {
        return LOGIN_USUARIO_NO_ENCONTRADO;
    }

    if(!almacenamiento_leer_usuario_offset(indiceBuscado.offsetDat, usuarioLogueado))
    {
        return LOGIN_DATOS_INVALIDOS;
    }

    if(usuarioLogueado->estado == 'B' || strcmp(usuarioLogueado->usuario, usuario) != 0 || strcmp(usuarioLogueado->contrasenia, contrasenia) != 0)
    {
        return LOGIN_CREDENCIALES_INCORRECTAS;
    }

    return LOGIN_OK;
}

eUsuarioRet usuario_registrar( tLista *listaIndices, tPila *pilaLibres, unsigned *ultimoId, const char *usuario, const char *contrasenia)
{
    tUsuario nuevoUsuario;
    tIndiceUsuario nuevoIndice;
    int resultado;

    if( listaIndices == NULL || ultimoId == NULL || usuario == NULL || contrasenia == NULL)
    {
        return USUARIO_DATOS_INVALIDOS;
    }

    if(usuario[0] == '\0')
    {
        return USUARIO_VACIO;
    }

    if(strlen(usuario) > MAX_USUARIO-1)
    {
        return USUARIO_DEMASIADO_LARGO;
    }

    if(strlen(usuario) < MIN_USUARIO)
    {
        return USUARIO_DEMASIADO_CORTO;
    }

    if(contrasenia[0] == '\0')
    {
        return CONTRASENIA_VACIA;
    }

    if(strlen(contrasenia) > MAX_CONTRASENIA-1)
    {
        return CONTRASENIA_DEMASIADO_LARGA;
    }

    if(strlen(contrasenia) < MIN_CONTRASENIA)
    {
        return CONTRASENIA_DEMASIADO_CORTA;
    }

    strcpy(nuevoIndice.usuario, usuario);
    if(lista_buscar(listaIndices, &nuevoIndice, sizeof(tIndiceUsuario), usuario_comparar) == LISTA_TODO_OK)
    {
        return USUARIO_DUPLICADO;
    }

    nuevoUsuario.id = *ultimoId + 1;
    strcpy(nuevoUsuario.usuario, usuario);
    strcpy(nuevoUsuario.contrasenia, contrasenia);
    nuevoUsuario.estado = 'A';

    nuevoIndice.id = nuevoUsuario.id;
    nuevoIndice.estado = 'A';

    if(!almacenamiento_guardar_nuevo_usuario(&nuevoUsuario, &nuevoIndice, pilaLibres))
    {
        return USUARIO_SIN_MEMORIA;
    }

    resultado = lista_insertar_en_orden( listaIndices, &nuevoIndice, sizeof(tIndiceUsuario), INSERTAR_SIN_DUP, usuario_comparar );

    if(resultado == LISTA_SIN_MEM)
    {
        return USUARIO_SIN_MEMORIA;
    }

    *ultimoId = nuevoUsuario.id;

    return USUARIO_REGISTRO_OK;
}

void baja_logica_indice(void *dato, void *extra)
{
    tIndiceUsuario *indice = (tIndiceUsuario *)dato;
    const char *usuarioBuscado = (const char *)extra;

    if(strcmp(indice->usuario, usuarioBuscado) == 0)
    {
        indice->estado = 'B';
    }
}

int usuario_dar_baja(tLista *listaIndices, tPila *pilaLibres, const char *usuario)
{
    tIndiceUsuario indiceBuscado;

    if(listaIndices == NULL || pilaLibres == NULL || usuario == NULL)
    {
        return 0;
    }

    strcpy(indiceBuscado.usuario, usuario);

    if(lista_buscar(listaIndices, &indiceBuscado, sizeof(tIndiceUsuario), usuario_comparar) == LISTA_TODO_OK)
    {
        // Dar de baja en el disco y guardar lugar en pila de espacios libres
        almacenamiento_baja_usuario(indiceBuscado.offsetDat, indiceBuscado.offsetIdx, pilaLibres);

        // Modificar el estado en la lista en el indice
        lista_recorrer(listaIndices, baja_logica_indice, (void *)usuario);
    }

    return 1;
}

int usuario_modificar_contrasenia(tLista *listaIndices, const char *usuario, const char *nuevaContrasenia)
{
    tIndiceUsuario indiceBuscado;
    tUsuario usuarioFisico;

    if(listaIndices == NULL || usuario == NULL || nuevaContrasenia == NULL)
    {
        return 0;
    }

    strcpy(indiceBuscado.usuario, usuario);

    if(lista_buscar(listaIndices, &indiceBuscado, sizeof(tIndiceUsuario), usuario_comparar) == LISTA_TODO_OK)
    {
        if(almacenamiento_leer_usuario_offset(indiceBuscado.offsetDat, &usuarioFisico))
        {
            strcpy(usuarioFisico.contrasenia, nuevaContrasenia);
            return almacenamiento_actualizar_usuario(indiceBuscado.offsetDat, &usuarioFisico);
        }
    }

    return 0;
}
