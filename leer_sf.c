/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

#include "directorios.h"


void mostrar_buscar_entrada(char* camino, char reservar) {
  unsigned int p_inodo_dir = 0;
  unsigned int p_inodo = 0;
  unsigned int p_entrada = 0;
  int error;
  printf("\ncamino: %s, reservar: %d\n", camino, reservar);
  if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
    mostrar_error_buscar_entrada(error);
  }
  printf("**********************************************************************\n");
  return;
}


int main(int argc, char** argv) {
  if (argv[1] == NULL) {
    fprintf(stderr, "Paràmetros no especificados. Uso: leer_sf <nombre_dispositivo>\n");
    exit(1);
  }

  bmount(argv[1]);
  struct superbloque SB;

  if (bread(posSB, &SB) == -1) {
    fprintf(stderr, "Error en leer_sf.c --> %d: %s\nerror al leer el superbloque", errno, strerror(errno));
    return FALLO;
  }

  printf("\nDatos del Superbloque:\n");
  printf("\tposPrimerBloqueMB = %d\n", SB.posPrimerBloqueMB);
  printf("\tposUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);
  printf("\tposPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
  printf("\tposUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
  printf("\tposPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
  printf("\tposUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
  printf("\tposInodoRaiz = %d\n", SB.posInodoRaiz);
  printf("\tposPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
  printf("\tcantBloquesLibres = %d\n", SB.cantBloquesLibres);
  printf("\tcantInodosLibres = %d\n", SB.cantInodosLibres);
  printf("\ttotBloques = %d\n", SB.totBloques);
  printf("\ttotInodos = %d\n\n", SB.totInodos);

  //Mostrar creación directorios y errores
  // mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
  // mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
  // mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
  // mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
  // mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
  // mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
  // mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);
  // //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
  // mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
  // mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
  // mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //ERROR_ENTRADA_YA_EXISTENTE
  // mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
  // mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2

  /*
  // Reservar bloque
  int bloque_reservado = reservar_bloque();
  bread(posSB,&SB);
  printf("\nReservado el bloque %d\nBloques libres: %d\n", bloque_reservado, SB.cantBloquesLibres);
  // Liberar bloque
  liberar_bloque(bloque_reservado);
  bread(posSB,&SB);
  printf("Liberado el bloque %d\nBloques libres: %d\n\n", bloque_reservado, SB.cantBloquesLibres);
  printf("bits de las zonas del dispositivo\n");
  printf("bit leido en posSB (bloque n.%d): %d\n", posSB, leer_bit(posSB));
  printf("bit leido en posPrimerBloqueMB (bloque n.%d): %d\n", SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));
  printf("bit leido en posUltimoBloqueMB (bloque n.%d): %d\n", SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));
  printf("bit leido en posPrimerBloqueAI (bloque n.%d): %d\n", SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));
  printf("bit leido en posUltimoBloqueAI (bloque n.%d): %d\n", SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));
  printf("bit leido en posPrimerBloqueDatos (bloque n.%d): %d\n", SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));
  printf("bit leido en posUltimoBloqueDatos (bloque n.%d): %d\n\n", SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));

  // Lectura datos
  printf("Lectura datos del directorio raíz\n");
  struct tm *ts;
  char atime[80];
  char mtime[80];
  char ctime[80];
  struct inodo inodo;
  int ninodo = 0;
  if (leer_inodo(ninodo,&inodo) == -1) {
    fprintf(stderr, "Error en leer_sf.c %d: %s\n error al leer el inodo", errno, strerror(errno));
    return FALLO;
  }
  ts = localtime(&inodo.atime);
  strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
  ts = localtime(&inodo.mtime);
  strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
  ts = localtime(&inodo.ctime);
  strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
  printf("tipo: %c\n", inodo.tipo);
  int permisos = (int) inodo.permisos;
  printf("Permisos: %d\n", permisos);
  printf("atime: %s\n", atime);
  printf("mtime: %s\n", mtime);
  printf("ctime: %s\n", ctime);
  printf("nlinks: %d\n", inodo.nlinks);
  printf("tamaño en bytes lógicos: %d\n", inodo.tamEnBytesLog);
  printf("NumBloquesOcupados: %d\n", inodo.numBloquesOcupados);

  printf("\nFunciones de traducciòn de bloques inodos\n ");
  bread(posSB,&SB);
  printf("\nposPrimerInodoLibre: %d\n",SB.posPrimerInodoLibre);
  int inodoReservado = reservar_inodo('f',6);

  printf("Se ha reservado el inodo %d\n", inodoReservado);
  printf("\nTRADUCCIÓN DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n");
  printf("Traducción bloque lògico 8: %d\n",traducir_bloque_inodo(1,8,'0'));
  printf("Traducción bloque lògico 204: %d\n",traducir_bloque_inodo(1,204,'0'));
  printf("Traducción bloque lògico 30.004: %d\n",traducir_bloque_inodo(1,30004,'0'));
  printf("Traducción bloque lògico 400.004: %d\n",traducir_bloque_inodo(1,400004,'0'));
  printf("Traducción bloque lògico 468.750: %d\n",traducir_bloque_inodo(1,468750,'0'));

  printf("\nLectura datos del inodo %d\n", inodoReservado);
  ninodo = inodoReservado;
  if (leer_inodo(ninodo,&inodo) == -1) {
    fprintf(stderr, "Error en leer_sf.c %d: %s\nImposible leer el inodo", errno, strerror(errno));
    return FALLO;
  }
  ts = localtime(&inodo.atime);
  strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
  ts = localtime(&inodo.mtime);
  strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
  ts = localtime(&inodo.ctime);
  strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

  printf("Tipo: %c\n", inodo.tipo);
  permisos = (int) inodo.permisos;
  printf("Permisos: %d\n", permisos);
  printf("atime: %s\n", atime);
  printf("mtime: %s\n", mtime);
  printf("ctime: %s\n", ctime);
  printf("N. links: %d\n", inodo.nlinks);
  printf("Tamaño en bytes lógicos: %d\n", inodo.tamEnBytesLog);
  printf("numBloquesOcupados: %d\n", inodo.numBloquesOcupados);
  bread(posSB,&SB);
  printf("posPrimerInodoLibre (después de la reserva): %d\n\n",SB.posPrimerInodoLibre);
  */
  bumount();
  return 0;
}