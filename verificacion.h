//verificacion.h
#include "simulacion.h"

/**
 * @author Bernat Parera
 * @author Rafael Crespí
 * @author Otto Vásquez
*/

struct INFORMACION {
  int pid;
  unsigned int nEscrituras; //validadas
  struct REGISTRO PrimeraEscritura;
  struct REGISTRO UltimaEscritura;
  struct REGISTRO MenorPosicion;
  struct REGISTRO MayorPosicion;
};

