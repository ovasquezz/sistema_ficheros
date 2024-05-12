#include "directorios.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

int main(int argc, char** argv) {
    bmount(argv[1]);
    if (argc <= 4) {
        char buffer[TAMBUFFER];
        memset(buffer, 0, TAMBUFFER);
        int n_entradas = mi_dir(argv[2], buffer);
        if (n_entradas < 0)
            return FALLO;
        printf("Total: %i\n", n_entradas);
        if (n_entradas > 0)
            printf("TIPO\tPERMISOS\tmTIME\tTAMAÑO(Bytes)\tNOMBRE\n------------------------------------------\n%s\n", buffer);
        bumount(argv[1]);
        return EXITO;
    } else {
        fprintf(stderr, RED "Sintaxis incorrecta: ./mi_ls <disco> </ruta_directorio>\n" RESET);
        return FALLO;
    }
}