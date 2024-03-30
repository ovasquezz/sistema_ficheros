#include "ficheros.h"

int main(int argc, int** argv) {
    if (argc != 3) {
        return -1;
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
        memset(buffer, 0, tambuffer);
    }
    fprintf(stderr, "BYTES LEIDOS: %d\n", leidos);
    mi_stat_f(ninodo, &stat);
    fprintf(stderr, "TAMAÑO DE BYTES LEIDOS: %d\n", stat.tamEnBytesLog);
}