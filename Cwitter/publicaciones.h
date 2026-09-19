#ifndef PUBLICACIONES_H_INCLUDED
#define PUBLICACIONES_H_INCLUDED

#define MAX_PUBLICACIONES 100
#define MAX_CHARS 80
#define CAMBIO_PUBLICACIONES 10
#define LARGO_MAX 141
#include "global/ListaDoble.h"
#include "usuarios.h"
#include "almacenamiento.h"
typedef struct{
   tListaDoble *p;
   unsigned posteo_actual;
   unsigned offset;
   unsigned inicio;
}tFeed;

typedef struct{
    unsigned id;
    char nombreUsuario[MAX_USUARIO];
    char publicacion[LARGO_MAX];
}tPosteo;

void siguientePosteo(tFeed* feed);
void mostrarPosteo(tPosteo pub);
void crearPosteo (tPosteo * pub, tUsuario user);

#endif // PUBLICACIONES_H_INCLUDED
