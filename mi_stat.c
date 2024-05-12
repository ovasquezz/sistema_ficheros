#include "directorios.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

int main(int argc, char** argv) {

    if (argc < 3) {
        fprintf(stderr, RED "Error al introducir el comando\n" RESET);
        return -1;
    }

    bmount(argv[1]);
    char* ruta;
    ruta = argv[2];
    struct STAT info;

    if (mi_stat(ruta, &info) != -1) {
        printf("tipo: %c\n", info.tipo);
        printf("permisos: %d\n", info.permisos);
        printf("atime: %s", asctime(gmtime(&info.atime)));
        printf("ctime: %s", asctime(gmtime(&info.ctime)));
        printf("mtime: %s", asctime(gmtime(&info.mtime)));
        printf("nlinks: %d\n", info.nlinks);
        printf("tamEnBytesLog: %d\n", info.tamEnBytesLog);
        printf("numBloquesOcupados: %d\n", info.numBloquesOcupados);
    }

    bumount(argv[1]);
    return 0;
}