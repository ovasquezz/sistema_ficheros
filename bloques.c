/**
 * @author Otto Vásquez
 * @author Bernat Parera
 *
*/

#include "bloques.h"

static int descriptor = 0;


/**
 * Monta el dispositivo virtual
*/
int bmount(const char* camino) {
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor == FALLO){
        perror("Error al abrir el archivo\n");
        return FALLO;
    } else {
        return descriptor;
    }
}


/**
 * Desmonta el dispositivo virtual
*/
int bumount() {
    int exitFile = close(descriptor);
    if (exitFile == FALLO){
        perror("Error al cerrar el fichero\n");
        return FALLO;
    } else {
        return EXITO;
    }
}


/**
 * Escribe 1 bloque del dispositivo virtual
*/
int bwrite(unsigned int nbloque, const void *buf){
    off_t desplazamiento = nbloque * BLOCKSIZE;
    if(lseek(descriptor, desplazamiento, SEEK_SET) == FALLO){
        perror("Error al realizar seek para escritura\n");
        return FALLO;
    }

    ssize_t bytesEscritos =  write(descriptor, buf, BLOCKSIZE);
    if (bytesEscritos == FALLO){
        perror("Error en escritura\n");
        return FALLO;
    } else {
        return bytesEscritos;
    }
}


/**
 * Lee 1 bloque del dispositivo virtual
*/
int bread(unsigned int nbloque, void *buf){
    off_t desplazamiento = nbloque * BLOCKSIZE;
    if(lseek(descriptor, desplazamiento, SEEK_SET) == FALLO){
        perror("Error al realizar seek para lectura\n");
        return FALLO;
    }

    ssize_t bytesLeidos =  read(descriptor, buf, BLOCKSIZE);
    if (bytesLeidos == FALLO){
        perror("Error en lectura\n");
        return FALLO;
    } else {
        return bytesLeidos;
    }

}