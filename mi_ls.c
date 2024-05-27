#include "directorios.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

int main(int argc, char** argv) {
    if (argc <= 4) {
        char buffer[TAMBUFFER];
        char* disco;
        char* ruta;
        char flag = 0;

        //procesar flag
        if (strlen(argv[1]) == 2 && argv[1][0] == '-' && argv[1][1] == 'l') {
            flag = 1;
            disco = argv[2];
            ruta = argv[3];
        } else {
            disco = argv[1];
            ruta = argv[2];
        }

        bmount(disco);

        memset(buffer, 0, TAMBUFFER);
        int n_entradas = mi_dir(ruta, buffer, flag);
        if (n_entradas < 0) {
            return FALLO;
        }

        printf("Total: %i\n", n_entradas);
        if (n_entradas > 0) {
            if (flag) {
                printf("TIPO\tPERMISOS\tmTIME\tTAMAÑO(Bytes)\tNOMBRE\n------------------------------------------\n%s\n", buffer);
            } else {
                printf("%s\n", buffer);
            }
        }

        bumount(disco);
        return EXITO;
    } else {
        fprintf(stderr, RED "Sintaxis incorrecta: ./mi_ls -l <disco> </ruta_directorio>\n" RESET);
        return FALLO;
    }
}