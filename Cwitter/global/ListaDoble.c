#include "ListaDoble.h"

void crearListaD(tListaDoble* p)
{
    p = NULL;
}

int agregarAlFinal(tListaDoble* p, void* informacion, size_t tam_informacion)
{
    tNodoD* act = *p;
    tNodoD* nue;

    if(act != NULL)
    {
        while(act->siguiente != NULL)
            act = act->siguiente;
    }

    nue = malloc(sizeof(tNodoD));
    if(!nue)
        return -1;

    nue->informacion = malloc(tam_informacion);
    if(!nue->informacion)
    {
        free(nue);
        return -1;
    }

    memcpy(nue->informacion,informacion,tam_informacion);
    nue->tam_informacion = tam_informacion;
    nue->siguiente = NULL;
    nue->anterior = act;

    if(act != NULL)
        act->siguiente = nue;
    else
        *p = nue;
    return 0;
}

int quitarDelFinal(tListaDoble* p, void* informacion, size_t tam_informacion)
{
    tNodoD* act = *p;
    tNodoD* elim;

    if (act == NULL)
        return -1; // lista vacía, nada que quitar

    if (act->siguiente == NULL)
    {
        // Caso especial: un solo nodo en la lista
        elim = act;
        memcpy(informacion, elim->informacion, MIN(tam_informacion, elim->tam_informacion));
        free(elim->informacion);
        free(elim);
        *p = NULL; // la lista queda vacía
        return 0;
    }

    // Caso general: avanzar hasta el anteúltimo nodo
    while (act->siguiente->siguiente != NULL)
        act = act->siguiente;

    elim = act->siguiente;

    memcpy(informacion, elim->informacion, MIN(tam_informacion, elim->tam_informacion));
    free(elim->informacion);
    free(elim);

    act->siguiente = NULL;
    return 0;
}

int agregarAlComienzo(tListaDoble* p, void* informacion, size_t tam_informacion)
{
    tNodoD* act = *p;
    tNodoD* nue = malloc(sizeof(tNodoD));
    if(!nue)
        return -1;

    nue->informacion = malloc(tam_informacion);
    if(!nue->informacion)
    {
        free(nue);
        return -1;
    }

    memcpy(nue->informacion,informacion,tam_informacion);
    nue->tam_informacion = tam_informacion;
    nue->siguiente = act;
    nue->anterior = NULL;

    if(act)
        act->anterior = nue;
    *p = nue;
    return 0;
}

int quitarDelComienzo(tListaDoble* p, void* informacion, size_t tam_informacion)
{
    tNodoD* act = *p;
    tNodoD* elim;

    if(!act)
        return -1;

    elim = act;

    if(act->siguiente != NULL)
    {
        *p = elim->siguiente;
        (*p)->anterior = NULL;
    }
    else
    {
        *p = NULL;
    }

    memcpy(informacion, elim->informacion, MIN(tam_informacion, elim->tam_informacion));
    free(elim->informacion);
    free(elim);
    return 0;
}

int obtenerPosicionN(tListaDoble* p, void* informacion, size_t tam_informacion, int N)
{
    int i = 0;
    tNodoD* act = *p;
    if(!act)
        return -1;

    while(i<N)
    {
        act = act->siguiente;
        if(!act)
            return -1;
        i++;
    }

    memcpy(informacion,act->informacion,MIN(tam_informacion,act->tam_informacion));
    return 0;
}

void vaciarLista(tListaDoble *p)
{
    tNodoD* act = *p;
    tNodoD* elim;

    if(!act)
        return;
    while(*p != NULL)
    {
        elim = act;

        if(act->siguiente != NULL)
        {
            *p = elim->siguiente;
            (*p)->anterior = NULL;
        }
        else
        {
            *p = NULL;
        }

        free(elim->informacion);
        free(elim);
    }
}

int eliminarDeListaDoble(tListaDoble *p, const void *clave, void *dest, size_t tamDest, tCmpD cmp)
{
    tNodoD *act = *p;

    while(act != NULL && cmp(clave, act->informacion) != 0)
    {
        act = act->siguiente;
    }

    if(act == NULL)
        return 0;

    if(dest != NULL)
    {
        memcpy(dest, act->informacion, MIN(tamDest, act->tam_informacion));
    }


    if(act->anterior != NULL)
    {
        act->anterior->siguiente = act->siguiente;
    }
    else
    {
        *p = act->siguiente;
    }

    if(act->siguiente != NULL)
    {
        act->siguiente->anterior = act->anterior;
    }

    free(act->informacion);
    free(act);

    return 1;
}

void recorrerListaDoble(const tListaDoble *p, tAccionD accion, void *extra)
{
    tNodoD *act = *p;
    while(act != NULL)
    {
        accion(act->informacion, extra);
        act = act->siguiente;
    }
}

