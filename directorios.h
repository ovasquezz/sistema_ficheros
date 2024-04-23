#define TAMNOMBRE 60 //tamaño del nombre de directorio o fichero, en Ext2 = 256
struct entrada {
  char nombre[TAMNOMBRE];
  unsigned int ninodo;
};
