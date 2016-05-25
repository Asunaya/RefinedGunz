/* zd_mem.c : zdelta dynamic allocation memory
 * used by zd_compress1 and zd_uncomperss1
       
   file modified by Dimitre Trendafilov (2003)   
 */

#include "zd_mem.h"

size_t zd_alloc(zd_mem_buffer *m, size_t n){

  if(n>0){
    if((m->buffer= (Bytef *) malloc(n * sizeof(Byte))) == NULL){
      perror("zd_alloc: could not allocate");
      return 0;
    }

    m->pos= m->buffer;
    m->size= n;
    return n;
  }

  m->size = 0;
  return 0;
}

size_t zd_realloc(zd_mem_buffer *m, size_t n){

  Bytef *p;
  int tmp_pos= (m->pos - m->buffer);

  if(n >0){
    p= (Bytef *) realloc(m->buffer, (m->size+n));
    if(p == NULL){
      perror("zd_realloc: could not realloc");
      return 0;
    }
    /* reset newly allocated memory*/
    if (m->buffer != p)
    {
       m->pos = p + tmp_pos;
       m->buffer= p;
    }

    m->size += n;
    return n;
  }
  
  return 0;
}

size_t zd_free(zd_mem_buffer *m){

  int n=0;
  
  if(m->size >0){
    free(m->buffer);
    m->pos= NULL;
    n= m->size; m->size=0;
    /*m->buffer= NULL;*/
  }
  
  return n;
}

size_t zd_memcpy(zd_mem_buffer *m, void *data, size_t n){

  size_t avail=0;
  avail = (m->size - (m->pos - m->buffer));
  if(avail < n)
    if(zd_realloc(m, (n-avail)+1)== 0)
      return 0;
  
  memcpy(m->pos, data, n);
  m->pos += n;
  return n;
}


int ZEXPORT dread_file(FILE *input, zd_mem_buffer* buffer){

  int nbread;

  assert(input!=NULL);
  assert(buffer!=NULL);
  zd_alloc(buffer, ZD_BUF_SIZE);
  for(;;){

    /* read data */
    nbread = fread((void *) buffer->pos, 
		   sizeof(Byte), (size_t) ZD_BUF_SIZE, input); 
    buffer->pos=buffer->pos+nbread;

    /* check for end of file */
    if(feof(input)){
      return (buffer->pos - buffer->buffer);
    }

    /* check for io error */
    if(ferror(input)){
      zd_free(buffer);
      return ZD_ERRNO;
    }

    /* increase the buffer */
    if(!zd_realloc(buffer, ZD_BUF_SIZE)){
      zd_free(buffer);
      /* not enough memory */
      return ZD_MEM_ERROR;
    }
  }		
}


