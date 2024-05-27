#include "directorios.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

int main(int argc, char** argv) {

    if (argc < 4) {
        fprintf(stderr, RED "Sintxis incorrecta: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n" RESET);
        return FALLO;
    }

    bmount(argv[1]);
    char ruta[strlen(argv[3])];
    int permisos = atoi(argv[2]);

    if ((permisos >= 0) && (permisos <= 7)) {
        strcpy(ruta, argv[3]);
        mi_chmod(ruta, permisos);
    }

    bumount(argv[1]);
    return EXITO;
}