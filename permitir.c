/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

#include "ficheros.h"

/**
 * Funcion para validar la sintaxis, montar el dispositivo, llamar mi_chod_f y desmontar dispositivo
 */
int main(int argc, char** argv) {
    char* disp;
    if (argc != 4) {
        perror("Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return FALLO;
    } else {
        disp = argv[1];
        unsigned int ninodo = atoi(argv[2]);
        unsigned int permisos = atoi(argv[3]);

        if (bmount(disp) == -1) {
            perror("Error al montar el dispositivo");
            return FALLO;
        }
        //LLamada chmod
        if (mi_chmod_f(ninodo, permisos) == -1) {
            perror("Error en el chmod");
            return FALLO;
        }

        if (bumount() == -1) {
            perror("Error al desmontar el dispositivo");
            return FALLO;
        }
        return EXITO;

    }
}