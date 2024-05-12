#include "directorios.h"
int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero> \n");
        return FALLO;
    }

    if (bmount(argv[1]) == -1) {
        perror("Error en bmount() en mi_cat.c");
        return FALLO;
    }

    int tambuffer = BLOCKSIZE * 4;
    const char* camino = argv[2];
    int offset = 0;
    unsigned char buffer[tambuffer];

    memset(buffer, 0, tambuffer);
    int leidos = 0;
    int leidosTotal = 0;
    leidos = mi_read(camino, buffer, offset, tambuffer);
    while (leidos > 0) {
        leidosTotal += leidos;
        write(1, buffer, leidos);
        offset += tambuffer;
        memset(buffer, 0, tambuffer);
        leidos = mi_read(camino, buffer, offset, tambuffer);
    }

    if(leidosTotal < 0){
        mostrar_error_buscar_entrada(leidosTotal);
        leidosTotal = 0;
    }

    fprintf(stderr, "BYTES LEIDOS: %d\n", leidosTotal);

    if (bumount() == -1) {
        perror("Error en bumount() en leer.c");
    }
}