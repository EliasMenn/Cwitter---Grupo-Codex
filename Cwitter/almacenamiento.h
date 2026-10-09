#ifndef ALMACENAMIENTO_H_INCLUDED
#define ALMACENAMIENTO_H_INCLUDED

#include "global/lista.h"
#include "global/ListaDoble.h"
#include "global/pila.h"
#include "usuarios.h"
#include "publicaciones.h"
#include "famecheck.h"

#define ARCHIVO_USUARIOS "datos/usuarios.dat"
#define ARCHIVO_INDICES_USUARIOS "datos/indice_usuarios.idx"
#define ARCHIVO_POSTEOS "datos/posteos.dat"
#define ARCHIVO_POSTEOS_TEMP "datos/posteos_temp.dat"
#define ARCHIVO_POSTEOS_NUE "datos/posteos_nue.dat"

int almacenamiento_cargar_indices(tLista *listaIndices, tPila *pilaLibres, unsigned *ultimoId);
int almacenamiento_guardar_nuevo_usuario(tUsuario *usuario, tIndiceUsuario *indiceACompletar, tPila *pilaLibres);
int almacenamiento_leer_usuario_offset(long offset, tUsuario *usuarioDestino);
int almacenamiento_baja_usuario(long offsetDat, long offsetIdx, tPila *pilaLibres);
int almacenamiento_actualizar_usuario(long offsetDat, tUsuario *usuarioActualizado);
int cargarNPosteos(tListaDoble* listaDoble, unsigned offset, unsigned cantPosteos, int Func(tListaDoble* p, void* informacion, size_t tam_informacion));
int cargarNPosteosFiltrado(tListaDoble* listaDoble, unsigned offset, unsigned cantPosteos, int Func(tListaDoble* p, void* informacion, size_t tam_informacion),
                           int cmp(void* a, void* b), void* parametro);
int cargarNPosteosAtras(tListaDoble* p, unsigned inicio, int n);
int guardarPosteo(tUsuario usuario);
int combinarPosteos();
unsigned almacenamiento_obtener_proximo_id_posteo(void);
int almacenamiento_eliminar_posteo(unsigned idPosteo, const char *nombreUsuario);
int almacenamiento_editar_posteo(unsigned idPosteo, const char *nombreUsuario, const char *nuevoTexto);
void volcarPosteo(tLista* p, FILE* destino);
#endif // ALMACENAMIENTO_H_INCLUDED
