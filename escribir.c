#include "ficheros.h"
//funcion para escribir texto en inodos haciendo uso de reservar_inodo(f,6) para obtener ninodo imprimirlo por pantalla y usarlo en mi_write_f()
int main(int argc,char **argv){
    void *nombre_fichero=argv[1];
    char *string=argv[2];
    int reserva_inodo= atoi(argv[3]); //offset
    int longitud=strlen(string);
    char buffer[longitud];
    int OFFSETS[5]={9000,209000,30725000,409605000,480000000};
    strcpy(buffer,longitud);
    struct STAT stat;
    if(bumount(nombre_fichero)==-1){
        return -1;
    }
    int n_inodo=reservar_inodo('f',6);
    int n_escritos=mi_write_f(n_inodo,buffer,OFFSETS[0],longitud);
    memset(buffer,0,longitud);
    mi_stat_f(n_inodo,&stat);
}