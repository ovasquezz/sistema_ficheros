/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

#include "directorios.h"
#define DEBUG7 0

/**
 * Funcion auxiliar para hacer pruebas
*/
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

#if DEBUG7
  //Mostrar creación directorios y errores
  mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
  mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
  mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
  mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
  mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
  mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
  mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);
  //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
  mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
  mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
  mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //ERROR_ENTRADA_YA_EXISTENTE
  mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
  mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
#endif

  bumount();
  return 0;
}