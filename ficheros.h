#include "bloques.h"
#include "ficheros_basico.h"
struct STAT {
    unsigned char tipo;
    unsigned char permisos;
    time_t atime;
    time_t ctime;
    time_t mtime;
    unsigned int nlinks;
    unsigned int numBloquesOcupados;
    unsigned int tamEnBytesLog;
};
int mi_write_f(unsigned int ninodo, const void *buf_original,unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);

