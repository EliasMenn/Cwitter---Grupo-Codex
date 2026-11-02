#include <limits.h>
#include "indices.h"

int cmpPosteoIndice(const void* a, const void* b)
{
    IndPosteo* n1 = (IndPosteo*) a;
    IndPosteo* n2 = (IndPosteo*) b;

    if(n1->verificado > n2->verificado)
        return -1;
    else if(n1->verificado < n2->verificado)
        return 1;
    else
    {
        /* id mayor primero: los posteos mas nuevos van antes (igual que el feed actual) */
        if(n1->id_posteo > n2->id_posteo)
            return -1;
        if(n1->id_posteo < n2->id_posteo)
            return 1;
        return 0;
    }
}

/* Respuestas de un mismo posteo juntas y en el orden que se muestran:
   verificadas primero, y dentro de cada grupo las mas viejas primero. */
int cmpRespuestaIndice(const void* a, const void* b)
{
    IndRespuesta* n1 = (IndRespuesta*) a;
    IndRespuesta* n2 = (IndRespuesta*) b;

    if(n1->id_padre > n2->id_padre)
        return 1;
    if(n1->id_padre < n2->id_padre)
        return -1;
    if(n1->verificado > n2->verificado)
        return -1;
    if(n1->verificado < n2->verificado)
        return 1;
    if(n1->id_posteo > n2->id_posteo)
        return 1;
    if(n1->id_posteo < n2->id_posteo)
        return -1;
    return 0;
}

static tArbol idxPosteos = NULL;
static tArbol idxRespuestas = NULL;

tArbol* indices_posteos(void)
{
    return &idxPosteos;
}

tArbol* indices_respuestas(void)
{
    return &idxRespuestas;
}

/* ---------- recorrido del feed (posteos + respuestas) ---------- */

static void respuestaAPosteo(const IndRespuesta* r, IndPosteo* p)
{
    p->verificado = r->verificado;
    p->id_posteo = r->id_posteo;
    p->id_padre = r->id_padre;
    p->offset = r->offset;
}

int indices_buscar_posteo(int id, IndPosteo* destino)
{
    IndPosteo clave;

    clave.id_posteo = id;
    clave.id_padre = 0;
    clave.offset = 0;
    for(clave.verificado = 1; clave.verificado >= 0; clave.verificado--)   /* no sabemos si es verificado */
    {
        if(arbol_buscar(indices_posteos(), &clave, sizeof(IndPosteo), cmpPosteoIndice) == ENCONTRADO)
        {
            *destino = clave;
            return 1;
        }
    }
    return 0;
}

/* Siguiente posteo que NO es una respuesta (ref NULL = el primero). */
static int posteoSiguienteTop(const IndPosteo* ref, IndPosteo* out)
{
    while(arbol_siguiente(indices_posteos(), ref, out, sizeof(IndPosteo), cmpPosteoIndice) == ENCONTRADO)
    {
        if(out->id_padre == 0)
            return 1;
        ref = out;
    }
    return 0;
}

/* Anterior posteo que NO es una respuesta (ref NULL = el ultimo). */
static int posteoAnteriorTop(const IndPosteo* ref, IndPosteo* out)
{
    while(arbol_anterior(indices_posteos(), ref, out, sizeof(IndPosteo), cmpPosteoIndice) == ENCONTRADO)
    {
        if(out->id_padre == 0)
            return 1;
        ref = out;
    }
    return 0;
}

static void claveRespuesta(IndRespuesta* k, int padre, int verificado, int id)
{
    k->id_padre = padre;
    k->verificado = verificado;
    k->id_posteo = id;
    k->offset = 0;
}

/* Primera respuesta de "padre": la verificada mas vieja, o si no hay, la mas vieja. */
static int primeraRespuesta(int padre, IndPosteo* out)
{
    IndRespuesta clave, r;

    claveRespuesta(&clave, padre, INT_MAX, 0);      /* va antes que cualquier respuesta de padre */
    if(arbol_siguiente(indices_respuestas(), &clave, &r, sizeof(IndRespuesta), cmpRespuestaIndice) == ENCONTRADO
       && r.id_padre == padre)
    {
        respuestaAPosteo(&r, out);
        return 1;
    }
    return 0;
}

/* Ultima respuesta de "padre" (la que se muestra al final de sus respuestas). */
static int ultimaRespuesta(int padre, IndPosteo* out)
{
    IndRespuesta clave, r;

    claveRespuesta(&clave, padre + 1, INT_MAX, 0);  /* va justo despues de todas las de padre */
    if(arbol_anterior(indices_respuestas(), &clave, &r, sizeof(IndRespuesta), cmpRespuestaIndice) == ENCONTRADO
       && r.id_padre == padre)
    {
        respuestaAPosteo(&r, out);
        return 1;
    }
    return 0;
}

/* Hermano siguiente / anterior de una respuesta (mismo padre). */
static int respuestaSiguiente(const IndPosteo* x, IndPosteo* out)
{
    IndRespuesta clave, r;

    claveRespuesta(&clave, x->id_padre, x->verificado, x->id_posteo);
    if(arbol_siguiente(indices_respuestas(), &clave, &r, sizeof(IndRespuesta), cmpRespuestaIndice) == ENCONTRADO
       && r.id_padre == x->id_padre)
    {
        respuestaAPosteo(&r, out);
        return 1;
    }
    return 0;
}

static int respuestaAnterior(const IndPosteo* x, IndPosteo* out)
{
    IndRespuesta clave, r;

    claveRespuesta(&clave, x->id_padre, x->verificado, x->id_posteo);
    if(arbol_anterior(indices_respuestas(), &clave, &r, sizeof(IndRespuesta), cmpRespuestaIndice) == ENCONTRADO
       && r.id_padre == x->id_padre)
    {
        respuestaAPosteo(&r, out);
        return 1;
    }
    return 0;
}

int indices_siguiente_feed(const IndPosteo* x, IndPosteo* sig)
{
    IndPosteo cur;

    if(x == NULL)
        return posteoSiguienteTop(NULL, sig);

    if(primeraRespuesta(x->id_posteo, sig))         /* tiene respuestas: sigue la primera */
        return 1;

    cur = *x;
    for(;;)
    {
        if(cur.id_padre == 0)                       /* posteo comun: el siguiente posteo */
            return posteoSiguienteTop(&cur, sig);
        if(respuestaSiguiente(&cur, sig))           /* respuesta: su hermana siguiente */
            return 1;
        if(!indices_buscar_posteo(cur.id_padre, &cur))
            return posteoSiguienteTop(&cur, sig);   /* padre borrado: seguir con los posteos */
        /* subimos al padre y probamos con su hermano siguiente (cur.id_padre < id: termina) */
    }
}

int indices_anterior_feed(const IndPosteo* x, IndPosteo* ant)
{
    IndPosteo s, t;
    int hay;

    if(x == NULL || x->id_padre == 0)
        hay = posteoAnteriorTop(x, &s);
    else
        hay = respuestaAnterior(x, &s);

    if(hay)
    {
        while(ultimaRespuesta(s.id_posteo, &t))     /* lo ultimo que se mostro antes de x */
            s = t;                                  /* es el ultimo descendiente del hermano */
        *ant = s;
        return 1;
    }

    if(x != NULL && x->id_padre != 0)               /* sin hermano anterior: lo anterior es el padre */
        return indices_buscar_posteo(x->id_padre, ant);
    return 0;
}
