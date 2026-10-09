#include "famecheck.h"


typedef struct {
    char *datos;
    size_t tamanio;
} tBufferHttp;

static size_t callback_escribir(void *contenido, size_t tamElem, size_t cantElem, void *contexto)
{
    size_t tamTotal = tamElem * cantElem;
    tBufferHttp *buf = (tBufferHttp *)contexto;
    char *nuevo = realloc(buf->datos, buf->tamanio + tamTotal + 1);
    if (!nuevo) return 0;
    buf->datos = nuevo;
    memcpy(&(buf->datos[buf->tamanio]), contenido, tamTotal);
    buf->tamanio += tamTotal;
    buf->datos[buf->tamanio] = '\0';
    return tamTotal;
}

void famecheck_inicializar(void)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void famecheck_limpiar(void)
{
    curl_global_cleanup();
}

void famecheck_registrar_fallo(const char *operacion, const char *detalle)
{
    FILE *log = fopen(ARCHIVO_LOG_FALLOS, "at");
    if (!log) return;

    time_t ahora = time(NULL);
    struct tm *t = localtime(&ahora);
    char fechaHora[32];
    strftime(fechaHora, sizeof(fechaHora), "%Y-%m-%d %H:%M:%S", t);

    fprintf(log, "[%s] Operacion: %s | Detalle: %s\n", fechaHora, operacion, detalle);
    fclose(log);
}

static int ejecutar_post(const char *url, const char *bodyJson, tBufferHttp *resp, long *httpCode)
{
    CURL *curl = curl_easy_init();
    if (!curl) return 0;

    struct curl_slist *headers = NULL;
    char authHeader[160];
    snprintf(authHeader, sizeof(authHeader), "Authorization: Bearer %s", FAMECHECK_API_KEY);
    headers = curl_slist_append(headers, authHeader);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    resp->datos = malloc(1);
    resp->tamanio = 0;
    if (resp->datos) resp->datos[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyJson ? bodyJson : "");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_escribir);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_HTTP);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        char errBuf[256];
        snprintf(errBuf, sizeof(errBuf), "curl_easy_perform error: %s", curl_easy_strerror(res));
        famecheck_registrar_fallo("POST", errBuf);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        if (resp->datos) { free(resp->datos); resp->datos = NULL; }
        return 0;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return 1;
}

int famecheck_verificar_cuenta(const char *nombreUsuario)
{
    cJSON *raiz = cJSON_CreateObject();
    char url[256];
    char desc[128];
    long code = 0;
    int verificado = 0;
    if (!raiz) return 0;
    cJSON_AddStringToObject(raiz, "Nombre", nombreUsuario);
    char *body = cJSON_PrintUnformatted(raiz);
    cJSON_Delete(raiz);


    snprintf(url, sizeof(url), "%s/api/Cwitter/Usuario", FAMECHECK_BASE_URL);

    tBufferHttp resp;

    if (ejecutar_post(url, body, &resp, &code)) {
        //////////////////////
            printf("\n>>> [RESPUESTA FAMECHECK] HTTP %ld: %s <<<\n", code, resp.datos);
    //////////////////////////////
        if (code == 200 || code == 201) {
            cJSON *jsonResp = cJSON_Parse(resp.datos);
            if (jsonResp) {
                cJSON *campoVerif = cJSON_GetObjectItemCaseSensitive(jsonResp, "verificado");
                if (cJSON_IsBool(campoVerif) && cJSON_IsTrue(campoVerif)) {
                    verificado = 1;
                }
                cJSON_Delete(jsonResp);
            }
        } else {
            snprintf(desc, sizeof(desc), "HTTP %ld al verificar @%s", code, nombreUsuario);
            famecheck_registrar_fallo("POST /api/Cwitter/Usuario", desc);
        }
        if (resp.datos) free(resp.datos);
    }

    cJSON_free(body);
    return verificado;
}

int famecheck_reportar_tweet(const char *nombreUsuario, const char *mensaje)
{
    cJSON *raiz = cJSON_CreateObject();
    char url[256];
    char desc[256];
    long code = 0;
    int exito = 0;
    if (!raiz) return 0;
    cJSON_AddStringToObject(raiz, "NombreUsuario", nombreUsuario);
    cJSON_AddStringToObject(raiz, "Mensaje", mensaje);
    char *body = cJSON_PrintUnformatted(raiz);
    cJSON_Delete(raiz);

    snprintf(url, sizeof(url), "%s/api/Cwitter/Cweet", FAMECHECK_BASE_URL);

    tBufferHttp resp;

    if (ejecutar_post(url, body, &resp, &code)) {
        if (code == 201) {
            exito = 1;
        } else {
            snprintf(desc, sizeof(desc), "HTTP %ld al reportar tweet de @%s", code, nombreUsuario);
            famecheck_registrar_fallo("POST /api/Cwitter/Cweet", desc);
        }
        if (resp.datos) free(resp.datos);
    }

    cJSON_free(body);
    return exito;
}
