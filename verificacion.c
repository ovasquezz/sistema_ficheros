
#include "verificacion.h"
#define DEBUG13 0

/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

int main(int argc, char** argv) {
    char* disp, * directorio;
    //Sintaxis
    if (argc != 3) {
        fprintf(stderr, RED "Uso: ./verificacion <nombre_dispositivo> <directorio_simulación>\n" RESET);
        return FALLO;
    }

    //Asignar variables
    disp = argv[1];
    directorio = argv[2];

    //Montar el dispositivo virtual.
    bmount(disp);

    //Calcular el nº de entradas del directorio de simulación a partir del stat de su inodo.
    struct STAT stat;
    mi_stat(directorio, &stat);
    int numentradas = stat.tamEnBytesLog / sizeof(struct entrada);

#if DEBUG13
    fprintf(stderr, "dir_sim: %s\n", directorio);
    fprintf(stderr, "numentradas: %d NUMPROCESOS: %d\n", numentradas, NUMPROCESOS);
#endif

    if (numentradas != NUMPROCESOS) {
        fprintf(stderr, RED "El numero de entradas no coincide con el numero de procesos.\n" RESET);
        return FALLO;
    }

    // Crear el fichero "informe.txt" dentro del directorio de simulación.
    char informe[strlen(directorio) + strlen("informe.txt")];
    strcpy(informe, directorio);
    strcat(informe, "informe.txt");
    if (mi_creat(informe, 6) < 0) {
        fprintf(stderr, "Error al crear informe.txt\n");
        return FALLO;
    }

    //Leer los directorios correspondientes a los procesos de golpe (Mejora)
    struct entrada entradas[NUMPROCESOS * sizeof(struct entrada)];
    mi_read(directorio, entradas, 0, sizeof(entradas));

    int n_escritos = 0;
    for (int i = 0; i < numentradas; i++) {
        // Extraer el PID a partir del nombre de la entrada y guardarlo en el registro info
        struct INFORMACION info;
        info.nEscrituras = 0;
        info.pid = atoi(strchr(entradas[i].nombre, '_') + 1);

        char prueba[strlen(directorio) + strlen(entradas[i].nombre) + strlen("prueba.dat")];
        strcpy(prueba, argv[2]);
        strcat(prueba, entradas[i].nombre);
        strcat(prueba, "/prueba.dat");

        //Mientras haya escrituras en prueba.dat
        int offset = 0;
        int cant_registros_buffer_escrituras = 256;
        struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        while (mi_read(prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0) {
            for (int j = 0; j < cant_registros_buffer_escrituras; j++) {
                //Si la escritura es válida
                if (buffer_escrituras[j].pid == info.pid) {
                    //Si es la primera escritura validada entonces
                    //inicializar los registros significativos con los datos de esa escritura.
                    if (info.nEscrituras == 0) {
                        info.PrimeraEscritura = buffer_escrituras[j];
                        info.UltimaEscritura = buffer_escrituras[j];
                        info.MenorPosicion = buffer_escrituras[j];
                        info.MayorPosicion = buffer_escrituras[j];
                    }
                    //Si no comparar nº de escritura (para obtener primera y última) y actualizarlas si es preciso
                    else {
                        if (buffer_escrituras[j].nEscritura < info.PrimeraEscritura.nEscritura) {
                            info.PrimeraEscritura = buffer_escrituras[j];
                        }

                        if (buffer_escrituras[j].nEscritura > info.UltimaEscritura.nEscritura) {
                            info.UltimaEscritura = buffer_escrituras[j];
                        }

                        if (buffer_escrituras[j].nRegistro < info.MenorPosicion.nRegistro) {
                            info.MenorPosicion = buffer_escrituras[j];
                        }

                        if (buffer_escrituras[j].nRegistro > info.MayorPosicion.nRegistro) {
                            info.MayorPosicion = buffer_escrituras[j];
                        }
                    }
                    info.nEscrituras++;
                }
            }
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
            offset += sizeof(buffer_escrituras);
        }

#if DEBUG13
        printf("[%i) %d escrituras validadas en %s]\n", (i + 1), info.nEscrituras, prueba);
#endif

        char buffer[BLOCKSIZE];
        char stringFecha[50];
        memset(buffer, 0, BLOCKSIZE);

        //Añadir la información del struct info al fichero informe.txt por el final.
        sprintf(buffer, "PID: %i\nNumero de escrituras: %i\n", info.pid, info.nEscrituras);

        strftime(stringFecha, sizeof(stringFecha), "%a %d-%m-%Y %H:%M:%S", localtime(&info.PrimeraEscritura.fecha));
        sprintf(buffer + strlen(buffer), "%s\t%i\t%i\t%s\n", "Primera escritura", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, stringFecha);

        strftime(stringFecha, sizeof(stringFecha), "%a %d-%m-%Y %H:%M:%S", localtime(&info.UltimaEscritura.fecha));
        sprintf(buffer + strlen(buffer), "%s\t%i\t%i\t%s\n", "Ultima escritura ", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, stringFecha);

        strftime(stringFecha, sizeof(stringFecha), "%a %d-%m-%Y %H:%M:%S", localtime(&info.MenorPosicion.fecha));
        sprintf(buffer + strlen(buffer), "%s\t%i\t%i\t%s\n", "Menor posicion   ", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, stringFecha);

        strftime(stringFecha, sizeof(stringFecha), "%a %d-%m-%Y %H:%M:%S", localtime(&info.MayorPosicion.fecha));
        sprintf(buffer + strlen(buffer), "%s\t%i\t%i\t%s\n\n", "Mayor posicion   ", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, stringFecha);

        if ((n_escritos += mi_write(informe, buffer, n_escritos, strlen(buffer))) < 0) {
            printf(RED "Error escritura fichero: '%s'\n" RESET, informe);
            bumount();
            return FALLO;
        }
    }
    bumount();
}