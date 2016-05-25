/* zd_mem.h : zdelta dynamic allocation memory
 * used by zd_compress1 and zd_uncomperss1
         
   file modified by Dimitre Trendafilov (2003)   
 */

#ifndef __ZD_MEM_H
#define __ZD_MEM_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zdlib.h" /* provides return codes */

typedef struct _mem_buffer{
  Bytef *buffer;
  Bytef *pos;
  size_t size;
}zd_mem_buffer;

size_t zd_alloc OF ((zd_mem_buffer*, size_t));
size_t zd_realloc OF ((zd_mem_buffer*, size_t));
size_t zd_free OF ((zd_mem_buffer*));
size_t zd_memcpy OF ((zd_mem_buffer *, void *, size_t));

/* default buffer size; used by dread_file() */
#define ZD_BUF_SIZE	1024*128

/* reads a file into mem_buffer */
int dread_file OF ((FILE *, zd_mem_buffer*));

#endif

