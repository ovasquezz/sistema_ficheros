#include "directorios.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, RED "Sintaxis incorrecta:./mi_link disco /ruta_fichero_original /ruta_enlace\n " RESET);
        return FALLO;
    }
    char *ruta_fichero_original, *ruta_enlace;

    ruta_fichero_original = argv[2];
    ruta_enlace = argv[3];

    //Comprobamos que la ruta es válida
    if (ruta_fichero_original[strlen(ruta_fichero_original) - 1] == '/') {
        fprintf(stderr, RED "Error: %s no es un fichero.\n" RESET, ruta_fichero_original);
        return FALLO;
    }

    if (ruta_enlace[strlen(ruta_enlace) - 1] == '/') {
        fprintf(stderr, RED "Error: %s no es un fichero.\n" RESET, ruta_enlace);
        return FALLO;
    }


    bmount(argv[1]);
    if(mi_link(ruta_fichero_original, ruta_enlace) < 0){
        return FALLO;
    }
    bumount(argv[1]);
}