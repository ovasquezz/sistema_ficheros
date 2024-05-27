#include "ficheros.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/


/**
 * Escribe el contenido procedente de un buffer de memoria en un fichero/directorio,
 * le indicamos la posición de escritura inicial en bytes lógicos, offset, con respecto al inodo, y el número de bytes, nbytes, que hay que escribir.
*/
int mi_write_f(unsigned int ninodo, const void* buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    unsigned int primerBL, ultimoBL, desp1, desp2, nbfisico;
    unsigned char buf_bloque[BLOCKSIZE];
    if (leer_inodo(ninodo, &inodo) == -1) {
        perror("Error leer inodo en mi_write_f()");
        return FALLO;
    }

    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, RED "No hay permisos de escritura\n" RESET);
        return FALLO;
    }
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //Caso 1 solo cabe un bloque
    if (primerBL == ultimoBL) { //obtenemos el numero de bloque
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, &inodo, primerBL, 1);
        mi_signalSem();
        if (nbfisico == -1) {
            return FALLO;
        }
        if (bread(nbfisico, &buf_bloque) == -1) {
            return FALLO;
        }
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == -1) {
            return FALLO;
        }

    } else {
        //La ooperación afecta a varios bloques
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, &inodo, primerBL, 1);
        mi_signalSem();
        if (nbfisico == -1) {
            return FALLO;
        }
        if (bread(nbfisico, buf_bloque) == -1) {
            return FALLO;
        }
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        if (bwrite(nbfisico, buf_bloque) == -1) {
            return FALLO;
        }
        //Bloques intermedios
        for (int bl = primerBL + 1;bl < ultimoBL;bl++) {
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, &inodo, bl, 1);
            mi_signalSem();
            if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (bl - primerBL - 1) * BLOCKSIZE) == -1) {
                return FALLO;
            }
        }
        //Último bloque
        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, &inodo, ultimoBL, 1);
        mi_signalSem();
        if (nbfisico == -1) {
            return FALLO;
        }
        if (bread(nbfisico, buf_bloque) == -1) {
            return FALLO;
        }
        desp2 = (offset + nbytes - 1) % BLOCKSIZE; //Desplazamiento para comprobar nbytes escritos despues del offset
        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);
        if (bwrite(nbfisico, buf_bloque) == -1) {
            return FALLO;
        }
    }

    //Guardar inodo
    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) == -1) {
        mi_signalSem();
        return FALLO;
    }
    if ((offset + nbytes) > inodo.tamEnBytesLog) {
        inodo.tamEnBytesLog = offset + nbytes;
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == -1) {
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    return nbytes;
}


/**
 * Lee información de un fichero/directorio y la almacena en un buffer de memoria,
 * le indicamos la posición de lectura inicial offset con respecto al inodo (en bytes) y el número de bytes nbytes que hay que leer.
*/
int mi_read_f(unsigned int ninodo, void* buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int leidos, bl_fisico;

    if (leer_inodo(ninodo, &inodo) == -1) {
        perror("Error leer_inodo en mi_read_f");
        return FALLO;
    }
    if ((inodo.permisos & 4) != 4) {
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
        return FALLO;
    }
    if (offset >= inodo.tamEnBytesLog) {
        leidos = 0;
        return leidos;
    }
    if ((offset + nbytes) >= inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //Primer y último bloque coinciden
    if (primerBL == ultimoBL) {
        bl_fisico = traducir_bloque_inodo(ninodo, &inodo, primerBL, 0);
        if (bl_fisico != -1) {
            if (bread(bl_fisico, buf_bloque) == -1) {
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);

        }
        leidos = nbytes;
    } else {
        //Primer bloque lógico
        bl_fisico = traducir_bloque_inodo(ninodo, &inodo, primerBL, 0);
        if (bl_fisico != -1) {
            if (bread(bl_fisico, buf_bloque) == -1) {
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        leidos = BLOCKSIZE - desp1;

        //Segundo bloque lógico
        for (int bl = primerBL + 1;bl < ultimoBL;bl++) {
            bl_fisico = traducir_bloque_inodo(ninodo, &inodo, bl, 0);
            if (bl_fisico != -1) {
                if (bread(bl_fisico, buf_bloque) == -1) {
                    return FALLO;
                }
                memcpy(buf_original + (BLOCKSIZE - desp1) + (bl - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            leidos = leidos + BLOCKSIZE;
        }

        //Tercer bloque lógico
        bl_fisico = traducir_bloque_inodo(ninodo, &inodo, ultimoBL, 0);
        if (bl_fisico != -1) {
            if (bread(bl_fisico, buf_bloque) == -1) {
                return FALLO;
            }
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
        }
        leidos = leidos + desp2 + 1;
    }

    //Guardar inodo
    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) < 0) {
        mi_signalSem();
        return FALLO;
    }

    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == -1) {
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    return leidos;
}


/**
 * Devuelve la metainformación de un fichero/directorio
*/
int mi_stat_f(unsigned int ninodo, struct STAT* p_stat) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == -1) {
        perror("Error al leer inodo en mi_stat_f()");
        return FALLO;
    }
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    return 1;

}


/**
 * Cambia los permisos de un fichero/directorio
*/
int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {
    mi_waitSem();
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == -1) {
        perror("Error en mi_chmod_f al leer inodo");
        mi_signalSem();
        return FALLO;
    }
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == -1) {
        perror("Error en mi_chmod_f al escribir inodo");
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    return 1;
}


/**
 * Trunca un fichero/directorio a los bytes indicados como nbytes, liberando los bloques necesarios.
*/
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {
    struct inodo inodo;
    unsigned int primerBL;
    int liberados;

    if (leer_inodo(ninodo, &inodo) == -1) {
        perror("Error al leer inodo en mi_truncar_f\n");
        return FALLO;
    }
    if ((inodo.permisos & 2) != 2) {
        perror("Error, faltan permisos de escritura en mi_truncar_f\n");
        return FALLO;
    }
    if (inodo.tamEnBytesLog <= nbytes) {
        perror("EOF en mi_truncar_f\n");
        return FALLO;
    }

    // Calcular primerBL
    if (nbytes % BLOCKSIZE == 0) {
        primerBL = nbytes / BLOCKSIZE;
    } else {
        primerBL = nbytes / BLOCKSIZE + 1;
    }

    liberados = liberar_bloques_inodo(primerBL, &inodo);
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= liberados;

    if (escribir_inodo(ninodo, &inodo) == -1) {
        perror("Error al escribir inodo en mi_truncar_f\n");
        return FALLO;
    }
    return liberados;
}
