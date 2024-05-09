#include "directorios.h"

/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

struct entrada* entrada;
int extraer_camino(const char* camino, char* inicial, char* final, char* tipo) {
    if (camino[0] != '/') { // Si el camino no comienza con /
        return FALLO;
    }

    // Localizar la primera / despues de la inicial.
    char* aux = strchr((camino + 1), '/');
    strcpy(tipo, "f");

    //Si se ha encotrado el caracter '/'
    if (aux) {
        strncpy(inicial, (camino + 1), (strlen(camino) - strlen(aux) - 1)); // camino - aux
        strcpy(final, aux); // aux

        if (final[0] == '/') { // Si es directorio
            strcpy(tipo, "d");
        }
    } else { //Si no se ha encotrado
        strcpy(inicial, (camino + 1));
        strcpy(final, "");
    }

    return EXITO;
}

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
        return 0;
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
            return ERROR_PERMISO_LECTURA;
        }

        while (num_entrada_inodo < cant_entradas_inodo && strcmp(inicial, entrada.nombre) != 0) {
            num_entrada_inodo++;
            memset(entrada.nombre, 0, sizeof(entrada.nombre));
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
                return ERROR_PERMISO_LECTURA;
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
                if (mi_write_f(*p_inodo_dir, entrada.nombre, num_entrada_inodo * sizeof(entrada), sizeof(entrada)) == -1) {
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
        return 0;
    } else {
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return 0;
}

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

int mi_creat(const char* camino, unsigned char permisos) {
    if (permisos > 7 || permisos < 0) {
        fprintf(stderr, RED "Error: modo inválido:<<%i>>\n", permisos);
        return FALLO;
    }
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int op_incorrecta = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);

    if (op_incorrecta < 0) {
        mostrar_error_buscar_entrada(op_incorrecta);

        return FALLO;
    }
    return EXITO;
}

int mi_dir(const char* camino, char* buffer) {
    struct entrada buf_entradas[BLOCKSIZE / sizeof(struct entrada)];

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    char inicial[TAMFILA];
    struct inodo inodo;
    int cant_entradas_inodo, ind_buffer;
    int op_incorrecta = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (op_incorrecta >= 0) {
        int num_entrada_inodo = 0;
        leer_inodo(p_inodo, &inodo);
        if (inodo.tipo == 'd') {
            cant_entradas_inodo = inodo.tamEnBytesLog / sizeof(struct entrada);
            if (cant_entradas_inodo > 0) {
                while (num_entrada_inodo < cant_entradas_inodo) {
                    memset(buf_entradas, 0, BLOCKSIZE);
                    mi_read_f(p_inodo, &buf_entradas, num_entrada_inodo * sizeof(struct entrada), BLOCKSIZE);
                    for (ind_buffer = 0; ind_buffer < BLOCKSIZE / sizeof(struct entrada) && (num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, buf_entradas[ind_buffer].nombre) != 0); ind_buffer++) {
                        leer_inodo(buf_entradas[ind_buffer].ninodo, &inodo);
                        char c[2];
                        c[0] = inodo.tipo;
                        strcat(buffer, c);
                        strcat(buffer, "\t");
                        if (inodo.permisos & 4)
                            strcat(buffer, "r");
                        else
                            strcat(buffer, "-");
                        if (inodo.permisos & 2)
                            strcat(buffer, "w");
                        else
                            strcat(buffer, "-");
                        if (inodo.permisos & 1)
                            strcat(buffer, "x");
                        else
                            strcat(buffer, "-");
                        strcat(buffer, "\t");
                        struct tm* tm;
                        char tmp[TAMFILA];
                        memset(tmp, 0, TAMFILA);
                        tm = localtime(&inodo.mtime);
                        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                        strcat(buffer, tmp);
                        strcat(buffer, "\t");
                        char info[20];
                        memset(info, 0, 20);
                        sprintf(info, "%d", inodo.tamEnBytesLog);
                        strcat(buffer, info);
                        strcat(buffer, "\t");
                        char final_aux[TAMFILA];
                        memset(final_aux, 0, TAMFILA);
                        if (inodo.tipo == 'd') {
                            do {
                                extraer_camino(buf_entradas[ind_buffer].nombre, inicial, final_aux, 0);
                            } while (strcmp(final_aux, "/") == 0);
                            strcat(buffer, buf_entradas[ind_buffer].nombre);
                        } else
                            strcat(buffer, buf_entradas[ind_buffer].nombre);
                        strcat(buffer, "\n");
                        num_entrada_inodo++;
                    }
                }
            }
        } else {
            struct entrada entrada;
            mi_read_f(p_inodo, &buf_entradas, num_entrada_inodo * sizeof(struct entrada), BLOCKSIZE);
            leer_inodo(entrada.ninodo, &inodo);
            strcat(buffer, "f");
            strcat(buffer, "\t");
            if (inodo.permisos & 4)
                strcat(buffer, "r");
            else
                strcat(buffer, "-");
            if (inodo.permisos & 2)
                strcat(buffer, "w");
            else
                strcat(buffer, "-");
            if (inodo.permisos & 1)
                strcat(buffer, "x");
            else
                strcat(buffer, "-");
            strcat(buffer, "\t");
            char tmp[TAMFILA];
            memset(tmp, 0, TAMFILA);
            struct tm* tm;
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);
            strcat(buffer, "\t");
            char info[20];
            memset(info, 0, 20);
            sprintf(info, "%d", inodo.tamEnBytesLog);
            strcat(buffer, info);
            strcat(buffer, "\t");
            strcat(buffer, entrada.nombre);
        }
    } else {
        mostrar_error_buscar_entrada(op_incorrecta);
        return FALLO;
    }
    return cant_entradas_inodo;
}


int mi_chmod(const char* camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int op_incorrecta = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (op_incorrecta >= 0) {
        mi_chmod_f(p_inodo, permisos);
        return EXITO;
    } else {
        mostrar_error_buscar_entrada(op_incorrecta);
        return FALLO;
    }
}

int mi_stat(const char* camino, struct STAT* p_stat) {

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int reservar = 0;
    int error;


    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
        mostrar_error_buscar_entrada(error);
    }

    mi_stat_f(p_inodo, p_stat);
    return p_inodo  ;


}