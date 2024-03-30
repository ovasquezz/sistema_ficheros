#include "ficheros.h"

int mi_write_f(unsigned int ninodo, const void* buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    unsigned int primerBL, ultimoBL, desp1, desp2, nbfisico;
    unsigned char buf_bloque[BLOCKSIZE];
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, RED, "No hay permisos de escritura/n" RESET);
    }
    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    //primer caso el buffer cabe en un solo bloque
    if (primerBL == ultimoBL) { //obtenemos el numero de bloque
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == -1) {
            return -1;
        }

    } else {
        // la operacion afecta a mas de un bloque
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (nbfisico == -1) {
            return -1;
        }
        if (bread(nbfisico, buf_bloque) == -1) {
            return -1;
        }
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp2);
        if (bwrite(nbfisico, buf_bloque) == -1) {
            return -1;
        }
        //bloques intermedios
        for (int i = primerBL + 1;i < ultimoBL;i++) {
            if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1) {
                return -1;
            }
        }
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (bread(nbfisico, buf_original) == -1) {
            return -1;
        }
        desp2 = (offset + nbytes - 1) % BLOCKSIZE; // desplazamiento para comprobar nbytes escritos despues del offset
        memcpy(buf_bloque, buf_original + (nbytes - (desp1 - 1)), desp2 + 1);
        if (bwrite(nbfisico, buf_bloque) == -1) {
            return -1;
        }
    }
    // actualizamos el inodo
    if (leer_inodo(ninodo, &inodo) == -1) {
        return -1;
    }
    if ((offset + nbytes) > inodo.tamEnBytesLog) {
        inodo.tamEnBytesLog = offset + nbytes;
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == -1) {
        return -1;
    }
    return nbytes; // numero de bytes escritos
}

int mi_read_f(unsigned int ninodo, void* buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int leidos, bl_fisico;
    if ((inodo.permisos & 4) != 4) {
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
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
    int reservar = 0;
    // bloque incial i bloque final coinciden
    if (primerBL = ultimoBL) {
        bl_fisico = traducir_bloque_inodo(ninodo, primerBL, reservar);
        if (bl_fisico == -1) {
            if (bread(bl_fisico, buf_bloque) == -1) {
                return -1;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);

        }
        leidos = nbytes;
    } else {
        // primera fase
        bl_fisico = traducir_bloque_inodo(ninodo, primerBL, reservar);
        if (bl_fisico != -1) {
            if (bread(bl_fisico, buf_bloque) == -1) {
                return -1;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        leidos = leidos - desp1;
        //segunda fase
        for (int i = primerBL + 1;i < ultimoBL;i++) {
            bl_fisico = traducir_bloque_inodo(ninodo, i, 0);
            if (bl_fisico == -1) {
                if (bread(bl_fisico, buf_bloque) == -1) {
                    return -1;
                }
                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            leidos = leidos + BLOCKSIZE;
        }
        //tercera fase
        bl_fisico = traducir_bloque_inodo(ninodo, ultimoBL, reservar);
        if (bl_fisico == -1) {
            if (bread(bl_fisico, buf_bloque) == -1) {
                return -1;
            }
            memcpy(buf_original + (BLOCKSIZE - desp1) + (ultimoBL - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
        }
        leidos = leidos + desp2 + 1;
    }
    if (leer_inodo(ninodo, &inodo) < 0) {
        return -1;
    }
    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == -1) {
        return -1;
    }
    return leidos;
}

int mi_stat_f(unsigned int ninodo, struct STAT* p_stat) {
    struct inodo inodo;
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
int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {
    struct inodo inodo;
    inodo.permisos = permisos;
    inodo.ctime = ctime(NULL);
    return 1;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    struct inodo inodo;
    unsigned int primerBL;
    int liberados;
    if (leer_inodo(ninodo, &inodo) == -1){
        perror("Error en mi_truncar_f, leer inodo\n");
        return -1;
    }
    if ((inodo.permisos & 2) != 2){
        perror("Error en mi_truncar_f, faltan permisos de escritura\n");
        return -1;
    }
    if (inodo.tamEnBytesLog <= nbytes){
        perror("Error en mi_truncar_f, EOF\n");
        return -1;
    }
    // Liberar el primer bloque lógico
    if (nbytes % BLOCKSIZE == 0) primerBL = nbytes / BLOCKSIZE;
    else primerBL = nbytes / (BLOCKSIZE + 1);
    liberados = liberar_bloques_inodo(primerBL, &inodo);
    inodo.mtime = ctime(NULL);
    inodo.ctime = ctime(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= liberados;
    if (escribir_inodo(ninodo, &inodo) == -1){
        perror("Error en mi_truncar_f, escribir_inodo\n");
        return -1;
    }
    return liberados;
}