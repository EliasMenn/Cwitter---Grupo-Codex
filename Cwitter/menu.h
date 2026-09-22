#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

#include "global/lista.h"
#include "global/pila.h"
#include "usuarios.h"

void menu_iniciar(tLista *indicesUsuarios, tPila *pilaLibres, unsigned *ultimoId);

#endif // MENU_H_INCLUDED
