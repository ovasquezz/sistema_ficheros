#include "simulacion.h"
#define DEBUG12a 0
#define DEBUG12b 0

int acabados = 0;

int main(int argc, char** argv) {
    signal(SIGCHLD, reaper);
    time_t fecha = time(NULL);
    pid_t pid;
    struct REGISTRO registro;
    char* disco;
    char stringFecha[16]; //fecha aaaammddhhmmss
    char stringDirectorioSimulacion[strlen(stringFecha) + 8]; // nombre directorio: /simul_aaaammddhhmmss/
    char stringDirectorioProceso[strlen(stringDirectorioSimulacion) + 8 + 10]; // nombre directorio del proceso: /simul_aaaammddhhmmss/proceso_[pid del proceso]/
    char stringFicheroPrueba[strlen(stringDirectorioProceso) + 10]; // nombre del fichero final para cada proceso: /simul_aaaammddhhmmss/proceso_[pid del proceso]/prueba.dat


    if (argc != 2) {
        fprintf(stderr, RED "Error de sintaxis: ./simulacion <nombre_dispositivo>\n" RESET);
        return FALLO;
    }

    disco = argv[1];
    bmount(disco);

    //Creamos el directorio de simulación
    strftime(stringFecha, sizeof(stringFecha), "%Y%m%d%H%M%S", localtime(&fecha));
    sprintf(stringDirectorioSimulacion, "/simul_%s/", stringFecha);

    if (mi_creat(stringDirectorioSimulacion, 6) < 0) {
        bumount();
        return FALLO;
    }

    printf("*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS *** \n", NUMPROCESOS, NUMESCRITURAS);

    int proceso;
    for (proceso = 1; proceso <= NUMPROCESOS; proceso++) {
        pid = fork();
        if (pid == 0) { //hijo
            bmount(disco);

            //Crear el directorio del proceso hijo añadiendo el PID al nombre.
            sprintf(stringDirectorioProceso, "%sproceso_%d/", stringDirectorioSimulacion, getpid());
            if (mi_creat(stringDirectorioProceso, 6) < 0) {
                fprintf(stderr, RED "Error al crear directorio en proceso hijo: %s\n" RESET, stringDirectorioProceso);
                bumount();
                exit(FALLO);
            }

            //Crear el fichero prueba.dat dentro del directorio anterior.
            sprintf(stringFicheroPrueba, "%sprueba.dat", stringDirectorioProceso);
            if (mi_creat(stringFicheroPrueba, 6) < 0) {
                fprintf(stderr, RED "Error al crear fichero en proceso hijo: %s\n" RESET, stringFicheroPrueba);
                bumount();
                exit(FALLO);
            }

            //Inicializar la semilla de números aleatorios
            srand(time(NULL) + getpid());
            for (int nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++) {
                //Inicializar registro
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro = rand() % REGMAX;
                //Escribir el registro con mi_write()
                if (mi_write(stringFicheroPrueba, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO)) < 0) {
                    bumount();
                    exit(FALLO);
                }
#if DEBUG12a
                static char colores[10][10] = {
                    "\033[31m", // Rojo
                    "\033[32m", // Verde
                    "\033[33m", // Amarillo
                    "\033[34m", // Azul
                    "\033[35m", // Magenta
                    "\033[36m", // Cian
                    "\033[37m", // Blanco
                    "\033[91m", // Rojo intenso
                    "\033[92m", // Verde intenso
                    "\033[93m", // Amarillo intenso
                };
                int indice_color = getpid() % 10;
                fprintf(stderr, "%s[simulación.c → Escritura %i en %s]\n" RESET, colores[indice_color], nescritura, stringFicheroPrueba);
#endif
                //Esperar 0,05 seg para hacer la siguiente escritura.
                usleep(50000);
            }
            //Desmontar el dispositivo. hijo
            bumount();
#if DEBUG12b
            printf("[Proceso %d: Completadas %d escrituras en %s]\n", proceso, NUMESCRITURAS, stringFicheroPrueba);
#endif
            exit(EXITO); //Necesario para que se emita la señal SIGCHLD
        }
        //Esperamos para lanzar el siguiente proceso 0,15 segundos
        usleep(150000);
    }
    //Permitir que el padre espere por todos los hijos
    while (acabados < NUMPROCESOS) {
        pause();
    }

    //Desmontar dispositivo padre
    bumount();
    exit(EXITO);
}

// Función enterrador.
void reaper() {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}