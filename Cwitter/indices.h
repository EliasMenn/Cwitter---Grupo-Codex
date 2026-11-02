#ifndef INDICES_H_INCLUDED
#define INDICES_H_INCLUDED

#include "global/arbol.h"
#include "global/ListaDoble.h"

typedef struct{
    int verificado;     /* 1 si lo escribio un usuario verificado */
    int id_posteo;
    int id_padre;       /* id del posteo al que responde, 0 = no es una respuesta */
    long int offset;    /* posicion del registro en posteos.dat */
}IndPosteo;

typedef struct{
    int verificado;
    int id_posteo;
    int id_padre;
    long int offset;
}IndRespuesta;

int cmpPosteoIndice(const void* a, const void* b);
int cmpRespuestaIndice(const void* a, const void* b);

/* Indices en memoria (un solo juego para todo el programa). Se llenan con
   almacenamiento_reconstruir_indices(). */
tArbol* indices_posteos(void);
tArbol* indices_respuestas(void);

/* Busca un posteo por id (sea o no una respuesta). Devuelve 1 si lo encontro. */
int indices_buscar_posteo(int id, IndPosteo* destino);

/* Orden del feed: cada posteo seguido de sus respuestas (verificadas primero, despues las mas
   viejas), y cada respuesta seguida de las suyas, antes del siguiente posteo.
   x == NULL: el primero (siguiente) o el ultimo (anterior). Devuelven 1 si hay uno, 0 si no. */
int indices_siguiente_feed(const IndPosteo* x, IndPosteo* sig);
int indices_anterior_feed(const IndPosteo* x, IndPosteo* ant);
#endif // INDICES_H_INCLUDED
