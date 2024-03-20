#include "ficheros.h"

int mi_write_f(unsigned int ninodo,const void *buf_original,unsigned int offset,unsigned int nbytes){
    struct inodo inodo;
    unsigned int primerBL, ultimoBL,desp1,desp2,nbfisico;
    unsigned char buf_bloque[BLOCKSIZE];
    if((inodo.permisos &2)!=2){
        fprintf(stderr,RED,"No hay permisos de escritura/n" RESET);
    }
    primerBL=offset/BLOCKSIZE;
    ultimoBL=(offset+nbytes-1)/BLOCKSIZE;
    desp1=offset % BLOCKSIZE;
    desp2=(offset + nbytes-1)%BLOCKSIZE;
    //primer caso el buffer cabe en un solo bloque
    if(primerBL==ultimoBL){ //obtenemos el numero de bloque
        nbfisico=traducir_bloque_inodo(ninodo,primerBL,1);
        memcpy(buf_bloque+desp1,buf_original,nbytes);
        if(bwrite(nbfisico,buf_bloque)==-1){
            return -1;
        }

    }else{
        // la operacion afecta a mas de un bloque
        nbfisico=traducir_bloque_inodo(ninodo,primerBL,1);
        if(nbfisico==-1){
            return -1;
        }
        if(bread(nbfisico,buf_bloque)==-1){
            return -1;
        }
        memcpy(buf_bloque+desp1,buf_original,BLOCKSIZE-desp2);
        if (bwrite(nbfisico,buf_bloque)==-1){
            return -1;
        }
        //bloques intermedios
        for(int i=primerBL+1;i<ultimoBL;i++){
            if(bwrite(nbfisico,buf_original+(BLOCKSIZE-desp1)+(i-primerBL-1)*BLOCKSIZE)==-1){
                return -1;
            }
        }
        nbfisico=traducir_bloque_inodo(ninodo,ultimoBL,1);
        if(bread(nbfisico,buf_original)==-1){
            return -1;
        }
        desp2=(offset+nbytes-1)%BLOCKSIZE; // desplazamiento para comprobar nbytes escritos despues del offset
        memcpy(buf_bloque,buf_original+(nbytes-(desp1-1)),desp2+1);
        if(bwrite(nbfisico,buf_bloque)==-1){
            return -1;
        }
    }
    // actualizamos el inodo
    if(leer_inodo(ninodo,&inodo)==-1){
        return -1;
    }
    if((offset+nbytes)>inodo.tamEnBytesLog){
        inodo.tamEnBytesLog=offset+nbytes;
        inodo.ctime=time(NULL);
    }
    inodo.mtime=time(NULL);
    if(escribir_inodo(ninodo,inodo)==-1){
        return -1;
    }
    return nbytes; // numero de bytes escritos
}
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){

}
int mi_stat_f(unsigned int ninodo,struct STAT *p_stat){
    struct inodo inodo;
    p_stat->tipo=inodo.tipo;
    p_stat->permisos=inodo.permisos;
    p_stat->atime=inodo.atime;
    p_stat->mtime=inodo.mtime;
    p_stat->ctime=inodo.ctime;
    p_stat->nlinks=inodo.nlinks;
    p_stat->numBloquesOcupados=inodo.numBloquesOcupados;
    p_stat->tamEnBytesLog=inodo.tamEnBytesLog;
    return 1;

}
int mi_chmod_f(unsigned int ninodo,unsigned char permisos){
    struct inodo inodo;
    inodo.permisos=permisos;
    inodo.ctime=ctime(NULL);
    return 1;
}