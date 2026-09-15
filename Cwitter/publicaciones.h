#ifndef PUBLICACIONES_H_INCLUDED
#define PUBLICACIONES_H_INCLUDED

#define MAX_PUBLICACIONES 100
#define CAMBIO_PUBLICACIONES 10
#define LARGO_MAX 141
#include "global/ListaDoble.h"
#include "usuarios.h"

typedef struct{
    char nombreUsuario[MAX_USUARIO];
    char publicacion[LARGO_MAX];
}tPosteo;

void mostrarPosteo(tPosteo pub);
void crearPosteo (tPosteo pub, tUsuario user);

#endif // PUBLICACIONES_H_INCLUDED
