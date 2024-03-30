#include "ficheros.h"

int main(int argc, char **argv) {
    if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) {
        fprintf(stderr, RED, "Error sintaxis. ej: truncar.c <nombre_dispositivo><ninodo><nbytes>\n");
        printf(GRAY);
        exit(1);
    }

    bmount(argv[1]); // montar dispositivo virtual
    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);

    printf(BLUE,"\nNumero de inodo %d \n",ninodo);
    printf(BLUE,"Numero de bytes %d \n",nbytes);
    printf(GRAY);

    if(nbytes == 0) {
        liberar_inodo(ninodo);
    }else{
        mi_truncar_f(ninodo ,nbytes);
    }

    struct STAT stat;
    mi_stat_f(ninodo, &stat);
    printf(BLUE,"tamEnBytesLog",stat.tamEnBytesLog);
    printf(BLUE,"numBloquesOcupados",stat.numBloquesOcupados);
    printf(GRAY);

    bumount();
    return 0;
}