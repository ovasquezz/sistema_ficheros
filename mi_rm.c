#include "directorios.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, RED "Sintexis incorrecta:./mi_rm disco /ruta\n" RESET);
    }
    bmount(argv[1]);
    if (strcmp(argv[2], "/") == 0) {
        fprintf(stderr, RED "No se puede eliminar el directorio raiz\n" RESET);
        return FALLO;
    }
    mi_unlink(argv[2]);
    bumount(argv[1]);
}