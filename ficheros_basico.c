#include "ficheros_basico.h"
/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/
#define DEBUG 0

/**
 * Funcion que calcula el tamaño en bloques necesario para el mapa de bits.
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


/**
 * Funcion que calcula el tamaño en bloques del array de inodos.
*/
int tamAI(unsigned int ninodos) {
    // comprobamos si hay un resto en la división
    if (((ninodos * INODOSIZE) % BLOCKSIZE) > 0) {
        // Si hay resto necesitaremos 1 bloque mas
        return ninodos / (BLOCKSIZE / INODOSIZE) + 1;
    }
    // Si no devolvemos el mismo numero
    return ninodos / (BLOCKSIZE / INODOSIZE);
}


/**
 * Funcion que inicializa los datos del superbloque
*/
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


/**
 * Funcion que inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos.
*/
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


/**
 * Funcion que inicializa la lista de inodos libres.
*/
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

/**
 * Funcion que escribe el valor indicado por el parámetro en un determinado bit del MB que representa el bloque nbloque.
*/
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
    if (bwrite(nbloqueabs, bufferMB) < 0) {
        fprintf(stderr, "Error en la escritura del bit en el bloque\n");
        return FALLO;
    }
    return 0;
}


/**
 * Lee un determinado bit del MB y devuelve el valor del bit leído.
*/
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
#if DEBUG
    fprintf(stderr, GRAY"[leer_bit(%i)]→posbyte:%i,posbit=%i;numbloqueMB:%i,nbloqueabs:%i\n", nbloque, posbyte, posbit, numbloqueMB, nbloqueabs);
    fprintf(stderr, RESET);
#endif
    return mask;
}

/**
 * Encuentra el primer bloque libre, consultando el MB (primer bit a 0), lo ocupa
 * (poniendo el correspondiente bit a 1 con la ayuda de la función escribir_bit()) y devuelve su posición.
*/
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


/**
 * Libera un bloque determinado.
*/
int liberar_bloque(unsigned int nbloque) {
    struct superbloque SB;
    escribir_bit(nbloque, 0);
    if (bread(posSB, &SB) == -1) {
        fprintf(stderr, "Error en la lectura del SB en liberar_bloque()\n");
        return FALLO;
    }
    SB.cantBloquesLibres = SB.cantBloquesLibres + 1;
    if (bwrite(posSB, &SB) == -1) {
        fprintf(stderr, "Error en la escritura del SB en liberar_bloque()\n");
    }
    return nbloque;
}


/**
 * Escribe el contenido de una variable de tipo struct inodo, pasada por referencia, en un determinado inodo del array de inodos, inodos.
*/
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


/**
 * Lee un determinado inodo del array de inodos para volcarlo en una variable de tipo struct inodo pasada por referencia.
*/
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


/**
 * Encuentra el primer inodo libre, lo reserva, devuelve su número y actualiza la lista enlazada de inodos libres.
*/
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

    for (int i = 0; i < DIRECTOS; i++)
        inodo.punterosDirectos[i] = 0;
    for (int i = 0; i < 3; i++)
        inodo.punterosIndirectos[i] = 0;
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

/**
 * Funcion para obtener el rango de punteros
*/
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


/**
 * Funcion para la obtención de los índices de los bloques de punteros
*/
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


/**
 * Esta función se encarga de obtener el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado.
 * Enmascara la gestión de los diferentes rangos de punteros directos e indirectos del inodo, de manera que
 * funciones externas no tienen que preocuparse de cómo acceder a los bloques físicos apuntados desde el inodo
*/
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
#if DEBUG
                    fprintf(stderr, "[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %d (reservado BF %d para BL %d)]\n", nRangoBL - 1, inodo->punterosIndirectos[nRangoBL - 1], inodo->punterosIndirectos[nRangoBL - 1], nblogico);
#endif
                } else { //el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;
#if DEBUG
                    fprintf(stderr, "[traducir_bloque_inodo()→ punteros_nivel%d[%d] = %d (reservado BF %d para BL %d)]\n", nivel_punteros, indice, buffer[indice], buffer[indice], nblogico);
#endif
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
#if DEBUG
                fprintf(stderr, "[traducir_bloque_inodo()→ inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n", nblogico, inodo->punterosDirectos[nblogico], inodo->punterosDirectos[nblogico], nblogico);
#endif
            } else {
                buffer[indice] = ptr; // assignamos la direccion del bloque de datos en el buffer
#if DEBUG
                fprintf(stderr, "[traducir_bloque_inodo()→ punteros_nivel%d[%d] = %d (reservado BF %d para BL %d)]\n", nivel_punteros, indice, buffer[indice], buffer[indice], nblogico);
#endif
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


/**
 * Funcion para liberar un inodo, liberar boques de datos y bloques de índice
*/
int liberar_inodo(unsigned int ninodo) {
    struct inodo inodo;
    int bloquesL = 0;
    leer_inodo(ninodo, &inodo);
    if (inodo.tamEnBytesLog > 0) {
        bloquesL = liberar_bloques_inodo(0, &inodo);
    }
    // printf("Liberados %d\n", bloquesL);
    // printf("inodo.numBloquesOcupados %d\n", inodo.numBloquesOcupados);

    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;
    inodo.numBloquesOcupados -= bloquesL;
    if (inodo.numBloquesOcupados) // !=0
        fprintf(stderr, RED "Faltan por liberar %d bloques del inodo\n" RESET, inodo.numBloquesOcupados);
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




/**
 * Libera los bloques de datos e índices iterando desde el primer bloque lógico a liberar hasta el último
 * Version recursiva con breads y bwrites optimizados
*/
int liberar_bloques_inodo(unsigned int primerBL, struct inodo* inodo) {
    //var
    unsigned int nivel_punteros = 0, nBL = primerBL, ultimoBL, ptr = 0;
    int nRangoBL = 0, liberados = 0, eof = 0;
    unsigned int nBLAux = 0, nBreads = 0, nBwrites = 0;

    //obtenemos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog == 0) {
        fprintf(stderr, "Error en liberar_bloques_inodo(): el fichero está vacío\n");
        return liberados; // fichero vacio, liberados = 0
    }

    // obtenemos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0) {
        ultimoBL = (inodo->tamEnBytesLog / BLOCKSIZE) - 1;
    } else {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }
#if DEBUG
    printf("[liberar_bloques_inodo()→ primer BL %d, ultimo BL %d]\n", primerBL, ultimoBL);
#endif
    nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
    if (nRangoBL == 0) {
        liberados += liberar_directos(&nBL, ultimoBL, inodo, &eof);
    }

    while (!eof) {
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
        nivel_punteros = nRangoBL;
        liberados += liberar_indirectos_recursivo(&nBL, primerBL, ultimoBL, inodo, nRangoBL, nivel_punteros, &ptr, &eof, &nBLAux, &nBreads, &nBwrites);
    }
#if DEBUG
    printf("[liberar_bloques_inodo()→ total bloques liberados: %d, total breads: %d, total bwrites: %d\n", liberados, nBreads, nBwrites);
#endif
    return liberados;

}

/**
 * Función para liberar bloques directos
*/
int liberar_directos(unsigned int* nBL, unsigned int ultimoBL, struct inodo* inodo, int* eof) {
    //var
    int liberados = 0;

    for (unsigned int d = *nBL; (d < DIRECTOS && !*eof); d++) {
        if (inodo->punterosDirectos[*nBL] != 0) {
            liberar_bloque(inodo->punterosDirectos[*nBL]);
#if DEBUG
            printf("[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d)]\n", inodo->punterosDirectos[*nBL], *nBL);
#endif
            inodo->punterosDirectos[*nBL] = 0;
            liberados++;
        }
        *nBL = *nBL + 1;
        if (*nBL > ultimoBL) {
            *eof = 1; // Fin del archivo
        }
    }
    return liberados;
}

/**
 * Función para liberar bloques indirectos de manera recursiva
*/
int liberar_indirectos_recursivo(unsigned int* nBL, unsigned int primerBL, unsigned int ultimoBL, struct inodo* inodo, int nRangoBL, unsigned int nivel_punteros,
    unsigned int* ptr, int* eof, unsigned int* nBLAux, unsigned int* nBreads, unsigned int* nBwrites) {

    //var
    int liberados = 0, indice_inicial;
    unsigned int bloquePunteros[NPUNTEROS], bloquePunteros_Aux[NPUNTEROS], bufferCeros[NPUNTEROS];

    memset(bufferCeros, 0, BLOCKSIZE);

    if (*ptr) { //si cuelga un bloque de punteros
        indice_inicial = obtener_indice(*nBL, nivel_punteros);

        if (indice_inicial == 0 || *nBL == primerBL) {  //solo leemos bloque si no estaba cargado
            if (bread(*ptr, bloquePunteros) == -1) {
                return FALLO;
            }
            *nBreads = *nBreads + 1;
            // Guardamos copia del bloque para ver si hay cambios
            memcpy(bloquePunteros_Aux, bloquePunteros, BLOCKSIZE);
        }

        for (int i = indice_inicial; (i < NPUNTEROS && !*eof); i++) {
            // printf("bloquePunteros[i] %d\n", bloquePunteros[i]);
            if (bloquePunteros[i] != 0) {
                if (nivel_punteros == 1) {
                    liberar_bloque(bloquePunteros[i]);
#if DEBUG
                    printf("[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d)]\n", bloquePunteros[i], *nBL);
#endif
                    bloquePunteros[i] = 0;
                    liberados++;
                    *nBLAux = *nBL;
                    *nBL = *nBL + 1;
                } else {
                    liberados += liberar_indirectos_recursivo(nBL, primerBL, ultimoBL, inodo, nRangoBL, nivel_punteros - 1, &bloquePunteros[i], eof, nBLAux, nBreads, nBwrites);
                }

            } else {//bloquePunteros[i]=0
                switch (nivel_punteros) {// Saltos al valer 0 un puntero según nivel
                case 1:
                    *nBL = *nBL + 1;
                    break;
                case 2:
                    *nBL += NPUNTEROS;
                    break;
                case 3:
                    *nBL += NPUNTEROS * NPUNTEROS;
                    break;
                }
            }

            if (*nBL > ultimoBL) {
                *eof = 1;
            }
        }

        // Si el bloque de punteros es distinto al original
        if (memcmp(bloquePunteros, bloquePunteros_Aux, BLOCKSIZE) != 0) {
            if (memcmp(bloquePunteros, bufferCeros, BLOCKSIZE) != 0) {
                bwrite(*ptr, bloquePunteros);
                *nBwrites = *nBwrites + 1;
#if DEBUG
                printf("[liberar_bloques_inodo()→ salvado BF %d de punteros_nivel%d correspondiente al BL %d\n", *ptr, nivel_punteros - 1, *nBLAux);
#endif
            } else {
                liberar_bloque(*ptr); //de punteros
#if DEBUG
                printf("[liberar_bloques_inodo()→ liberado BF = %d nivel_punteros%d correspondiente BL %d)]\n", *ptr, nivel_punteros - 1, *nBL);
#endif
                * ptr = 0; //ponemos a 0 el puntero que apuntaba al bloque liberado
                liberados++;
            }
        }

    } else { // *ptr=0
        //sólo entrará si es un puntero del inodo, resto de casos sólo se llama recursivamente con *ptr!=0
        switch (nRangoBL) {
        case 1:
            *nBL = INDIRECTOS0;
            break;
        case 2:
            *nBL = INDIRECTOS1;
            break;
        case 3:
            *nBL = INDIRECTOS2;
            break;
        }

    }

    return liberados;
}
