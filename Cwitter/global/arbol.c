#include "arbol.h"

void crear_arbol(tArbol* arbol)
{
    *arbol = NULL;
}
void arbol_vaciar(tArbol* arbol)
{
    if(*arbol == NULL)
        return;
    arbol_vaciar(&(*arbol)->izq);
    arbol_vaciar(&(*arbol)->der);
    free((*arbol)->dato);
    free(*arbol);
    *arbol = NULL;
}
int arbol_insertar(tArbol* arbol, const void* dato, size_t tamDato, tCmp cmp)
{
    int c;
    int rc;
    if(*arbol == NULL)
    {
        tNodoA* nue = malloc(sizeof(tNodoA));
        if(!nue)
            return ERROR_MEM;
        nue->dato = malloc(tamDato);
        if(!(nue->dato))
        {
            free(nue);
            return ERROR_MEM;
        }
        memcpy(nue->dato,dato,tamDato);
        nue->altura = 1;
        nue->izq = NULL;
        nue->der = NULL;

        *arbol = nue;
        return ARBOL_OK;
    }

    c = cmp(dato,(*arbol)->dato);
    if(c == 0)
        return DUPLICADO;
    if(c < 0)
        rc = arbol_insertar(&(*arbol)->izq,dato,tamDato,cmp);
    else
        rc = arbol_insertar(&(*arbol)->der,dato,tamDato,cmp);
    if(rc == ARBOL_OK)
        equilibrar(arbol);
    return rc;
}
int arbol_quitar(tArbol* arbol, void* dato, size_t tamDato, tCmp cmp);
int arbol_buscar(const tArbol* arbol, void* dato, size_t tamDato, tCmp cmp);
void arbol_recorrer_inorden(const tArbol* arbol, tAccion accion, void* ctx);
void arbol_recorrer_inorden_inverso(const tArbol* arbol, tAccion accion, void* ctx);
void arbol_recorrer_rango(const tArbol* arbol, const void* desde, const void* hasta,
                          tCmp cmp, tAccion accion, void* ctx);
int altura(const tNodoA* n)
{
    return n ? n->altura : 0;
}

void actualizar_altura(tNodoA* n)
{
    int hi = altura(n->izq);
    int hd = altura(n->der);

    n->altura = 1 + (hi > hd ? hi : hd);
}
int factor_equilibrio(const tNodoA* n)
{
    return n ? altura(n->izq) - altura(n->der) : 0;
}
void rotar_der(tArbol* raiz)
{
    tNodoA* y = *raiz;
    tNodoA* x = y->izq;

    y->izq = x->der;
    x->der = y;
    actualizar_altura(y);       /* y ended up lower, so update it first */
    actualizar_altura(x);
    *raiz = x;
}
void rotar_izq(tArbol* raiz)
{
    tNodoA* x = *raiz;
    tNodoA* y = x->der;

    x->der = y->izq;
    y->izq = x;
    actualizar_altura(x);
    actualizar_altura(y);
    *raiz = y;
}
void equilibrar(tArbol* raiz)
{
    int fe;

    actualizar_altura(*raiz);
    fe = factor_equilibrio(*raiz);

    if (fe > 1)                                     /* left-heavy */
    {
        if (factor_equilibrio((*raiz)->izq) < 0)    /* left-right case */
            rotar_izq(&(*raiz)->izq);
        rotar_der(raiz);                            /* left-left case */
    }
    else if (fe < -1)                               /* right-heavy */
    {
        if (factor_equilibrio((*raiz)->der) > 0)    /* right-left case */
            rotar_der(&(*raiz)->der);
        rotar_izq(raiz);                            /* right-right case */
    }
}
