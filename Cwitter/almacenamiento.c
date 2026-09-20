#include "almacenamiento.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct {
    long offsetDat;
    long offsetIdx;
} tEspacioLibre;

int almacenamiento_cargar_indices(tLista *listaIndices, tPila *pilaLibres, unsigned *ultimoId)
{
    FILE *archivo = fopen(ARCHIVO_INDICES_USUARIOS, "rb");
    tIndiceUsuario aux;
    int leidos;
    unsigned idMax = 0;

    *ultimoId = 0;

    if(archivo == NULL)
    {
        return 1;
    }

    leidos = fread(&aux, sizeof(tIndiceUsuario), 1, archivo);
    while(leidos == 1)
    {
        if(aux.estado == 'A')
        {
            lista_insertar_en_orden(listaIndices, &aux, sizeof(tIndiceUsuario), INSERTAR_SIN_DUP, usuario_comparar);
            if(aux.id > idMax)
            {
                idMax = aux.id;
            }
        }
        else if(aux.estado == 'B')
        {
            tEspacioLibre hueco;
            hueco.offsetDat = aux.offsetDat;
            hueco.offsetIdx = aux.offsetIdx;
            ponerEnPila(pilaLibres, &hueco, sizeof(tEspacioLibre));
        }

        leidos = fread(&aux, sizeof(tIndiceUsuario), 1, archivo);
    }

    *ultimoId = idMax;
    fclose(archivo);
    return 1;
}

int almacenamiento_guardar_nuevo_usuario(tUsuario *usuario, tIndiceUsuario *indiceACompletar, tPila *pilaLibres)
{
    FILE *archDat = fopen(ARCHIVO_USUARIOS, "r+b");
    FILE *archIdx = fopen(ARCHIVO_INDICES_USUARIOS, "r+b");

    if(archDat == NULL)
    {
        archDat = fopen(ARCHIVO_USUARIOS, "w+b");
    }

    if(archIdx == NULL)
    {
        archIdx = fopen(ARCHIVO_INDICES_USUARIOS, "w+b");
    }

    if(archDat == NULL || archIdx == NULL)
    {
        if(archDat)
        {
            fclose(archDat);
        }
        if(archIdx)
        {
            fclose(archIdx);
        }
        return 0;
    }

    tEspacioLibre hueco;
    long posDat, posIdx;

    if(sacarDePila(pilaLibres, &hueco, sizeof(tEspacioLibre)))
    {
        posDat = hueco.offsetDat;
        posIdx = hueco.offsetIdx;
    }
    else
    {
        fseek(archDat, 0, SEEK_END);
        fseek(archIdx, 0, SEEK_END);
        posDat = ftell(archDat);
        posIdx = ftell(archIdx);
    }

    fseek(archDat, posDat, SEEK_SET);
    fwrite(usuario, sizeof(tUsuario), 1, archDat);

    indiceACompletar->offsetDat = posDat;
    indiceACompletar->offsetIdx = posIdx;

    fseek(archIdx, posIdx, SEEK_SET);
    fwrite(indiceACompletar, sizeof(tIndiceUsuario), 1, archIdx);

    fclose(archDat);
    fclose(archIdx);

    return 1;
}

int almacenamiento_leer_usuario_offset(long offset, tUsuario *usuarioDestino)
{
    FILE *archDat = fopen(ARCHIVO_USUARIOS, "rb");

    if(!archDat)
    {
        return 0;
    }

    fseek(archDat, offset, SEEK_SET);
    int leidos = fread(usuarioDestino, sizeof(tUsuario), 1, archDat);
    fclose(archDat);

    return leidos == 1;
}

int almacenamiento_baja_usuario(long offsetDat, long offsetIdx, tPila *pilaLibres)
{
    FILE *archDat = fopen(ARCHIVO_USUARIOS, "r+b");
    FILE *archIdx = fopen(ARCHIVO_INDICES_USUARIOS, "r+b");

    if(!archDat || !archIdx)
    {
        if(archDat)
        {
            fclose(archDat);
        }
        if(archIdx)
        {
            fclose(archIdx);
        }
        return 0;
    }

    tUsuario usuario;
    tIndiceUsuario indice;
    tEspacioLibre hueco;

    fseek(archDat, offsetDat, SEEK_SET);
    if(fread(&usuario, sizeof(tUsuario), 1, archDat) == 1)
    {
        usuario.estado = 'B';
        fseek(archDat, offsetDat, SEEK_SET);
        fwrite(&usuario, sizeof(tUsuario), 1, archDat);
    }

    fseek(archIdx, offsetIdx, SEEK_SET);
    if(fread(&indice, sizeof(tIndiceUsuario), 1, archIdx) == 1)
    {
        indice.estado = 'B';
        fseek(archIdx, offsetIdx, SEEK_SET);
        fwrite(&indice, sizeof(tIndiceUsuario), 1, archIdx);
    }

    fclose(archDat);
    fclose(archIdx);

    hueco.offsetDat = offsetDat;
    hueco.offsetIdx = offsetIdx;
    ponerEnPila(pilaLibres, &hueco, sizeof(tEspacioLibre));

    return 1;
}

int almacenamiento_actualizar_usuario(long offsetDat, tUsuario *usuarioActualizado)
{
    FILE *archDat = fopen(ARCHIVO_USUARIOS, "r+b");

    if(!archDat)
    {
        return 0;
    }

    fseek(archDat, offsetDat, SEEK_SET);
    fwrite(usuarioActualizado, sizeof(tUsuario), 1, archDat);
    fclose(archDat);

    return 1;
}

int cargarNPosteos(tListaDoble* listaDoble, unsigned offset, unsigned cantPosteos, int Func(tListaDoble* p, void* informacion, size_t tam_informacion))
{
    unsigned i = 0;
    tPosteo publicacion;
    FILE* posteos = fopen(ARCHIVO_POSTEOS,"rb");
    if(!posteos)
    {
        printf("Hubo un error al abrir el archivo\n");
        return 0;
    }
    fseek(posteos,offset,SEEK_SET);
    while(i<cantPosteos && fread(&publicacion, sizeof(tPosteo), 1, posteos) == 1)
    {
        Func(listaDoble,&publicacion,sizeof(tPosteo));
        i++;
    }
    fclose(posteos);
    return i;
}

int cargarNPosteosFiltrado(tListaDoble* listaDoble, unsigned offset, unsigned cantPosteos, int Func(tListaDoble* p, void* informacion, size_t tam_informacion),
                           int cmp(void* a, void* b), void* parametro)
{
    unsigned i = 0;
    tPosteo publicacion;
    FILE* posteos = fopen(ARCHIVO_POSTEOS,"rb");
    if(!posteos)
    {
        printf("Hubo un error al abrir el archivo\n");
        return 0;
    }
    fseek(posteos,offset,SEEK_SET);
    while(i<cantPosteos && fread(&publicacion, sizeof(tPosteo), 1, posteos) == 1)
    {
        if(cmp(&publicacion, parametro) == 0)
        {
            Func(listaDoble,&publicacion,sizeof(tPosteo));
            i++;
        }
    }

    fclose(posteos);
    return i;
}

int cargarNPosteosAtras(tListaDoble* p, unsigned inicio, int n)
{
    FILE* publicacion = fopen(ARCHIVO_POSTEOS, "rb");
    tPosteo posteo;
    long pos = inicio;
    int cargados = 0, ok = 1;

    if(!publicacion)
        return 0;

    while(ok && cargados < n && pos >= (long)sizeof(tPosteo))
    {
        if(fseek(publicacion, pos - (long)sizeof(tPosteo), SEEK_SET) == 0 &&
           fread(&posteo, sizeof(tPosteo), 1, publicacion) == 1)
        {
            pos -= (long)sizeof(tPosteo);
            agregarAlComienzo(p, &posteo, sizeof(tPosteo));
            cargados++;
        }
        else
            ok = 0;
    }
    fclose(publicacion);
    return cargados;
}

int guardarPosteo(tUsuario usuario)
{
    tPosteo pub;
    FILE* posteo = fopen(ARCHIVO_POSTEOS_TEMP,"ab");
    if(!posteo)
        return -1;
    crearPosteo(&pub, usuario);
    fwrite(&pub,sizeof(tPosteo),1,posteo);
    fclose(posteo);
    return 0;
}

int combinarPosteos()
{
    int flag = 0;
    tLista p;
    lista_crear(&p);
    tPosteo publicacion;
    FILE* posteo_tmp = fopen(ARCHIVO_POSTEOS_TEMP,"rb");
    if(!posteo_tmp)
        return -1;
    FILE* posteo = fopen(ARCHIVO_POSTEOS,"rb");
    if(!posteo)
    {
        if(errno == ENOENT)
            flag = 1;
        else
        {
            fclose(posteo_tmp);
            return -1;
        }
    }
    FILE* posteo_nue = fopen(ARCHIVO_POSTEOS_NUE,"wb");
    if(!posteo_nue)
    {
        fclose(posteo_tmp);
        fclose(posteo);
        return -1;
    }
    while(fread(&publicacion, sizeof(tPosteo), 1, posteo_tmp) == 1)
    {
        lista_insertar_comienzo(&p,&publicacion,sizeof(tPosteo));
    }
    while(lista_vacia(&p) == LISTA_TODO_OK)
    {
        lista_sacar_primero(&p,&publicacion,sizeof(tPosteo));
        fwrite(&publicacion,sizeof(tPosteo),1,posteo_nue);
    }
    if(flag == 1)
    {
        while(fread(&publicacion, sizeof(tPosteo), 1, posteo) == 1)
        {
            fwrite(&publicacion,sizeof(tPosteo),1,posteo_nue);
        }
        fclose(posteo);
    }
    fclose(posteo_tmp);
    fclose(posteo_nue);

    remove(ARCHIVO_POSTEOS_TEMP);
    remove(ARCHIVO_POSTEOS);
    rename(ARCHIVO_POSTEOS_NUE, ARCHIVO_POSTEOS);
    return 0;
}
