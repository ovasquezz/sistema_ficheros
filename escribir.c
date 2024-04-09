#include "ficheros.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/
//funcion para escribir texto en inodos haciendo uso de reservar_inodo(f,6) para obtener ninodo imprimirlo por pantalla y usarlo en mi_write_f()
int main(int argc, char** argv) {
    if(argc != 4){
		printf("Sintaxis: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
		return -1;
	}
    void* nombre_fichero = argv[1];
    char* string = argv[2];
    int reserva_inodo = atoi(argv[3]); //offset
    int longitud = strlen(string);
    char buffer[longitud];
    int OFFSETS[5] = { 9000,209000,30725000,409605000,480000000 };
    strcpy(buffer, string);
    struct STAT stat;
    if (bmount(nombre_fichero) == -1) {
        return FALLO;
    }
    int n_inodo = reservar_inodo('f', 6);
    int n_escritos = mi_write_f(n_inodo, buffer, OFFSETS[0], longitud);
    memset(buffer, 0, longitud);
    mi_stat_f(n_inodo, &stat);

    printf("Nº inodo reservado: %d\noffset: %d\nBytes escritos: %d\n", n_inodo, OFFSETS[0], n_escritos);
    printf("Tamaño en bytes lógicos: %d\n", stat.tamEnBytesLog);
    printf("N. de bloques ocupados: %d\n\n", stat.numBloquesOcupados);

    for (int i = 1; i < 5; ++i) {
        if (reserva_inodo != 0) {
            n_inodo = reservar_inodo('f', 6);
        }
        int bEscritos = mi_write_f(n_inodo, string, OFFSETS[i], longitud);
        memset(buffer, 0, longitud);
        mi_stat_f(n_inodo, &stat);
        printf("\nNº inodo reservado: %d\noffset: %d\nBytes escritos: %d\n", n_inodo, OFFSETS[i], bEscritos);
        printf("Tamaño en bytes lógicos: %d\n", stat.tamEnBytesLog);
        printf("N. de bloques ocupados: %d\n\n", stat.numBloquesOcupados);
    }

    if (bumount() == -1) {
        fprintf(stderr, "Error en escribir.c --> %d: %s\n", errno, strerror(errno));
        return FALLO;
    }
    return 0;
}