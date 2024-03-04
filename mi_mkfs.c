#include "ficheros_basico.h"

int main(int argc, char **argv)
{
    ////Comprobamos sintaxis
    if (argc < 3)
    {
        fprintf(stderr, "Error al introducir el comando\n");
        return -1;
    }
    // Montamos dispositivo usando el nombre introducido en consola
    bmount(argv[1]);
    // Creamos un bloque libre (todo 0)
    unsigned char libre[BLOCKSIZE]; // esto esta en bloques.h
    // Ponemos los bits de cada byte del bloque a 0.
    memset(libre, 0, BLOCKSIZE);
    // Convertimos la cantidad de nbloques pasada por consola a un entero.
    int nbloques = atoi(argv[2]);
    // bucle para escribir nbloques veces un bloque libre
    for (int i = 0; i < nbloques; i++)
    {
        bwrite(i, libre);
    }

    initSB();
    initMB();
    initAI();

    reservar_inodo ('d', 7);

    bumount(argv[1]);
    return 0;
}