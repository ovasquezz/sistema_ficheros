#include "directorios.h"

int main(int argc, char** argv) {
    if (argc >= 4) {
        char ruta[strlen(argv[3])];
        strcpy(ruta, argv[3]);

        bmount(argv[1]);
        mi_creat(ruta, atoi(argv[2]));
    } else {
        perror("Sintaxis incorrecta, sintaxis correcta:./mi_mkdir <disco> <permisos> </ruta>\n");
        return FALLO;
    }
}