#include "publicaciones.h"

void mostrarPosteo(tPosteo pub)
{
    printf("[Tweet #%u] @%s:\n  \"%s\"\n\n", pub.id, pub.nombreUsuario, pub.publicacion);
}

void crearPosteo (tPosteo* pub, tUsuario user)
{
    strcpy(pub->nombreUsuario, user.usuario);
    int valido = 0;
    while(!valido)
    {
        if (fgets(pub->publicacion, LARGO_MAX, stdin) == NULL) {
            return;
        }

        size_t len = strlen(pub->publicacion);
        char* ultimo = pub->publicacion + len -1;
        if (len > 0 && *ultimo == '\n') {
            *ultimo = '\0';
            len--;
        } else {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            len = MAX_CHARS + 1;
        }

        if (len > MAX_CHARS) {
            printf("Error: el texto supera los %d caracteres. Intente de nuevo.\n\n", MAX_CHARS);
        } else {
            valido = 1;
        }
    }
}

int cmpNombreUsuario(void* a, void* b)
{
    tPosteo* n1 = (tPosteo*) a;
    tPosteo* n2 = (tPosteo*) b;

    return strcmp(n1->nombreUsuario, n2->nombreUsuario);
}

int cmpTexto(void* a, void* b)
{
    tPosteo* n1;
    tPosteo* n2;
    const char *str;
    const char *pat;
    const char *star;
    const char *backtrack;
    char cStr;
    char cPat;

    n1 = (tPosteo*) a;
    n2 = (tPosteo*) b;

    str = n1->publicacion;
    pat = n2->publicacion;
    star = NULL;
    backtrack = NULL;

    while (*str)
    {
        cStr = *str;
        if (cStr >= 'A' && cStr <= 'Z')
        {
            cStr = cStr + 32;
        }

        cPat = *pat;
        if (cPat >= 'A' && cPat <= 'Z')
        {
            cPat = cPat + 32;
        }

        if (*pat == '*')
        {
            star = pat++;
            backtrack = str;
        }
        else if (*pat == '?' || cPat == cStr)
        {
            pat++;
            str++;
        }
        else if (star)
        {
            pat = star + 1;
            str = ++backtrack;
        }
        else
        {
            return 1;
        }
    }

    while (*pat == '*')
    {
        pat++;
    }

    return (*pat == '\0') ? 0 : 1;
}

void iniciarFeed(tFeed* feed)
{
    crearListaD(&feed->p);
    feed->posteo_actual = 0;
    feed->inicio = 0;
    feed->offset = (long)cargarNPosteos(&feed->p, 0,
                   MAX_PUBLICACIONES, agregarAlFinal) * (long)sizeof(tPosteo);
}

void siguientePosteo(tFeed* feed)
{
    tPosteo posteo;
    int i;
    int cargados;
    int ok;
    int j;

    for(i = 0; i < 3; i++)
    {
        ok = obtenerPosicionN(&feed->p, &posteo, sizeof(tPosteo), feed->posteo_actual);

        if (ok != 0)
        {
            cargados = cargarNPosteos(&feed->p, feed->offset,
                                      CAMBIO_PUBLICACIONES, agregarAlFinal);

            if (cargados > 0)
            {
                for (j = 0; j < cargados; j++)
                {
                    quitarDelComienzo(&feed->p, &posteo, sizeof(tPosteo));
                }

                feed->posteo_actual -= cargados;
                feed->offset += (long)cargados * (long)sizeof(tPosteo);
                feed->inicio += (long)cargados * (long)sizeof(tPosteo);

                ok = obtenerPosicionN(&feed->p, &posteo, sizeof(tPosteo), feed->posteo_actual);
            }
        }

        if (ok == 0)
        {
            mostrarPosteo(posteo);
            feed->posteo_actual++;
        }
        else
        {
            break;
        }
    }
}

void posteoAnterior(tFeed* feed)
{
    tPosteo posteo;
    int cargados;
    int objetivo;
    int j;

    feed->posteo_actual -= 6;
    if (feed->posteo_actual < 0)
    {
        feed->posteo_actual = 0;
    }

    objetivo = feed->posteo_actual - 2;

    if (objetivo < 0)
    {
        cargados = cargarNPosteosAtras(&feed->p, feed->inicio, CAMBIO_PUBLICACIONES);

        if (cargados > 0)
        {
            for (j = 0; j < cargados; j++)
            {
                quitarDelComienzo(&feed->p, &posteo, sizeof(tPosteo));
            }

            feed->posteo_actual += cargados;
            feed->offset -= (long)cargados * (long)sizeof(tPosteo);
            feed->inicio -= (long)cargados * (long)sizeof(tPosteo);

            objetivo = feed->posteo_actual - 2;
        }
    }

    siguientePosteo(feed);
}
void cargarPostsFiltrados(tFeed* feed, int cmp(void*a, void*b), void* parametroFiltro)
{
    crearListaD(&feed->p);
    feed->posteo_actual = 0;
    feed->offset = cargarNPosteosFiltrado(&feed->p, 0,
                   MAX_PUBLICACIONES, agregarAlFinal, cmp, parametroFiltro) * sizeof(tPosteo);
    feed->inicio = 0;
}

void siguientePosteoFiltrados(tFeed* feed)
{
    tPosteo posteo;

    if (obtenerPosicionN(&feed->p, &posteo, sizeof(tPosteo), feed->posteo_actual) == 0)
    {
        mostrarPosteo(posteo);
        feed->posteo_actual++;
    }
    else
    {

        if (feed->posteo_actual > 0 &&
            obtenerPosicionN(&feed->p, &posteo, sizeof(tPosteo), feed->posteo_actual - 1) == 0)
        {
            feed->posteo_actual++;
        }
    }
}

void posteoAnteriorFiltrado(tFeed* feed)
{
    tPosteo posteo;
    int objetivo = feed->posteo_actual - 2;

    if (objetivo < 0)
    {
        feed->posteo_actual = 0;
        return;
    }

    if (obtenerPosicionN(&feed->p, &posteo, sizeof(tPosteo), objetivo) == 0)
    {
        mostrarPosteo(posteo);
        feed->posteo_actual--;
    }
}



// POR TEMAS DE PERFORMANCE/ DIFICULTAD NO SE TOMA EN CUENTA MAS DE [MAX_POSTEOS] MENSAJES FILTRADOS, ES DECIR, SIEMPRE SE TOMARAN EN CUENTA LOS ULTIMOS [MAX_POSTEOS] QUE COINCIDAN CON EL FILTRO
