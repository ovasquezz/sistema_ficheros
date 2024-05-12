#include "directorios.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

int main(int argc, char** argv) {
    if (argc == 4) {
        char *disco, *ruta;
        int permisos;

        disco = argv[1];
        permisos = atoi(argv[2]);
        ruta = argv[3];

        bmount(disco);
        if(mi_creat(ruta, permisos)<0){
            return FALLO;
        }
    } else {
        perror(RED "Sintaxis incorrecta, sintaxis correcta:./mi_mkdir <disco> <permisos> </ruta>\n");
        return FALLO;
    }
}