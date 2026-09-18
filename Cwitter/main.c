#include <stdio.h>
#include <stdlib.h>
#include "usuarios.h"
#include "tweets.h"
#include "almacenamiento.h"

void mostrar_usuario(void *dato, void *extra)
{
    tUsuario *usuario = (tUsuario *)dato;

    (void)extra;

    printf(
        "ID: %u | Usuario: %s\n",
        usuario->id,
        usuario->usuario
    );
}

void mostrar_resultado(eUsuarioRet resultado)
{
    switch(resultado)
    {
    case USUARIO_REGISTRO_OK:
        printf("Usuario registrado correctamente.\n");
        break;

    case USUARIO_DATOS_INVALIDOS:
        printf("Los datos recibidos son invalidos.\n");
        break;

    case USUARIO_VACIO:
        printf("El nombre de usuario esta vacio.\n");
        break;

    case USUARIO_DEMASIADO_LARGO:
        printf("El nombre de usuario es demasiado largo.\n");
        break;

    case CONTRASENIA_VACIA:
        printf("La contrasenia esta vacia.\n");
        break;

    case CONTRASENIA_DEMASIADO_LARGA:
        printf("La contrasenia es demasiado larga.\n");
        break;

    case USUARIO_DUPLICADO:
        printf("El nombre de usuario ya existe.\n");
        break;

    case USUARIO_DEMASIADO_CORTO:
        printf("El nombre de usuario debe tener al menos 6 caracteres.\n");
        break;

    case CONTRASENIA_DEMASIADO_CORTA:
        printf("La contrasenia debe tener al menos 6 caracteres.\n");
        break;

    case USUARIO_SIN_MEMORIA:
        printf("No hay memoria suficiente.\n");
        break;
    }
}
int main()
{
    tLista usuarios;
    unsigned ultimoId = 0;
    eUsuarioRet resultado;

    lista_crear(&usuarios);

    printf("Caso 1: registro correcto\n");
    resultado = usuario_registrar(&usuarios,&ultimoId,"franco","clave123");
    mostrar_resultado(resultado);

    printf("\nCaso 2: segundo usuario correcto\n");
    resultado = usuario_registrar(&usuarios,&ultimoId,"anaana","prueba456");
    mostrar_resultado(resultado);

    printf("\nCaso 3: usuario duplicado\n");
    resultado = usuario_registrar(&usuarios,&ultimoId,"franco","otraClave");
    mostrar_resultado(resultado);

    printf("\nCaso 4: usuario vacio\n");
    resultado = usuario_registrar(&usuarios,&ultimoId,"","clave789");
    mostrar_resultado(resultado);

    printf("\nCaso 5: usuario de 21 caracteres\n");
    resultado = usuario_registrar(&usuarios, &ultimoId, "123456789012345678901", "clave789");
    mostrar_resultado(resultado);

    printf("\nCaso 6: usuario de 5 caracteres\n");
    resultado = usuario_registrar(&usuarios, &ultimoId, "juane", "abc123");
    mostrar_resultado(resultado);

    printf("\nCaso 7: contrasenia de 5 caracteres\n");
    resultado = usuario_registrar(&usuarios, &ultimoId, "juanes", "abc12");
    mostrar_resultado(resultado);

    printf("\nCaso 8: usuario y contrasenia de 6 caracteres\n");
    resultado = usuario_registrar(&usuarios, &ultimoId, "juanes", "abc123");
    mostrar_resultado(resultado);

    printf("\nUsuarios registrados:\n");
    lista_recorrer(
        &usuarios,
        mostrar_usuario,
        NULL
    );

    printf("\nUltimo ID utilizado: %u\n", ultimoId);

    lista_vaciar(&usuarios);

    return 0;
}
