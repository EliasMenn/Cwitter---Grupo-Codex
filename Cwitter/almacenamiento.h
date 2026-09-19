#ifndef ALMACENAMIENTO_H_INCLUDED
#define ALMACENAMIENTO_H_INCLUDED

#include "global/lista.h"
#include "global/pila.h"
#include "usuarios.h"

#define ARCHIVO_USUARIOS "datos/usuarios.dat"
#define ARCHIVO_INDICES_USUARIOS "datos/indice_usuarios.idx"

int almacenamiento_cargar_indices(tLista *listaIndices, tPila *pilaLibres, unsigned *ultimoId);
int almacenamiento_guardar_nuevo_usuario(tUsuario *usuario, tIndiceUsuario *indiceACompletar, tPila *pilaLibres);
int almacenamiento_leer_usuario_offset(long offset, tUsuario *usuarioDestino);
int almacenamiento_baja_usuario(long offsetDat, long offsetIdx, tPila *pilaLibres);
int almacenamiento_actualizar_usuario(long offsetDat, tUsuario *usuarioActualizado);

#endif // ALMACENAMIENTO_H_INCLUDED
