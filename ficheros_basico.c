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
    bread(posSB, &SB);
    int used_blocks = SB.posPrimerBloqueMB;
    int metadata = tamMB(SB.totBloques) + tamAI(SB.totInodos) + 1;
    SB.cantBloquesLibres = SB.cantBloquesLibres - metadata;
    if ((metadata / 8 / BLOCKSIZE) > 1) {
        unsigned char* libre = malloc(BLOCKSIZE);
        for (; metadata > BLOCKSIZE; used_blocks++) {
            memset(libre, 255, BLOCKSIZE);
            bwrite(1 + used_blocks, libre);
            // Miramos si hace falta un bloque completo más para guardar los metadata.
            metadata = metadata / BLOCKSIZE;
        }
    }
    int bufferSize = metadata / 8;
    unsigned char bufferMB[BLOCKSIZE];
    memset(bufferMB, 0, BLOCKSIZE);
    for (int i = 0; i < bufferSize; i++) {
        bufferMB[i] = 255;
    }
    int datos_restantes = metadata % 8;
    unsigned char resto = 0;
    for (int i = 0; i < datos_restantes; i++) {
        resto += 1 << (7 - i);
    }
    bufferMB[bufferSize] = resto;
    bwrite(used_blocks, bufferMB);
    if (bwrite(posSB, &SB) < 0) {
        fprintf(stderr, "Error en la escritura del MB\n");
        return FALLO;
    }
    return EXITO;
}

int initAI() {
    struct superbloque SB;
    bread(posSB, &SB);
    int bufferSize = BLOCKSIZE / INODOSIZE;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int inodoCount = SB.posPrimerInodoLibre + 1;
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        bread(i, inodos);
        for (int j = 0; j < bufferSize; j++) {
            inodos[j].tipo = 'l';
            if (inodoCount < SB.totInodos) {
                inodos[j].punterosDirectos[0] = inodoCount;
                inodoCount++;
            } else {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                j = bufferSize;
            }
        }
        if (bwrite(i, inodos) < 0) {
            fprintf(stderr, "Error al iniciar el AI\n");
        };
    }
    return bwrite(posSB, &SB);
}

int escribir_bit(unsigned int nbloque, unsigned int bit) {
    struct superbloque SB;
    bread(posSB, &SB);
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int numbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + numbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) < 0) {
        fprintf(stderr, "Error en la lectura del bloque de escribit_bit\n");
        return FALLO;
    }
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mask = 128;
    mask >>= posbit;
    if (bit == 1) {
        bufferMB[posbyte] |= mask;
    } else {
        bufferMB[posbyte] &= ~mask;
    }
    if (bwrite(nbloqueabs, bufferMB) < 0) {
        fprintf(stderr, "Error en la escritura del bit en el bloque\n");
        return FALLO;
    }
    return 0;
}

char leer_bit(unsigned int nbloque) {
    struct superbloque SB;
    if (bread(posSB, &SB) < 0) {
        fprintf(stderr, "Error en la lectura del superbloque al leer_bit\n");
        return FALLO;
    }
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int numbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + numbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) < 0) {
        fprintf(stderr, "Error al leer el bloque en leer_bit()\n");
        return FALLO;
    }
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mask = 128;
    mask >>= posbit;
    mask &= bufferMB[posbyte];
    mask >>= (7 - posbit);
#if DEBUGN1
    fprintf(stderr, GRIS_T "[leer_bit(%i)]→posbyte:%i,posbit=%i;numbloqueMB:%i,nbloqueabs:%i\n", nbloque, posbyte, posbit, numbloqueMB, nbloqueabs);
    fprintf(stderr, RESET);
#endif
    return mask;
}

int reservar_bloque() {
    struct superbloque SB;
    if (bread(posSB, &SB) < 0) {
        fprintf(stderr, "Error en la lectura del SB en reservar_bloque\n");
        return FALLO;
    };
    if (SB.cantBloquesLibres > 0) {
        unsigned char bufferMB[BLOCKSIZE], bufferaux[BLOCKSIZE];
        memset(bufferaux, 255, BLOCKSIZE);
        memset(bufferMB, 0, BLOCKSIZE);
        int numbloqueMB = SB.posPrimerBloqueMB;
        bread(numbloqueMB, bufferMB);
        for (; numbloqueMB <= SB.posUltimoBloqueMB && memcmp(bufferMB, bufferaux, BLOCKSIZE) == 0;) {
            numbloqueMB++;
            bread(numbloqueMB, bufferMB);
        }
        int posbyte = 0;
        while (bufferMB[posbyte] == 255) {
            posbyte++;
        }
        unsigned char mask = 128;
        int posbit = 0;
        while (bufferMB[posbyte] & mask) {
            bufferMB[posbyte] <<= 1;
            posbit++;
        }
        int nbloque = ((numbloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
        escribir_bit(nbloque, 1);
        SB.cantBloquesLibres = SB.cantBloquesLibres - 1;

        memset(bufferaux, 0, BLOCKSIZE);
        if (bwrite(SB.posPrimerBloqueDatos + nbloque - 1, bufferaux) < 0) {
            fprintf(stderr, "Error al escrbir el bloque de reserva en reservar_bloque\n");
            return FALLO;
        }
        if (bwrite(posSB, &SB) < 0) {
            fprintf(stderr, "Error en la escritura del SB en reservar bloque\n");
            return FALLO;
        }
        return nbloque;
    }
    return FALLO;
}

int liberar_bloque(unsigned int nbloque) {
    escribir_bit(nbloque, 0);
    struct superbloque SB;
    if (bread(posSB, &SB) < 0) {
        fprintf(stderr, "Error en la lectura del SB en liberar_bloque\n");
        return FALLO;
    }
    unsigned char vacio[BLOCKSIZE];
    memset(vacio, 0, BLOCKSIZE);
    bwrite(nbloque, vacio);
    SB.cantBloquesLibres = SB.cantBloquesLibres + 1;
    if (bwrite(posSB, &SB) < 0) {
        fprintf(stderr, "Error en la escritura del SB en liberar_bloque\n");
    }
    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo){
    struct superbloque SB;
	bread(posSB, &SB);
	int nbloqueAI = (ninodo*INODOSIZE)/BLOCKSIZE;
    int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
	struct inodo inodos[BLOCKSIZE/INODOSIZE];
	bread(nbloqueabs, inodos);
	inodos[ninodo % (BLOCKSIZE/INODOSIZE)] = *inodo;
	return bwrite(nbloqueabs, inodos);
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    struct superbloque SB;
	bread(posSB, &SB);
	int nbloqueAI = (ninodo*INODOSIZE)/BLOCKSIZE;
    int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
	struct inodo inodos[BLOCKSIZE/INODOSIZE];
	if(bread(nbloqueabs, &inodos)){
        *inodo = inodos[ninodo % (BLOCKSIZE/INODOSIZE)];
        return EXITO;
    } else {
        return FALLO;
    }
}

int reservar_inodo(unsigned char tipo, unsigned char permisos){
    struct superbloque SB;
    struct inodo inodo;
	bread(posSB, &SB);
	int posInodoReservado = SB.posPrimerInodoLibre;
	if(leer_inodo(posInodoReservado, &inodo)){
        perror("resrvar_inodo: Error al leer inodo\n");
        return FALLO
    }
	unsigned int siguientelibre = inodo.punterosDirectos[0];
	inodo.tipo = tipo;
	inodo.permisos = permisos;
	inodo.nlinks = 1;
	inodo.tamEnBytesLog = 0;
	inodo.atime = time(NULL);
	inodo.mtime = time(NULL);
	inodo.ctime = time(NULL);
	inodo.numBloquesOcupados = 0;
	memset(inodo.punterosDirectos, 0, sizeof(inodo.punterosDirectos));
	memset(inodo.punterosIndirectos, 0, sizeof(inodo.punterosIndirectos));
	escribir_inodo(&inodo, SB.posPrimerInodoLibre); // Escribimos el inodo reservado
	// Actualizar la lista enlazada de inodos libres
	SB.cantInodosLibres--;
	SB.posPrimerInodoLibre = siguientelibre;
	bwrite(posSB, &SB);
	return posInodoReservado;
}