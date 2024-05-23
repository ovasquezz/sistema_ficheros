/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int descriptor = 0;
static sem_t* mutex;
static unsigned int inside_sc = 0;

void mi_waitSem() {
    if (!inside_sc) {//inside_sc==0, no se ha hecho un wait
        waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) {
        signalSem(mutex);
    }
}


/**
 * Monta el dispositivo virtual
*/
int bmount(const char* camino) {
    umask(000);
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (!mutex) {// el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem();
        if (mutex == SEM_FAILED) {
            return FALLO;
        }
    }
    if (descriptor == FALLO) {
        perror("Error al abrir el archivo");
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
    deleteSem();
    if (exitFile == FALLO) {
        perror("Error al cerrar el fichero");
        return FALLO;
    } else {
        return EXITO;
    }
}


/**
 * Escribe 1 bloque del dispositivo virtual
*/
int bwrite(unsigned int nbloque, const void* buf) {
    off_t desplazamiento = nbloque * BLOCKSIZE;
    if (lseek(descriptor, desplazamiento, SEEK_SET) == FALLO) {
        perror("Error al realizar seek para escritura");
        return FALLO;
    }

    ssize_t bytesEscritos = write(descriptor, buf, BLOCKSIZE);
    if (bytesEscritos == FALLO) {
        perror("Error en escritura");
        return FALLO;
    } else {
        return bytesEscritos;
    }
}


/**
 * Lee 1 bloque del dispositivo virtual
*/
int bread(unsigned int nbloque, void* buf) {
    off_t desplazamiento = nbloque * BLOCKSIZE;
    if (lseek(descriptor, desplazamiento, SEEK_SET) == FALLO) {
        perror("Error al realizar seek para lectura ");
        return FALLO;
    }

    ssize_t bytesLeidos = read(descriptor, buf, BLOCKSIZE);
    if (bytesLeidos == FALLO) {
        perror("Error en lectura");
        return FALLO;
    } else {
        return bytesLeidos;
    }

}