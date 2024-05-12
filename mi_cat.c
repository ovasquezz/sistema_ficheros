#include "directorios.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, RED "Sintaxis: ./mi_cat <disco> </ruta_fichero> \n" RESET);
        return FALLO;
    }

    if(argv[2][strlen(argv[2]) - 1] == '/') {
        fprintf(stderr, RED "Error: %s es un directorio.\n" RESET, argv[2]);
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
        write(1, buffer, leidos);
        memset(buffer, 0, tambuffer);
        offset += tambuffer;
        leidosTotal += leidos;
        leidos = mi_read(camino, buffer, offset, tambuffer);
    }

    if(leidosTotal < 0){
        mostrar_error_buscar_entrada(leidosTotal);
        leidosTotal = 0;
    }

    fprintf(stderr, "\nBYTES LEIDOS: %d\n", leidosTotal);

    if (bumount() == -1) {
        perror("Error en bumount() en leer.c");
    }
}