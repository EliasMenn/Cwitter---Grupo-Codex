#include <stdio.h>
#include <stdlib.h>
#include "usuarios.h"
#include "almacenamiento.h"
#include "menu.h"
#include "tests.h"
#include "famecheck.h"
#include <windows.h>

// Poner en 1 para ejecutar tests definidos
// Poner en 0 para ejecutar normalmente
#define MODO_TEST 0

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    famecheck_inicializar();

    tLista indicesUsuarios;
    tPila pilaLibres;
    unsigned ultimoId = 0;

    tFeed feed;
    almacenamiento_reconstruir_indices();
    iniciarFeed(&feed);

    lista_crear(&indicesUsuarios);
    crearPila(&pilaLibres);

    // Cargar indices desde disco (si existen) y armar pila de libres
    almacenamiento_cargar_indices(&indicesUsuarios, &pilaLibres, &ultimoId);

#if MODO_TEST
    tests_ejecutar_todos(&indicesUsuarios, &pilaLibres, &ultimoId);
#else
    menu_iniciar(&indicesUsuarios, &pilaLibres, &ultimoId);
#endif

    vaciarLista(&feed.p);
    lista_vaciar(&indicesUsuarios);
    vaciarPila(&pilaLibres);

    arbol_vaciar(indices_posteos());
    arbol_vaciar(indices_respuestas());
    famecheck_limpiar();
    return 0;
}
