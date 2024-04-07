/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

#include "ficheros.h"

int main(int argc, char** argv) {
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) {
        fprintf(stderr, RED "Error sintaxis. ej: truncar.c <nombre_dispositivo><ninodo><nbytes>\n");
        printf(GRAY);
        exit(1);
    }

    bmount(argv[1]); // montar dispositivo virtual
    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);

    struct STAT stat;
    mi_stat_f(ninodo, &stat);
    printf(YELLOW);
    printf("DATOS INODO INICIAL:\n");

    printf("tipo %d\n", stat.tipo);
    printf("permisos %d\n", stat.permisos);
    printf("atime %ld\n", stat.atime);
    printf("ctime %ld\n", stat.ctime);
    printf("mtime %ld\n", stat.mtime);
    printf("nlinks %d\n", stat.nlinks);
    printf("tamEnBytesLog %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados %d\n", stat.numBloquesOcupados);
    printf(GRAY);

    if (nbytes == 0) {
        liberar_inodo(ninodo);
    } else {
        mi_truncar_f(ninodo, nbytes);
    }

    mi_stat_f(ninodo, &stat);
    printf(BLUE);
    printf("DATOS INODO FINAL:\n");

    printf("tipo %d\n", stat.tipo);
    printf("permisos %d\n", stat.permisos);
    printf("atime %ld\n", stat.atime);
    printf("ctime %ld\n", stat.ctime);
    printf("mtime %ld\n", stat.mtime);
    printf("nlinks %d\n", stat.nlinks);
    printf("tamEnBytesLog %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados %d\n", stat.numBloquesOcupados);
    printf(GRAY);

    bumount();
    return 0;
}