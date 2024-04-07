/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

#include "ficheros.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        return FALLO;
    }
    if (bmount(argv[1]) == -1) {
        perror("Error en bmount() en leer.c");
        return FALLO;
    }
    int tambuffer = 1024;
    int ninodo = atoi(argv[2]);
    int offset = 0;
    struct STAT stat;
    unsigned char buffer[tambuffer];
    memset(buffer, 0, tambuffer);
    int leidos = 0;
    leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    while (leidos > 0) {
        write(1, buffer, leidos);
        offset += tambuffer;
        memset(buffer, 0, tambuffer);
        leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    }
    fprintf(stderr, "BYTES LEIDOS: %d\n", leidos);
    mi_stat_f(ninodo, &stat);
    fprintf(stderr, "TAMAÑO DE BYTES LEIDOS: %d\n", stat.tamEnBytesLog);

    if (bumount() == -1) {
        perror("Error en bumount() en leer.c");
    }
}