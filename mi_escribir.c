#include "directorios.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

int main(int argc, char** argv) {
    if (argc != 5) {
        printf(RED "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>" RESET);
        return FALLO;
    }

    char * disp, *camino, *string;
    disp = argv[1];
    camino = argv[2];
    string = argv[3];
    int longitud = strlen(string);
    char buffer[longitud];
    int offset = atoi(argv[4]);
    strcpy(buffer, string);

#if DEBUG9
    fprintf(stderr, "Longitud texto: %d\n", longitud);
#endif

    if (bmount(disp) == -1) {
        return FALLO;
    }
    int n_escritos = mi_write(camino, buffer, offset, longitud);
    memset(buffer, 0, longitud);

    if (n_escritos < 0) {
        n_escritos = 0;
    }
    printf("Bytes escritos: %d\n", n_escritos);

    if (bumount() == -1) {
        fprintf(stderr, RED "Error en mi_escribir.c --> %d: %s\n" RESET, errno, strerror(errno));
        return FALLO;
    }
    return 0;
}