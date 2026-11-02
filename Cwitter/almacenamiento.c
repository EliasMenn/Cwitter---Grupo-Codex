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

/* Lee del archivo el posteo al que apunta una entrada del indice. */
static int leerPosteoPorIndice(FILE* archivo, const IndPosteo* ind, tPosteo* destino)
{
    return fseek(archivo, ind->offset, SEEK_SET) == 0 &&
           fread(destino, sizeof(tPosteo), 1, archivo) == 1;
}

/* Vuelve a armar los dos arboles leyendo el archivo de posteos de principio a fin.
   Llamar al iniciar y despues de cualquier cosa que reescriba el archivo (los offsets cambian). */
int almacenamiento_reconstruir_indices(void)
{
    FILE* archivo;
    tPosteo publicacion;
    IndPosteo ip;
    IndRespuesta ir;
    long pos;
    int ok = 1;

    arbol_vaciar(indices_posteos());
    arbol_vaciar(indices_respuestas());

    archivo = fopen(ARCHIVO_POSTEOS, "rb");
    if(!archivo)
        return 1;                       /* sin archivo todavia: indices vacios */

    pos = ftell(archivo);
    while(fread(&publicacion, sizeof(tPosteo), 1, archivo) == 1)
    {
        ip.verificado = (publicacion.verificado == 'S');
        ip.id_posteo = (int)publicacion.id;
        /* una respuesta siempre es mas nueva que su padre; si no, el dato esta mal y se trata como posteo comun */
        ip.id_padre = (publicacion.id_padre != 0 && publicacion.id_padre < publicacion.id) ? (int)publicacion.id_padre : 0;
        ip.offset = pos;
        if(arbol_insertar(indices_posteos(), &ip, sizeof(IndPosteo), cmpPosteoIndice) == ERROR_MEM)
            ok = 0;

        if(ip.id_padre != 0)            /* 0 = no es una respuesta */
        {
            ir.verificado = ip.verificado;
            ir.id_posteo = ip.id_posteo;
            ir.id_padre = ip.id_padre;
            ir.offset = pos;
            if(arbol_insertar(indices_respuestas(), &ir, sizeof(IndRespuesta), cmpRespuestaIndice) == ERROR_MEM)
                ok = 0;
        }
        pos = ftell(archivo);
    }
    fclose(archivo);
    return ok;
}

/* Carga hasta cantPosteos posteos que siguen a "desde" en el orden del feed: cada posteo con sus
   respuestas debajo (desde NULL = empezar por el primero). Devuelve cuantos cargo. */
int cargarNPosteos(tListaDoble* listaDoble, const IndPosteo* desde, unsigned cantPosteos, int Func(tListaDoble* p, void* informacion, size_t tam_informacion))
{
    unsigned i = 0;
    tPosteo publicacion;
    IndPosteo cursor;
    IndPosteo sig;
    const IndPosteo* ref = desde;
    FILE* posteos = fopen(ARCHIVO_POSTEOS,"rb");
    if(!posteos)
    {
        printf("Hubo un error al abrir el archivo\n");
        return 0;
    }
    while(i<cantPosteos &&
          indices_siguiente_feed(ref, &sig) &&
          leerPosteoPorIndice(posteos, &sig, &publicacion))
    {
        Func(listaDoble,&publicacion,sizeof(tPosteo));
        cursor = sig;
        ref = &cursor;
        i++;
    }
    fclose(posteos);
    return i;
}

/* Igual que cargarNPosteos pero solo se queda con los posteos para los que cmp(posteo, parametro) == 0. */
int cargarNPosteosFiltrado(tListaDoble* listaDoble, const IndPosteo* desde, unsigned cantPosteos, int Func(tListaDoble* p, void* informacion, size_t tam_informacion),
                           int cmp(void* a, void* b), void* parametro)
{
    unsigned i = 0;
    tPosteo publicacion;
    IndPosteo actual;
    const IndPosteo* ref = desde;
    FILE* posteos = fopen(ARCHIVO_POSTEOS,"rb");
    if(!posteos)
    {
        printf("Hubo un error al abrir el archivo\n");
        return 0;
    }
    while(i<cantPosteos &&
          arbol_siguiente(indices_posteos(), ref, &actual, sizeof(IndPosteo), cmpPosteoIndice) == ENCONTRADO &&
          leerPosteoPorIndice(posteos, &actual, &publicacion))
    {
        ref = &actual;
        if(cmp(&publicacion, parametro) == 0)
        {
            Func(listaDoble,&publicacion,sizeof(tPosteo));
            i++;
        }
    }

    fclose(posteos);
    return i;
}

/* Carga hasta n posteos que van ANTES de "hasta" en el orden del feed, agregandolos al comienzo
   de la lista de a uno (el mas cercano a "hasta" primero), asi la lista queda en orden. */
int cargarNPosteosAtras(tListaDoble* p, const IndPosteo* hasta, int n)
{
    FILE* publicacion = fopen(ARCHIVO_POSTEOS, "rb");
    tPosteo posteo;
    IndPosteo cursor;
    IndPosteo ant;
    const IndPosteo* ref = hasta;
    int cargados = 0;

    if(!publicacion)
        return 0;

    while(cargados < n &&
          indices_anterior_feed(ref, &ant) &&
          leerPosteoPorIndice(publicacion, &ant, &posteo))
    {
        agregarAlComienzo(p, &posteo, sizeof(tPosteo));
        cursor = ant;
        ref = &cursor;
        cargados++;
    }
    fclose(publicacion);
    return cargados;
}

unsigned almacenamiento_obtener_proximo_id_posteo(void)
{
    unsigned maxId = 0;
    tPosteo p;
    FILE *arch = fopen(ARCHIVO_POSTEOS, "rb");
    if(arch)
    {
        while(fread(&p, sizeof(tPosteo), 1, arch) == 1)
        {
            if(p.id > maxId)
                maxId = p.id;
        }
        fclose(arch);
    }

    FILE *archTemp = fopen(ARCHIVO_POSTEOS_TEMP, "rb");
    if(archTemp)
    {
        while(fread(&p, sizeof(tPosteo), 1, archTemp) == 1)
        {
            if(p.id > maxId)
                maxId = p.id;
        }
        fclose(archTemp);
    }

    return maxId + 1;
}

static int guardarPosteoPadre(tUsuario usuario, unsigned idPadre)
{
    tPosteo pub;
    pub.id = almacenamiento_obtener_proximo_id_posteo();
    pub.id_padre = idPadre;     /* 0 = posteo comun, otro valor = respuesta a ese posteo */
    FILE* posteo = fopen(ARCHIVO_POSTEOS_TEMP,"ab");
    if(!posteo)
        return -1;
    crearPosteo(&pub, usuario);
    fwrite(&pub,sizeof(tPosteo),1,posteo);
    fclose(posteo);

    if(usuario.verificado == 'S')
    {
        famecheck_reportar_tweet(usuario.usuario, pub.publicacion);
    }

    return 0;
}

int guardarPosteo(tUsuario usuario)
{
    return guardarPosteoPadre(usuario, 0);
}

/* Guarda una respuesta al posteo idPadre. Devuelve -2 si ese posteo no existe (o todavia no se
   combino, ver combinarPosteos), si no lo mismo que guardarPosteo. */
int guardarRespuesta(tUsuario usuario, unsigned idPadre)
{
    IndPosteo padre;

    if(idPadre == 0 || !indices_buscar_posteo((int)idPadre, &padre))
        return -2;
    return guardarPosteoPadre(usuario, idPadre);
}

int combinarPosteos()
{
    int primero = 0, tmpVerificado = 0, volcado = 0;
    tLista p;
    lista_crear(&p);
    tPosteo publicacion;
    FILE* posteo_tmp = fopen(ARCHIVO_POSTEOS_TEMP,"rb");
    if(!posteo_tmp)
        return -1;
    FILE* posteo = fopen(ARCHIVO_POSTEOS,"rb");
    if(!posteo && errno != ENOENT)
    {
        fclose(posteo_tmp);
        return -1;
    }
    FILE* posteo_nue = fopen(ARCHIVO_POSTEOS_NUE,"wb");
    if(!posteo_nue)
    {
        fclose(posteo_tmp);
        if(posteo)
            fclose(posteo);
        return -1;
    }
    while(fread(&publicacion, sizeof(tPosteo), 1, posteo_tmp) == 1)
    {
        if(primero == 0)
        {
            if(publicacion.verificado == 'S')
            {
                tmpVerificado = 1;
            }
            primero = 1;
        }
        lista_insertar_comienzo(&p,&publicacion,sizeof(tPosteo));
    }
    if(tmpVerificado == 1)
    {
        volcarPosteo(&p,posteo_nue);
        volcado = 1;
    }
    if(posteo != NULL)
    {
        while(fread(&publicacion, sizeof(tPosteo), 1, posteo) == 1)
        {
            if(volcado == 0 && publicacion.verificado != 'S')
            {
                volcarPosteo(&p, posteo_nue);
                volcado = 1;
            }
            fwrite(&publicacion,sizeof(tPosteo),1,posteo_nue);
        }
        fclose(posteo);
    }

    if(!volcado)
        volcarPosteo(&p, posteo_nue);

    fclose(posteo_tmp);
    fclose(posteo_nue);

    remove(ARCHIVO_POSTEOS_TEMP);
    remove(ARCHIVO_POSTEOS);
    rename(ARCHIVO_POSTEOS_NUE, ARCHIVO_POSTEOS);
    almacenamiento_reconstruir_indices();   /* el archivo cambio: los offsets tambien */
    return 0;
}

void volcarPosteo(tLista* p, FILE* destino)
{
    tPosteo publicacion;
    while(lista_vacia(p) == LISTA_TODO_OK)
    {
        lista_sacar_primero(p, &publicacion, sizeof(tPosteo));
        fwrite(&publicacion, sizeof(tPosteo), 1, destino);
    }
}
int almacenamiento_eliminar_posteo(unsigned idPosteo, const char *nombreUsuario)
{
    combinarPosteos();

    FILE *archOrig = fopen(ARCHIVO_POSTEOS, "rb");
    if(!archOrig)
    {
        return 0;
    }

    FILE *archNue = fopen(ARCHIVO_POSTEOS_NUE, "wb");
    if(!archNue)
    {
        fclose(archOrig);
        return -2;
    }

    tPosteo pub;
    int encontrado = 0;
    int noAutorizado = 0;

    while(fread(&pub, sizeof(tPosteo), 1, archOrig) == 1)
    {
        if(pub.id == idPosteo)
        {
            encontrado = 1;
            if(strcmp(pub.nombreUsuario, nombreUsuario) != 0)
            {
                noAutorizado = 1;
                fwrite(&pub, sizeof(tPosteo), 1, archNue);
            }
        }
        else
        {
            fwrite(&pub, sizeof(tPosteo), 1, archNue); // Se conserva el tweet en el archivo nuevo
        }
    }

    fclose(archOrig);
    fclose(archNue);

    if(!encontrado)
    {
        remove(ARCHIVO_POSTEOS_NUE);
        return 0; // Tweet no encontrado
    }

    if(noAutorizado)
    {
        remove(ARCHIVO_POSTEOS_NUE);
        return -1; // No pertenece al usuario
    }

    remove(ARCHIVO_POSTEOS);
    rename(ARCHIVO_POSTEOS_NUE, ARCHIVO_POSTEOS);
    almacenamiento_reconstruir_indices();   /* al borrar, los offsets de los posteos siguientes cambian */
    return 1;
}

int almacenamiento_editar_posteo(unsigned idPosteo, const char *nombreUsuario, const char *nuevoTexto)
{
    FILE *archOrig;
    FILE *archNue;
    tPosteo pub;
    int encontrado;
    int noAutorizado;

    combinarPosteos();

    archOrig = fopen(ARCHIVO_POSTEOS, "rb");
    if(!archOrig)
    {
        return 0;
    }

    archNue = fopen(ARCHIVO_POSTEOS_NUE, "wb");
    if(!archNue)
    {
        fclose(archOrig);
        return -2;
    }

    encontrado = 0;
    noAutorizado = 0;

    while(fread(&pub, sizeof(tPosteo), 1, archOrig) == 1)
    {
        if(pub.id == idPosteo)
        {
            encontrado = 1;
            if(strcmp(pub.nombreUsuario, nombreUsuario) != 0)
            {
                noAutorizado = 1;
                fwrite(&pub, sizeof(tPosteo), 1, archNue);
            }
            else
            {
                strcpy(pub.publicacion, nuevoTexto);
                fwrite(&pub, sizeof(tPosteo), 1, archNue);
            }
        }
        else
        {
            fwrite(&pub, sizeof(tPosteo), 1, archNue);
        }
    }

    fclose(archOrig);
    fclose(archNue);

    if(!encontrado)
    {
        remove(ARCHIVO_POSTEOS_NUE);
        return 0;
    }

    if(noAutorizado)
    {
        remove(ARCHIVO_POSTEOS_NUE);
        return -1;
    }

    remove(ARCHIVO_POSTEOS);
    rename(ARCHIVO_POSTEOS_NUE, ARCHIVO_POSTEOS);
    return 1;
}
