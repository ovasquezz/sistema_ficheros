#include "directorios.h"
#define DEBUG7 0
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

struct entrada* entrada;
static struct UltimaEntrada UltimaEntradaEscritura;
static struct UltimaEntrada UltimaEntradaLectura;

/**
 * Funcion para obtener el camino del directorio o fichero
*/
int extraer_camino(const char* camino, char* inicial, char* final, char* tipo) {
    const char* aux;

    if (camino[0] != '/') {
        return FALLO;
    }

    camino++;
    aux = strchr(camino, '/');

    if (aux != NULL) {
        strcpy(final, aux);
        strncpy(inicial, camino, aux - camino);
        inicial[aux - camino] = 0;
        *tipo = 'd';

    } else {
        strcpy(inicial, camino);
        strcpy(final, "");
        *tipo = 'f';
    }

    return EXIT_SUCCESS;
}

/**
 * Funcion para buscar una entrada entre las entradas del inodo correspondiente a su directorio padre
*/
int buscar_entrada(const char* camino_parcial, unsigned int* p_inodo_dir, unsigned int* p_inodo, unsigned int* p_entrada, char reservar, unsigned char permisos) {
    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo;
    int num_entrada_inodo;

    // Leer SB
    if (bread(posSB, &SB) < 0) {
        fprintf(stderr, "Error en la lectura del superbloque\n");
        return FALLO;
    }

    // Si es raiz
    if (!strcmp(camino_parcial, "/")) {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }

    //Obtenemos camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1) {
        return ERROR_CAMINO_INCORRECTO;
    }
#if DEBUG7
    printf("[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
#endif

    // Buscar entrada con nombre inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4) {
        return ERROR_PERMISO_LECTURA;
    }

    // Iniciar buffer con 0s
    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    // Calcular cantidad de entradas
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;

    // Leemos de entrada en entrada
    if (cant_entradas_inodo > 0) {
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
            perror("Error en mi_read_f en buscar_entrada()");
            return FALLO;
        }

        while (num_entrada_inodo < cant_entradas_inodo && strcmp(inicial, entrada.nombre) != 0) {
            num_entrada_inodo++;
            memset(entrada.nombre, 0, sizeof(entrada.nombre));
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
                perror("Error en mi_read_f en buscar_entrada()");
                return FALLO;
            }
        }
    }

    if (strcmp(inicial, entrada.nombre) != 0 && num_entrada_inodo == cant_entradas_inodo) {
        // la entrada no existe
        switch (reservar) {
        case 0:
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1:
            if (inodo_dir.tipo == 'f') {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            if ((inodo_dir.permisos & 2) != 2) {
                return ERROR_PERMISO_ESCRITURA;
            } else {
                strcpy(entrada.nombre, inicial);
                if (tipo == 'd') {
                    if (strcmp(final, "/") == 0) {
                        entrada.ninodo = reservar_inodo('d', permisos);
#if DEBUG7
                        fprintf(stderr, "[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif
                    } else {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                } else {
                    entrada.ninodo = reservar_inodo('f', permisos);
#if DEBUG7
                    fprintf(stderr, "[buscar()->reservado inodo: %d tipo %c con permisos %d para '%s']\n", entrada.ninodo, tipo, permisos, entrada.nombre);
#endif
                }
#if DEBUG7
                fprintf(stderr, "[buscar_entrada()->creada entrada: %s, %d] \n", inicial, entrada.ninodo);
#endif
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(entrada), sizeof(entrada)) == -1) {
                    if (entrada.ninodo != -1) {
                        liberar_inodo(entrada.ninodo);
#if DEBUG7
                        fprintf(stderr, "[buscar_entrada()-> liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial);
#endif
                    }
                    return FALLO;
                }
            }
        }
    }
    if ((strcmp(final, "/")) == 0 || strcmp(final, "") == 0) {
        if (num_entrada_inodo < cant_entradas_inodo && reservar == 1) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    } else {
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return EXITO;
}

/**
 * Funcion auxiliar para obtener el error especifico de la funcion buscar_entrada()
*/
void mostrar_error_buscar_entrada(int error) {
    switch (error) {
    case -2: fprintf(stderr, RED "Error: Camino incorrecto.\n"); break;
    case -3: fprintf(stderr, RED "Error: Permiso denegado de lectura.\n"); break;
    case -4: fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n"); break;
    case -5: fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n"); break;
    case -6: fprintf(stderr, RED "Error: Permiso denegado de escritura.\n"); break;
    case -7: fprintf(stderr, RED "Error: El archivo ya existe.\n"); break;
    case -8: fprintf(stderr, RED "Error: No es un directorio.\n"); break;
    }
    fprintf(stderr, WHITE);
}

/**
 * Funcion para crear un fichero/directorio y su entrada de directorio
*/
int mi_creat(const char* camino, unsigned char permisos) {
    mi_waitSem();
    if (permisos > 7 || permisos < 0) {
        fprintf(stderr, RED "Error: modo inválido:<<%i>>\n", permisos);
        mi_signalSem();
        return FALLO;
    }
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);

    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    return EXITO;
}

/**
 * Funcion para poner el contenido del directorio en un buffer de memoria y devuelve el número de entradas.
 * Mejora añadida: flag para determinar si imprimir version extendida o simple
 * Mejora no añadida: tipo de archivo
*/
int mi_dir(const char* camino, char* buffer, char flag) {
    struct inodo inodo;
    unsigned int p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int cant_entradas_inodo;
    int error;
    char tmp[30];
    char tipo[2];
    char tamBytes[16];

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) {
        return error;
    }

    if (leer_inodo(p_inodo, &inodo) == -1) {
        return FALLO;
    }

    if (inodo.tipo != 'd') {
        return FALLO;
    }

    if ((inodo.permisos & 4) != 4) {
        return FALLO;
    }

    cant_entradas_inodo = inodo.tamEnBytesLog / sizeof(struct entrada);
    struct entrada buffer_entradas[cant_entradas_inodo];

    if (mi_read_f(p_inodo, &buffer_entradas, 0, sizeof(struct entrada) * cant_entradas_inodo) == -1) {
        return FALLO;
    }

    strcpy(buffer, "");

    for (int i = 0; i < cant_entradas_inodo; i++) {
        if (leer_inodo(buffer_entradas[i].ninodo, &inodo) == -1) {
            return FALLO;
        }

        if (flag) { //version extendida
            sprintf(tipo, "%c", inodo.tipo);
            strcat(buffer, tipo);
            strcat(buffer, "\t");

            if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
            if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
            if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");

            strcat(buffer, "\t");

            struct tm* tm; //ver info: struct tm
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);
            sprintf(tamBytes, "\t%d", inodo.tamEnBytesLog);
            strcat(buffer, tamBytes);
            strcat(buffer, "\t");

            if (inodo.tipo == 'd') {
                strcat(buffer, GREEN); // si es directorio: color verde
            } else {
                strcat(buffer, CYAN); // si es fichero: color cyan
            }

            strcat(buffer, buffer_entradas[i].nombre);
            strcat(buffer, RESET);
            strcat(buffer, "\n");
        } else { //version simple
            if (inodo.tipo == 'd') {
                strcat(buffer, GREEN); // si es directorio: color verde
            } else {
                strcat(buffer, CYAN); // si es fichero: color cyan
            }

            strcat(buffer, buffer_entradas[i].nombre);
            strcat(buffer, RESET);
            strcat(buffer, "\t");
        }
    }
    return cant_entradas_inodo;
}


/**
 * Funcion para cambiar los permisos de un fichero o directorio
*/
int mi_chmod(const char* camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if (error >= 0) {
        mi_chmod_f(p_inodo, permisos);
        return EXITO;
    } else {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
}


/**
 * Funcion que muestra la información acerca del inodo de un fichero o directorio
*/
int mi_stat(const char* camino, struct STAT* p_stat) {

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int reservar = 0;
    int error;


    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 4)) < 0) {
        mostrar_error_buscar_entrada(error);
    }
    // printf("Nº de inodo: %d\n", p_inodo);
    return mi_stat_f(p_inodo, p_stat);
}


/**
 * Funcion para escribir contenido en un fichero
*/
int mi_write(const char* camino, const void* buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo = 0;
    unsigned int ninodo = 0;
    unsigned int inicial = 0;
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0) {
        p_inodo = UltimaEntradaEscritura.p_inodo;
    } else {
        if (buscar_entrada(camino, &ninodo, &p_inodo, &inicial, 0, 4) == -1) {
            return FALLO;
        } else {
            strcpy(UltimaEntradaEscritura.camino, camino);
            UltimaEntradaEscritura.p_inodo = p_inodo;
        }
    }
    return mi_write_f(p_inodo, buf, offset, nbytes);
}


/**
 * Funcion para leer contenido en un fichero
*/
int mi_read(const char* camino, void* buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo = 0;
    unsigned int ninodo = 0;
    unsigned int inicial = 0;
    if (strcmp(UltimaEntradaLectura.camino, camino) == 0) {
        p_inodo = UltimaEntradaLectura.p_inodo;
    } else {
        if (buscar_entrada(camino, &ninodo, &p_inodo, &inicial, 0, 2) == -1) {
            return FALLO;
        } else {
            strcpy(UltimaEntradaLectura.camino, camino);
            UltimaEntradaLectura.p_inodo = p_inodo;
        }
    }
    return mi_read_f(p_inodo, buf, offset, nbytes);
}


/**
 * Funcion que crea un enlace a un fichero
*/
int mi_link(const char* camino1, const char* camino2) {
    mi_waitSem();
    unsigned int p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_inodo_source;
    struct inodo inodo;
    int error = buscar_entrada(camino1, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error >= 0) {
        if (leer_inodo(p_inodo, &inodo) < 0) {
            mi_signalSem();
            perror("Error al leer_inodo en mi_link");
            return FALLO;
        }
        if (inodo.tipo == 'f' && inodo.permisos & 4) {
            p_inodo_source = p_inodo;
            p_inodo = 0;
            p_inodo_dir = 0;
            p_entrada = 0;
            error = buscar_entrada(camino2, &p_inodo_dir, &p_inodo, &p_entrada, 1, 6);
            if (error >= 0) {
                struct entrada entrada;

                if (mi_read_f(p_inodo_dir, &entrada, p_entrada * (sizeof(struct entrada)), sizeof(struct entrada)) < 0) {
                    mi_signalSem();
                    perror("Error en mi_read_f en mi_link");
                    return FALLO;
                }

                entrada.ninodo = p_inodo_source;

                if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
                    mi_signalSem();
                    perror("Error en mi_write_f en mi_link");
                    return FALLO;
                }

                liberar_inodo(p_inodo);
                inodo.nlinks++;
                inodo.ctime = time(NULL);
                mi_signalSem();
                return escribir_inodo(p_inodo_source, &inodo);
            } else {
                mostrar_error_buscar_entrada(error);
                mi_signalSem();
                return FALLO;
            }
        }
        mi_signalSem();
        return EXITO;
    } else {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }
}


/**
 * Funcion que deshace un enlace a un fichero
*/
int mi_unlink(const char* camino) {
    mi_waitSem();
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0, cant_entradas_inodo;
    struct inodo inodo;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error >= 0) {
        leer_inodo(p_inodo, &inodo);
        if (inodo.tamEnBytesLog > 0 && inodo.tipo == 'd') {
            fprintf(stderr, RED "El directorio %s no está vacío, no se puede borrar" RESET, camino);
            mi_signalSem();
            return FALLO;
        }
        leer_inodo(p_inodo_dir, &inodo);
        cant_entradas_inodo = inodo.tamEnBytesLog / sizeof(struct entrada);
        if (p_entrada != cant_entradas_inodo - 1) {
            struct entrada borrada, aux;
            int size = sizeof(struct entrada);
            int n_entrada = size * p_entrada;
            // leemos la entrada a borrar en memoria
            mi_read_f(p_inodo_dir, &borrada, n_entrada, size);
            // Leemos la ultima entrada y la colocamos en la psoicion de la entrada a eliminar
            mi_read_f(p_inodo_dir, &aux, (cant_entradas_inodo - 1) * size, size);
            mi_write_f(p_inodo_dir, &aux, n_entrada, size);
        }

        mi_truncar_f(p_inodo_dir, inodo.tamEnBytesLog - sizeof(struct entrada));
        leer_inodo(p_inodo, &inodo);
        inodo.nlinks = inodo.nlinks - 1;
        if (inodo.nlinks > 0) {
            inodo.ctime = time(NULL);
            escribir_inodo(p_inodo, &inodo);
        } else
            liberar_inodo(p_inodo);
        mi_signalSem();
        return EXITO;
    } else {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return EXITO;
    }
}
