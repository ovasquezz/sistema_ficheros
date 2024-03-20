#include "ficheros.h"
//funcion para validar la sintaxis, montar el dispositivo, llamar mi_chod_f y desmontar dispositivo
int main(int argc, char **argv){
    if(argc !=4){
        return -1;
    }
    else{
        //montar dispositivo
        if(bmount(argv[1])==-1){
            return -1;
        }
        // llamamos a mi_chmod_f 
        unsigned int ninodo=atoi(argv[2]),permisos=atoi(argv[3]);
        if(mi_chmod_f(ninodo,permisos)){
            return -1;
        }
        //desmontar dispotivo
        if(bumount()==-1){
            return -1;
        }
        else{
            return 0;
        }
    }
}