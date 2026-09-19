#include "publicaciones.h"

void mostrarPosteo(tPosteo pub)
{
    printf("%s%s\n\t", pub.nombreUsuario, pub.publicacion);
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
        // quitar el \n si esta presente
        if (len > 0 && *ultimo == '\n') {
            *ultimo = '\0';
            len--;
        } else {
            // no habia \n: la linea era mas larga que el buffer, limpiar stdin
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            len = MAX_CHARS + 1; // forzar que se considere invalido
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
    tPosteo* n1 = (tPosteo*) a;
    tPosteo* n2 = (tPosteo*) b;

    const char *str = n1->publicacion;   // text
    const char *pat = n2->publicacion;   // pattern, e.g. "*quick*"
    const char *star = NULL, *backtrack = NULL;

    while (*str)
    {
        if (*pat == '*')
        {
            star = pat++;
            backtrack = str;
        }
        else if (*pat == '?' || *pat == *str)
        {
            pat++;
            str++;
        }
        else if (star)
        {
            pat = star + 1;
            str = ++backtrack;
        }
        else return 1;   // no match
    }

    while (*pat == '*')
        pat++;

    return (*pat == '\0') ? 0 : 1;   // 0 = match, 1 = no match
}
