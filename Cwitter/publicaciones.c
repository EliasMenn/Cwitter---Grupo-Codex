#include "publicaciones.h"

static int obtenerNivelProfundidad(unsigned idPadre)
{
    int nivel = 0;
    IndPosteo p;
    unsigned actual = idPadre;

    while(actual != 0 && nivel < 50 && indices_buscar_posteo((int)actual, &p))
    {
        nivel++;
        actual = (unsigned)p.id_padre;
    }
    return nivel;
}

void mostrarPosteo(tPosteo pub)
{
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M", localtime(&pub.fecha));

    // Calcula el nivel de profundidad del posteo para aplicar sangria al posteo
    int nivel = obtenerNivelProfundidad(pub.id_padre);

    if(nivel == 0)
    {
        printf("[Tweet #%u] %s @%s[%c]:\n", pub.id, buffer, pub.nombreUsuario, pub.verificado);
        printf("  \"%s\"\n\n", pub.publicacion);
    }
    else
    {
        int espaciosHeader = (nivel - 1) * 4 + 2;
        int espaciosCuerpo = espaciosHeader + 6;

        for(int i = 0; i < espaciosHeader; i++)
        {
            putchar(' ');
        }

        printf("[Tweet #%u] %s @%s[%c] (en respuesta a #%u):\n", pub.id, buffer, pub.nombreUsuario, pub.verificado, pub.id_padre);

        for(int i = 0; i < espaciosCuerpo; i++)
        {
            putchar(' ');
        }

        printf("\"%s\"\n\n", pub.publicacion);
    }
}

void crearPosteo (tPosteo* pub, tUsuario user)
{
    strcpy(pub->nombreUsuario, user.usuario);
    pub->verificado = user.verificado;
    pub->fecha = time(NULL);
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

/* La posicion de la ventana se deduce de los posteos que contiene: cada tPosteo trae
   su id y si es verificado, que es justo la clave del indice. */
static void claveDePosteo(const tPosteo* p, IndPosteo* clave)
{
    clave->verificado = (p->verificado == 'S');
    clave->id_posteo = (int)p->id;
    clave->id_padre = (int)p->id_padre;
    clave->offset = 0;                  /* no se usa para comparar */
}

/* Ultimo posteo de la ventana. Devuelve 0 si lo encontro, -1 si la lista esta vacia. */
static int ultimoDeLaVentana(tListaDoble* p, tPosteo* ultimo)
{
    tPosteo aux;
    int n = 0;
    int hay = 0;

    while(obtenerPosicionN(p, &aux, sizeof(tPosteo), n) == 0)
    {
        *ultimo = aux;
        hay = 1;
        n++;
    }
    return hay ? 0 : -1;
}

/* Agrega al final de la ventana los CAMBIO_PUBLICACIONES posteos que siguen al ultimo. */
static int cargarSiguientes(tFeed* feed)
{
    tPosteo ultimo;
    IndPosteo clave;

    if(ultimoDeLaVentana(&feed->p, &ultimo) != 0)
        return cargarNPosteos(&feed->p, NULL, CAMBIO_PUBLICACIONES, agregarAlFinal);

    claveDePosteo(&ultimo, &clave);
    return cargarNPosteos(&feed->p, &clave, CAMBIO_PUBLICACIONES, agregarAlFinal);
}

/* Agrega al comienzo de la ventana los CAMBIO_PUBLICACIONES posteos que van antes del primero. */
static int cargarAnteriores(tFeed* feed)
{
    tPosteo primero;
    IndPosteo clave;

    if(obtenerPosicionN(&feed->p, &primero, sizeof(tPosteo), 0) != 0)
        return 0;

    claveDePosteo(&primero, &clave);
    return cargarNPosteosAtras(&feed->p, &clave, CAMBIO_PUBLICACIONES);
}

void iniciarFeed(tFeed* feed)
{
    crearListaD(&feed->p);
    feed->posteo_actual = 0;
    cargarNPosteos(&feed->p, NULL, MAX_PUBLICACIONES, agregarAlFinal);
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
            cargados = cargarSiguientes(feed);

            if (cargados > 0)
            {
                for (j = 0; j < cargados; j++)
                {
                    quitarDelComienzo(&feed->p, &posteo, sizeof(tPosteo));
                }

                feed->posteo_actual -= cargados;

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
    int j;

    /* retroceder dos paginas (la que se mostro y la anterior). OJO: no limitar a 0 todavia,
       un valor negativo significa "faltan posteos antes del comienzo de la ventana" */
    feed->posteo_actual -= 6;

    while (feed->posteo_actual < 0)
    {
        cargados = cargarAnteriores(feed);
        if (cargados == 0)
            break;                      /* no hay mas posteos antes: estamos en el principio */

        /* entraron al comienzo: para mantener el tamanio de la ventana salen del final */
        for (j = 0; j < cargados; j++)
        {
            quitarDelFinal(&feed->p, &posteo, sizeof(tPosteo));
        }

        feed->posteo_actual += cargados;
    }

    if (feed->posteo_actual < 0)
    {
        feed->posteo_actual = 0;
    }

    siguientePosteo(feed);
}
void cargarPostsFiltrados(tFeed* feed, int cmp(void*a, void*b), void* parametroFiltro)
{
    crearListaD(&feed->p);
    feed->posteo_actual = 0;
    cargarNPosteosFiltrado(&feed->p, NULL, MAX_PUBLICACIONES, agregarAlFinal, cmp, parametroFiltro);
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
