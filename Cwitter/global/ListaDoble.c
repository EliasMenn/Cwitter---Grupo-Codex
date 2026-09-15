#include "ListaDoble.h"

void crearListaD(tListaDoble* p)
{
    *p = NULL;
}

int agregarAlFinal(tListaDoble* p, void* informacion, size_t tam_informacion)
{
    tNodo* act = *p;
    tNodo* nue;

    if(act != NULL)
    {
        while(act->siguiente != NULL)
            act = act->siguiente;
    }

    nue = malloc(sizeof(tNodo));
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

    return 0;
}

int quitarDelFinal(tListaDoble* p, void* informacion, size_t tam_informacion)
{
    tNodo* act = *p;
    tNodo* elim;

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
    tNodo* act = *p;
    tNodo* nue = malloc(sizeof(tNodo));
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
    tNodo* act = *p;
    tNodo* elim;

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
    tNodo* act = *p;
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
    tNodo* act = *p;
    tNodo* elim;

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

