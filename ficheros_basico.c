#include "ficheros_basico.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

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
        fprintf(stderr, "Error en la escritura del MB\n");
        return FALLO;
    }
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
    if (bread(posSB, &SB) == -1) {
        perror("Error leer SB en escribit_bit()");
        return FALLO;
    }
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int numbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + numbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    memset(bufferMB, '\0', BLOCKSIZE);

    if (bread(nbloqueabs, &bufferMB) < 0) {
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
    if (bwrite(nbloqueabs, &bufferMB) < 0) {
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
    if (bread(nbloqueabs, &bufferMB) < 0) {
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
    struct superbloque SB;
    escribir_bit(nbloque, 0);
    if (bread(posSB, &SB) < 0) {
        fprintf(stderr, "Error en la lectura del SB en liberar_bloque()\n");
        return FALLO;
    }
    SB.cantBloquesLibres = SB.cantBloquesLibres + 1;
    if (bwrite(posSB, &SB) < 0) {
        fprintf(stderr, "Error en la escritura del SB en liberar_bloque()\n");
    }
    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo* inodo) {
    struct superbloque SB;
    if (bread(posSB, &SB) < 0) {
        fprintf(stderr, "Error en la lectura del SB en escribir_inodo()\n");
        return FALLO;
    }

    int nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    if (bread(nbloqueabs, inodos) == -1) {
        return FALLO;
    }
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = *inodo;
    if (bwrite(nbloqueabs, inodos) == -1) {
        return FALLO;
    }
    return EXITO;
}

int leer_inodo(unsigned int ninodo, struct inodo* inodo) {
    struct superbloque SB;
    if (bread(posSB, &SB) == -1) {
        fprintf(stderr, "Error en la lectura del SB en leer_inodo()\n");
        return FALLO;
    }
    int nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    if (bread(nbloqueabs, inodos) != -1) {
        *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];
        return EXITO;
    } else {
        return FALLO;
    }
}

int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    struct superbloque SB;
    struct inodo inodo;
    if (bread(posSB, &SB) == -1) {
        perror("Error en lectura superbloque en reservar_inodo()");
        return FALLO;
    }
    if (SB.cantInodosLibres == 0) {
        perror("No hay inodos libres");
        return FALLO;
    }
    int posInodoReservado = SB.posPrimerInodoLibre;

    if (leer_inodo(posInodoReservado, &inodo)) {
        perror("resrvar_inodo: Error al leer inodo\n");
        return FALLO;
    }

    inodo.tipo = tipo;
    inodo.permisos = permisos;
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.atime = time(NULL);
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.numBloquesOcupados = 0;
    SB.posPrimerInodoLibre = inodo.punterosDirectos[0];
    memset(inodo.punterosDirectos, 0, sizeof(inodo.punterosDirectos));
    memset(inodo.punterosIndirectos, 0, sizeof(inodo.punterosIndirectos));
    if (escribir_inodo(posInodoReservado, &inodo) == -1) {
        perror("Error en escribir_inodo() en reservar_inodo()");
        return FALLO;
    }
    // Actualizar la lista enlazada de inodos libres
    SB.cantInodosLibres--;
    if (bwrite(posSB, &SB) == -1) {
        perror("Error en bwrite() en reservar_inodo()");
        return FALLO;
    }
    return posInodoReservado;
}
int obtener_nRangoBL(struct inodo* inodo, unsigned int nblogico, unsigned int* ptr) {
    if (nblogico < DIRECTOS) {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0; //<12
    } else if (nblogico < INDIRECTOS0) {
        *ptr = inodo->punterosIndirectos[0];
        return 1; //268
    } else if (nblogico < INDIRECTOS1) {
        *ptr = inodo->punterosIndirectos[1];
        return 2; //65804
    } else if (nblogico < INDIRECTOS2) {
        *ptr = inodo->punterosIndirectos[2];
        return 3; // 16843020
    } else {
        *ptr = 0;
        perror("Bloque logico fuera de rango");
        return FALLO;
    }
}
int obtener_indice(unsigned int nblogico, int nivel_punteros) {
    if (nblogico < DIRECTOS) { //ej. nblogico=8
        return nblogico;
    } else if (nblogico < INDIRECTOS0) { //ej. nblogico=204
        return nblogico - DIRECTOS;
    } else if (nblogico < INDIRECTOS1) { // ej. nblogico=30004
        if (nivel_punteros == 2) {
            return(nblogico - INDIRECTOS0) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            return(nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    } else if (nblogico < INDIRECTOS2) { // ej nblogico=400004
        if (nivel_punteros == 3) {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        } else if (nivel_punteros == 2) {
            return (nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            return(nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS) % NPUNTEROS;
        }
    }
    return FALLO; // error
}
int traducir_bloque_inodo(unsigned int ninodo, struct inodo* inodo, unsigned int nblogico, unsigned char reservar) {
    //VAR
    unsigned int ptr, ptr_ant;
    int nRangoBL, nivel_punteros, indice, cambios;
    unsigned int buffer[NPUNTEROS];
    ptr = 0, ptr_ant = 0; cambios = 0;
    nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr); //0:D,1:I0,2:I1,3:I2
    nivel_punteros = nRangoBL; // el nivel_punteros mas alto es el que cuelga directamnete del inodo
    while (nivel_punteros > 0) { // iterar para cada nivel de puneteros indirectos
        if (ptr == 0) { //no cuelgan bloques de punteros
            if (reservar == 0) { //bloque inexistente
                return FALLO;
            } else { // reservar bloques de punteros y crear enlaces desde le inodo hasta el bloque de datos
                cambios = 1;
                ptr = reservar_bloque(); //de punteros
                inodo->numBloquesOcupados++;
                inodo->ctime = time(NULL); //fecha actual
                if (nivel_punteros == nRangoBL) { // el bloque cuelga directamente del inodo
                    inodo->punterosIndirectos[nRangoBL - 1] = ptr;
                } else { //el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;
                    if (bwrite(ptr_ant, buffer) == -1) {
                        return FALLO;
                    } //salvamos en el dispositivo el biffer de punteros modificado
                }
                memset(buffer, 0, BLOCKSIZE); //ponemos a 0 todos los punteros del buffer
            }
        } else {
            if (bread(ptr, buffer) == -1) {
                return FALLO;
            } //leemos del dispositivo el bloque de punteros ya existente
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr; //guardamos el puntero actual
        ptr = buffer[indice]; //y lo desplazamos al siguiente nivel
        nivel_punteros--;
    }// al salir de este bucle ya estamos al nivel de datos

    if (ptr == 0) { // no existe bloque de datos
        if (reservar == 0) { // error de lectura bloque inexistente
            return FALLO;
        } else {
            cambios = 1;
            ptr = reservar_bloque(); // de datos
            inodo->numBloquesOcupados++;
            inodo->ctime = time(NULL);
            if (nRangoBL == 0) { // si era un puntero directo
                inodo->punterosDirectos[nblogico] = ptr; // asignamos la dirección del bloque de datos en el inodo
            } else {
                buffer[indice] = ptr; // assignamos la direccion del bloque de datos en el buffer
                if (bwrite(ptr_ant, buffer) == -1) {
                    return FALLO;
                } // salvamos en el dispositivo el buffer de punteros modificado
            }
        }
    } //mi_write_f() se encargará de salvar los cambios del inodo en el disco
    if (cambios == 1) {
        if (escribir_inodo(ninodo, inodo) == -1) {
            return FALLO;
        };
    }
    return ptr; // numero de bloque fisico correspondiente al bloque de datos logico, nblogico
}

int liberar_inodo(unsigned int ninodo) {
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    int bloquesL = liberar_bloques_inodo(0, &inodo);
    printf("Liberados %d\n", bloquesL);
    printf("inodo.numBloquesOcupados %d\n", inodo.numBloquesOcupados);

    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;
    inodo.numBloquesOcupados -= bloquesL;
    struct superbloque SB;
    if (bread(posSB, &SB) == -1) {
        return FALLO;
    }
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    inodo.ctime = time(NULL);
    SB.posPrimerInodoLibre = ninodo;
    SB.cantInodosLibres = SB.cantInodosLibres + 1;
    if (bwrite(posSB, &SB) < 0) {
        perror("Error en la escritura del SB en liberar_bloque");
        return FALLO;
    }
    escribir_inodo(ninodo, &inodo);
    return ninodo;
}

int liberar_bloques_inodo(unsigned int primerBL, struct inodo* inodo) {
    unsigned int nivel_punteros, indice, nBL, ultimoBL, ptr = 0;
    int nRangoBL, liberados;
    unsigned int bloques_punteros[3][NPUNTEROS]; // array de bloques de punteros
    unsigned char bufAux_punteros[BLOCKSIZE]; // para llenar 0s y comparar
    int ptr_nivel[3], indices[3]; // punteros a bloques de cada nivel / indices de cada nivel
    int contador_breads = 0;  //para comprobar optimización eficiencia
    int contador_bwrites = 0; //para comprobar optimización eficiencia

    liberados = 0;
    if (inodo->tamEnBytesLog == 0) {
        fprintf(stderr, "Error en liberar_bloques_inodo(): el fichero está vacío\n");
        return liberados; // fichero vacio, liberados = 0
    }
    // obtenemos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0) ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    else {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

    memset(bufAux_punteros, 0, BLOCKSIZE);
    for (nBL = primerBL; nBL < ultimoBL; nBL++) { // recorrer bloques lógicos
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
        if (nRangoBL < 0) {
            fprintf(stderr, "Error en liberar_bloques_inodo nRangoBL<0: %d, %s\n", errno, strerror(errno));
            return FALLO;
        }
        nivel_punteros = nRangoBL; // el nivel más alto cuelga del inodo
        while (ptr > 0 && nivel_punteros > 0) { // cuelgan bloques de punteros
            indice = obtener_indice(nBL, nivel_punteros);
            if (indice == 0 || nBL == primerBL) bread(ptr, bloques_punteros[nivel_punteros - 1]);
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }
        if (ptr > 0) { // si existe bloque de datos
            liberar_bloque(ptr);
            liberados++;
            contador_breads++;
            contador_bwrites++;
            if (nRangoBL == 0) inodo->punterosDirectos[nBL] = 0; //es un puntero directo
            else {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL) {
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0) {
                        // no cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        liberados++;
                        contador_breads++;
                        contador_bwrites++;
                        //Incluir mejora 1 saltando los bloques que no sea necesario explorar
                        //al eliminar bloque de punteros

                        if (nivel_punteros == nRangoBL) inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        nivel_punteros++;
                    } else { // escribimos en el dispositivo el bloque de punteros modificado
                        bwrite(ptr, bloques_punteros[nivel_punteros - 1]);
                        contador_bwrites++;
                        nivel_punteros = nRangoBL + 1; // hemos de salir del bucle
                    }
                }
            }
        } else {
            //Incluir mejora 2 saltando los bloques que no sea necesario explorar al valer 0 un puntero

        }
    }
    #if DEBUG
        fprintf(stderr, "[liberar_bloques_inodo()→ total bloques liberados: %d, total_breads: %d, total_bwrites:%d]\n", liberados, contador_breads, contador_bwrites);
    #endif
    return liberados;
}