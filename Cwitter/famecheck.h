#ifndef FAMECHECK_H_INCLUDED
#define FAMECHECK_H_INCLUDED

#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>

#define TIMEOUT_HTTP 6L
#define FAMECHECK_BASE_URL "https://algoritmos-api.azurewebsites.net"
#define FAMECHECK_API_KEY  "PiensoLuegoCopio"
#define ARCHIVO_LOG_FALLOS "datos/log_fallos.txt"

void famecheck_inicializar(void);
void famecheck_limpiar(void);
int famecheck_verificar_cuenta(const char *nombreUsuario);
int famecheck_reportar_tweet(const char *nombreUsuario, const char *mensaje);
void famecheck_registrar_fallo(const char *operacion, const char *detalle);

#endif // FAMECHECK_H_INCLUDED
