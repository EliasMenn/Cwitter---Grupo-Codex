#ifndef TESTS_H_INCLUDED
#define TESTS_H_INCLUDED

#include "global/lista.h"
#include "global/pila.h"
#include "usuarios.h"
#include "almacenamiento.h"

void tests_ejecutar_todos(tLista *indicesUsuarios, tPila *pilaLibres, unsigned *ultimoId);

#endif // TESTS_H_INCLUDED
