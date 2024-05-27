#include "ficheros.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

/**
 * Funcion para escribir texto en inodos haciendo uso de reservar_inodo(f,6) para obtener ninodo imprimirlo por pantalla y usarlo en mi_write_f()
 */
int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Sintaxis: ./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        return FALLO;
    }

    struct STAT stat;
    int arr_offsets[5] = { 9000, 209000, 30725000, 409605000, 480000000 };
    char* disp = argv[1];
    char* string_fichero = argv[2];
    int diferentes_inodos = atoi(argv[3]);
    int longitud = strlen(string_fichero);
    char buffer[longitud];

    strcpy(buffer, string_fichero);

    if (bmount(disp) == -1) {
        return FALLO;
    }

    int ninodo = reservar_inodo('f', 6);
    printf("Nº inodo reservado: %d\noffset: %d\n", ninodo, arr_offsets[0]);
    int n_escritos = mi_write_f(ninodo, buffer, arr_offsets[0], longitud);
    memset(buffer, 0, longitud);
    mi_stat_f(ninodo, &stat);

    printf("Bytes escritos: %d\n", n_escritos);
    printf("Tamaño en bytes lógicos: %d\n", stat.tamEnBytesLog);
    printf("N. de bloques ocupados: %d\n\n", stat.numBloquesOcupados);

    for (int i = 1; i < 5; ++i) {
        if (diferentes_inodos != 0) {
            ninodo = reservar_inodo('f', 6);
        }
        printf("Nº inodo reservado: %d\noffset: %d\n", ninodo, arr_offsets[i]);
        int bEscritos = mi_write_f(ninodo, string_fichero, arr_offsets[i], longitud);
        memset(buffer, 0, longitud);
        mi_stat_f(ninodo, &stat);

        printf("Bytes escritos: %d\n", bEscritos);
        printf("Tamaño en bytes lógicos: %d\n", stat.tamEnBytesLog);
        printf("N. de bloques ocupados: %d\n\n", stat.numBloquesOcupados);
    }

    bumount();
    return EXITO;
}