#include "ficheros_basico.h"

int tamMB(unsigned int nbloques) {
    // comprobamos si hay un resto en la división
    if ((nbloques / 8) % BLOCKSIZE > 0) {
        // si hay resto necesitaremos un bloque mas
        return (nbloques / 8 / BLOCKSIZE) + 1;
    }
    // si no devolvemos ka operación
    return nbloques / 8 / BLOCKSIZE;
}

int tamAI(unsigned int ninodos) {
    // comprobamos si hay un resto en la división
    if (ninodos % (BLOCKSIZE / INODOSIZE) > 0) {
        // Si hay resto necesitaremos 1 bloque mas
        return ninodos / (BLOCKSIZE / INODOSIZE) + 1;
    }
    // Si no devolvemos el mismo numero
    return ninodos / (BLOCKSIZE / INODOSIZE);
}

int initSB(unsigned int nbloques, unsigned int ninodos) {
    struct superbloque SB;
    // Completamos la información de las variables de ficheros_basico.h
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;
    // Inicializamos el Super bloque
    if (bwrite(posSB, &SB) < 0) {
        fprintf(stderr, "Error escritura Super bloque\n");
        return FALLO;
    }
    return EXITO;
}

int initMB() {
    struct superbloque SB;

    if (bread(posSB, &SB) < 0) {
        perror("Error lectura Super Bloque\n");
        return FALLO;
    }

    // TODO




}

int initAI() {

    struct superbloque SB;
    if (bread(posSB, &SB) < 0) {
        perror("Error lectura Super Bloque\n");
        return FALLO;
    }

    int contInodos = SB.posPrimerInodoLibre + 1;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        int over = 0;
        for (int j = 0; (j < (BLOCKSIZE / INODOSIZE)) && (!over); j++) {
            inodos[j].tipo = 'l';
            if (contInodos < SB.totInodos) {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            } else {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                over = 1;
            }
        }
        bwrite(i, inodos);
    }
    return EXITO;
}


