#ifndef ALMACENAMIENTO_H_INCLUDED
#define ALMACENAMIENTO_H_INCLUDED

#include "global/lista.h"

#define ARCHIVO_USUARIOS "datos/usuarios.dat"

int almacenamiento_guardar_usuarios(const tLista *listaUsuarios);
int almacenamiento_cargar_usuarios(tLista *listaUsuarios, unsigned *ultimoId);
void accion_guardar_usuario(void *dato, void *extra);

#endif // ALMACENAMIENTO_H_INCLUDED
