#ifndef LISTADOBLE_H_INCLUDED
#define LISTADOBLE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct sNodo{
    void* informacion;
    size_t tam_informacion;
    struct sNodo* siguiente;
    struct sNodo* anterior;
}tNodo;

typedef tNodo tListaDoble*;

void crearListaD(tListaDoble* p);
int agregarAlFinal(tListaDoble* p, void* informacion, size_t tam_informacion);
int quitarDelFinal(tListaDoble* p, void* informacion, size_t tam_informacion);
int agregarAlComienzo(tListaDoble* p, void* informacion, size_t tam_informacion);
int quitarDelComienzo(tListaDoble* p, void* informacion, size_t tam_informacion);
int obtenerPosicionN(tListaDoble* p, void* informacion, size_t tam_informacion, int N);
#endif // LISTADOBLE_H_INCLUDED
