#include "almacenamiento.h"
#include "usuarios.h"
#include <stdio.h>
#include <stdlib.h>

int almacenamiento_guardar_usuarios(const tLista *listaUsuarios)
{
    FILE *archivo = fopen(ARCHIVO_USUARIOS, "wb");
    if(archivo == NULL)
    {
        return 0;
    }

    // Usamos el recorrido de la lista genérica para escribir cada elemento
    lista_recorrer(listaUsuarios, accion_guardar_usuario, archivo);

    fclose(archivo);
    return 1;
}

int almacenamiento_cargar_usuarios(tLista *listaUsuarios, unsigned *ultimoId)
{
    FILE *archivo = fopen(ARCHIVO_USUARIOS, "rb");
    tUsuario aux;
    int leidos;
    unsigned idMax = 0;

    // Si el archivo no existe (primera ejecución), no hay usuarios registrados todavia
    if(archivo == NULL)
    {
        *ultimoId = 0;
        return 1;
    }

    leidos = fread(&aux, sizeof(tUsuario), 1, archivo);
    while(leidos == 1)
    {
        // Cargamos el usuario en la lista en memoria
        lista_insertar_en_orden(listaUsuarios, &aux, sizeof(tUsuario), INSERTAR_SIN_DUP, usuario_comparar);

        // Guardamos cual es el ID más alto para seguir desde ahí cuando se registre un nuevo usuario
        if(aux.id > idMax)
        {
            idMax = aux.id;
        }

        leidos = fread(&aux, sizeof(tUsuario), 1, archivo);
    }

    *ultimoId = idMax;
    fclose(archivo);
    return 1;
}

void accion_guardar_usuario(void *dato, void *extra)
{
    tUsuario *usuario = (tUsuario *)dato;
    FILE *archivo = (FILE *)extra;

    fwrite(usuario, sizeof(tUsuario), 1, archivo);
}
