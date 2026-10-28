#ifndef ARBOL_H_INCLUDED
#define ARBOL_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARBOL_OK 1
#define ERROR_MEM -1
#define DUPLICADO 10

typedef struct sNodoA{
    void* dato;
    int altura;
    struct sNodoA* der;
    struct sNodoA* izq;
}tNodoA;
typedef tNodoA* tArbol;

typedef void (*tAccion)(void *elem, void *extra);
typedef int (*tCmp)(const void *a, const void *b);

void crear_arbol(tArbol* arbol);
void arbol_vaciar(tArbol* arbol);
int arbol_insertar(tArbol* arbol, const void* dato, size_t tamDato, tCmp cmp);
int arbol_quitar(tArbol* arbol, void* dato, size_t tamDato, tCmp cmp);
int arbol_buscar(const tArbol* arbol, void* dato, size_t tamDato, tCmp cmp);
void arbol_recorrer_inorden(const tArbol* arbol, tAccion accion, void* ctx);
void arbol_recorrer_inorden_inverso(const tArbol* arbol, tAccion accion, void* ctx);
void arbol_recorrer_rango(const tArbol* arbol, const void* desde, const void* hasta,
                          tCmp cmp, tAccion accion, void* ctx);
int altura(const tNodoA* nodo);
void    actualizar_altura(tNodoA* nodo);
int     factor_equilibrio(const tNodoA* nodo);
void    rotar_der(tArbol* raiz);
void    rotar_izq(tArbol* raiz);
void    equilibrar(tArbol* raiz);
tNodoA* desprender_menor(tArbol* raiz);
#endif // ARBOL_H_INCLUDED

