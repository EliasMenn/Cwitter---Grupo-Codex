#ifndef PUBLICACIONES_H_INCLUDED
#define PUBLICACIONES_H_INCLUDED

#define MAX_PUBLICACIONES 5
#define MAX_CHARS 80
#define CAMBIO_PUBLICACIONES 1
#define LARGO_MAX 141
#include "global/ListaDoble.h"
#include "usuarios.h"
#include "almacenamiento.h"
typedef struct{
   tListaDoble p;
   int posteo_actual;
   long offset;
   long inicio;
}tFeed;

typedef struct{
    unsigned id;
    char nombreUsuario[MAX_USUARIO];
    char publicacion[LARGO_MAX];
}tPosteo;

void siguientePosteo(tFeed* feed);
void mostrarPosteo(tPosteo pub);
void crearPosteo (tPosteo * pub, tUsuario user);
void iniciarFeed(tFeed* feed);
void posteoAnterior(tFeed* feed);
void cargarPostsFiltrados(tFeed* feed, int cmp(void*a, void*b), void* parametroFiltro);
void siguientePosteoFiltrados(tFeed* feed);
void posteoAnteriorFiltrado(tFeed* feed);
int cmpNombreUsuario(void* a, void* b);
int cmpTexto(void* a, void* b);

#endif // PUBLICACIONES_H_INCLUDED
