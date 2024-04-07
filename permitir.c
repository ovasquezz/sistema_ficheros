/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

#include "ficheros.h"
//funcion para validar la sintaxis, montar el dispositivo, llamar mi_chod_f y desmontar dispositivo
int main(int argc, char **argv){
    if(argc !=4){
        fprintf(stderr, "Sintaxis correcta: ./permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return FALLO;
    }
    else{
        //montar dispositivo
        if(bmount(argv[1])==-1){
            perror("Error al montar el dispositivo");
            return FALLO;
        }
        // llamamos a mi_chmod_f
        unsigned int ninodo=atoi(argv[2]);
        unsigned int permisos=atoi(argv[3]);
        if(mi_chmod_f(ninodo,permisos) == -1){
            perror("Error en el chmod");
            return FALLO;
        }
        //desmontar dispotivo
        if(bumount()==-1){
            perror("Error al desmontar el dispositivo");
            return FALLO;
        }
        else{
            return 0;
        }
    }
}