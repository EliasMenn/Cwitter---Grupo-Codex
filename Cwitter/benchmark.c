#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include "indices.h"
#include "global/arbol.h"

#define MAX_CHARS 280
#define LARGO_MAX (MAX_CHARS + 1)
#define MAX_USUARIO 20

typedef struct {
    unsigned id;
    unsigned id_padre;
    char nombreUsuario[MAX_USUARIO];
    char verificado;
    time_t fecha;
    char publicacion[LARGO_MAX];
} tPosteo;

// Contador global para comparaciones en el arbol AVL
static long long g_comparaciones_avl = 0;

static int cmpPosteoIndiceContador(const void* a, const void* b)
{
    g_comparaciones_avl++;
    return cmpPosteoIndice(a, b);
}

// Búsqueda lineal directa en archivo (Sprint 1)
static int busqueda_lineal_archivo(FILE *f, unsigned idBuscado, tPosteo *dest, long long *comparaciones)
{
    tPosteo p;
    *comparaciones = 0;
    fseek(f, 0, SEEK_SET);

    while(fread(&p, sizeof(tPosteo), 1, f) == 1)
    {
        (*comparaciones)++;
        if(p.id == idBuscado)
        {
            if(dest) *dest = p;
            return 1;
        }
    }
    return 0;
}

// Búsqueda en el árbol AVL (Sprint 3)
static int busqueda_avl(tArbol *arbol, int idBuscado, IndPosteo *dest)
{
    IndPosteo clave;
    clave.id_posteo = idBuscado;
    clave.id_padre = 0;
    clave.offset = 0;

    // Se busca tanto en verificados como no verificados
    for(clave.verificado = 1; clave.verificado >= 0; clave.verificado--)
    {
        if(arbol_buscar(arbol, &clave, sizeof(IndPosteo), cmpPosteoIndiceContador) == ENCONTRADO)
        {
            if(dest) *dest = clave;
            return 1;
        }
    }
    return 0;
}

void ejecutar_benchmark_tam(int cantidad, const char *nombreArch)
{
    LARGE_INTEGER freq, t_ini, t_fin;
    QueryPerformanceFrequency(&freq);

    // 1. Crear archivo con la cantidad de tweets
    FILE *f = fopen(nombreArch, "wb");
    if(!f)
    {
        printf("Error al crear archivo %s\n", nombreArch);
        return;
    }

    tPosteo p;
    memset(&p, 0, sizeof(tPosteo));
    strcpy(p.nombreUsuario, "usuarioTest");
    p.verificado = 'N';
    p.fecha = 1700000000;
    strcpy(p.publicacion, "Texto de prueba para benchmark de rendimiento en Algoritmos y Estructuras de Datos.");

    for(int i = 1; i <= cantidad; i++)
    {
        p.id = i;
        p.id_padre = (i % 5 == 0) ? (i - 1) : 0;
        p.verificado = (i % 2 == 0) ? 'S' : 'N';
        fwrite(&p, sizeof(tPosteo), 1, f);
    }
    fclose(f);

    // 2. Cargar en Árbol AVL
    tArbol arbol;
    crear_arbol(&arbol);

    f = fopen(nombreArch, "rb");
    long pos = ftell(f);
    IndPosteo ip;
    while(fread(&p, sizeof(tPosteo), 1, f) == 1)
    {
        ip.verificado = (p.verificado == 'S');
        ip.id_posteo = (int)p.id;
        ip.id_padre = (int)p.id_padre;
        ip.offset = pos;
        arbol_insertar(&arbol, &ip, sizeof(IndPosteo), cmpPosteoIndice);
        pos = ftell(f);
    }

    // ID para el PEOR CASO (no existe en la colección)
    unsigned idInexistente = cantidad + 9999;

    // --- Medicion 1: Búsqueda Lineal en Disco (Sprint 1) ---
    long long comp_lineal = 0;
    // Promediamos repeticiones para estabilidad
    int repeticiones = (cantidad <= 10000) ? 10 : 3;
    double tiempo_lineal_acum = 0.0;

    for(int r = 0; r < repeticiones; r++)
    {
        QueryPerformanceCounter(&t_ini);
        busqueda_lineal_archivo(f, idInexistente, NULL, &comp_lineal);
        QueryPerformanceCounter(&t_fin);
        tiempo_lineal_acum += (double)(t_fin.QuadPart - t_ini.QuadPart) * 1000.0 / freq.QuadPart;
    }
    double tiempo_lineal_ms = tiempo_lineal_acum / repeticiones;

    // --- Medicion 2: Búsqueda AVL en Memoria (Sprint 3) ---
    g_comparaciones_avl = 0;
    double tiempo_avl_acum = 0.0;
    int rep_avl = 1000; // Al ser tan rapido, repetimos 1.000 veces para medir con precision

    for(int r = 0; r < rep_avl; r++)
    {
        QueryPerformanceCounter(&t_ini);
        busqueda_avl(&arbol, idInexistente, NULL);
        QueryPerformanceCounter(&t_fin);
        tiempo_avl_acum += (double)(t_fin.QuadPart - t_ini.QuadPart) * 1000.0 / freq.QuadPart;
    }
    double tiempo_avl_ms = tiempo_avl_acum / rep_avl;
    long long comp_avl = g_comparaciones_avl / rep_avl;

    fclose(f);
    arbol_vaciar(&arbol);
    remove(nombreArch);

    printf("| %7d | %16.3f ms | %15ld | %13.5f ms | %12ld |\n",
           cantidad, tiempo_lineal_ms, (long)comp_lineal, tiempo_avl_ms, (long)comp_avl);
}

int main()
{
    printf("\n========================================================================================\n");
    printf("         BENCHMARK DE ESCALABILIDAD: BUSQUEDA POR ID (PEOR CASO)\n");
    printf("========================================================================================\n\n");
    printf("|  Tweets | T. Lineal (Disco) | Comp. Lineales | T. AVL (Memoria) | Comp. Arbol  |\n");
    printf("|--------:|------------------:|---------------:|-----------------:|-------------:|\n");

    int cantidades[] = {100, 1000, 10000, 100000};
    int n = sizeof(cantidades) / sizeof(cantidades[0]);

    for(int i = 0; i < n; i++)
    {
        char nombre[64];
        snprintf(nombre, sizeof(nombre), "datos/bench_%d.dat", cantidades[i]);
        ejecutar_benchmark_tam(cantidades[i], nombre);
    }

    printf("\n========================================================================================\n");
    printf("Benchmark completado con exito.\n");
    return 0;
}
