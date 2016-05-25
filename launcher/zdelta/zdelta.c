/* file zdelta.c; copyright Dimitre Trendafilov (2003)
 * zdelta interface
 */

#include <stdio.h>      /* provides FILE */
#include "zd_mem.h"     /* provides dynamic memory allocation */
#include "zutil.h"

#include "zdlib.h"


/*      constant definitions    */

#define EXPECTED_RATIO 4

#if REFNUM > 4
#error: Support provided for AT MOST 4 reference files!!!
#endif

/*===========================================================================
 *  function implementation
 *  see zdlib.h for documentation
 */

/*
 * COMPRESSOR WRAPPERS 
 */

int ZEXPORT zd_compress(const Bytef *ref,  uLong rsize,
			const Bytef *tar,  uLong tsize,
			Bytef *delta, uLongf *dsize)
{
  int rval;
  zd_stream strm;   

  /* init io buffers */
  strm.base[0]  = (Bytef*) ref;
  strm.base_avail[0] = rsize;
  strm.base_out[0] = 0;
  strm.refnum      = 1;

  strm.next_in  = (Bytef*) tar;
  strm.total_in = 0;
  strm.avail_in = tsize;

  strm.next_out  = delta; 
  strm.total_out = 0;
  strm.avail_out = *dsize; 

  /* init the rest of the stream structure */
  strm.zalloc = (alloc_func)0;
  strm.zfree = (free_func)0;
  strm.opaque = (voidpf)0;
  rval = zd_deflateInit(&strm, ZD_DEFAULT_COMPRESSION);
  if (rval != ZD_OK)
  {
    fprintf(stderr, "%s error: %d\n", "deflateInit", rval);
    return rval;
  }

  /* compress the data */
  rval = zd_deflate(&strm,ZD_FINISH);
  if(rval != ZD_STREAM_END){
    zd_deflateEnd(&strm);
    return rval == ZD_OK ? ZD_BUF_ERROR : rval;
  }

  *dsize = strm.total_out;
  /* release memory */
  return zd_deflateEnd(&strm);
}

/*
 * same as above, but with dynamic memory allocation
 */
int ZEXPORT zd_compress1(const Bytef *ref, uLong rsize,
			 const Bytef *tar, uLong tsize,
			 Bytef **delta, uLongf *dsize)
{
  int rval;
  zd_stream strm;
  zd_mem_buffer dbuf;

  /* the zstream output buffer should have size greater than zero try to
   * guess buffer size, such that no memory realocation will be needed 
   */ 
  if(!(*dsize)) *dsize = tsize/EXPECTED_RATIO + 64; /* *dsize should not be 0*/

  /* init io buffers */
  strm.base[0]  = (Bytef*) ref;
  strm.base_avail[0] = rsize;
  strm.base_out[0] = 0;
  strm.refnum      = 1;

  strm.next_in  = (Bytef*) tar;
  strm.total_in = 0;
  strm.avail_in = tsize;

  /* allocate the output buffer */
  zd_alloc(&dbuf, *dsize);	
  
  strm.next_out  = dbuf.pos;
  strm.total_out = 0;
  strm.avail_out = *dsize; 

  strm.zalloc = (alloc_func)0;
  strm.zfree = (free_func)0;
  strm.opaque = (voidpf)0;

  /* init huffman coder */
  rval = zd_deflateInit(&strm, ZD_DEFAULT_COMPRESSION);
  if (rval != ZD_OK)
  {
    fprintf(stderr, "%s error: %d\n", "deflateInit", rval);
    return rval;
  }

  /* compress the data */
  while((rval = zd_deflate(&strm,ZD_FINISH)) == ZD_OK){

    /* set correctly the mem_buffef internal pointer */
    dbuf.pos = strm.next_out; 

    /* allocate more memory */
    zd_realloc(&dbuf,dbuf.size);

    /* restore zstream internal pointer */
    strm.next_out = (uch*)dbuf.pos;
    strm.avail_out = dbuf.size - strm.total_out;
  }

  /* set correcty the mem_buffer pointers */
  dbuf.pos = strm.next_out; 

  if(rval != ZD_STREAM_END){
    fprintf(stderr, "%s error: %d\n", "deflateInit", rval);
    zd_free(&dbuf);
    zd_deflateEnd(&strm);
    return rval;
  }

  *delta = dbuf.buffer;
  *dsize = (uLong) strm.total_out;

  /* release memory */
  return zd_deflateEnd(&strm);
}

#if REFNUM > 1
/* compresses ONE target file in relation to N reference files */
int ZEXPORT zd_compressN(const Bytef *ref[], uLong rsize[], int rnum, 
			 const Bytef *tar, uLong tsize,
			 Bytef *delta, uLongf *dsize)
{
  int rval;
  int i;
  zd_stream strm;

  /* init io buffers */

  /* init reference data buffers */
  strm.refnum = rnum;
  for(i = 0; i<rnum; ++i){
    strm.base[i]  = (Bytef*) ref[i];
    strm.base_avail[i] = rsize[i];
    strm.base_out[i] = 0;
  }
  /* init target data buffers */
  strm.next_in  = (Bytef*) tar;
  strm.total_in = 0;
  strm.avail_in = tsize;

  strm.next_out  = delta; 
  strm.total_out = 0;
  strm.avail_out = *dsize; 

  strm.zalloc = (alloc_func)0;
  strm.zfree = (free_func)0;
  strm.opaque = (voidpf)0;

  /* init huffman coder */
  rval = zd_deflateInit(&strm, ZD_DEFAULT_COMPRESSION);
  if (rval != ZD_OK)
  {
    fprintf(stderr, "%s error: %d\n", "deflateInit", rval);
    return rval;
  }

  /* compress the data */
  rval = zd_deflate(&strm,ZD_FINISH);

  if(rval != ZD_STREAM_END){
    zd_deflateEnd(&strm);
    return rval == ZD_OK ? ZD_BUF_ERROR : rval;
  }

  *dsize = strm.total_out;
  /* release memory */
  return zd_deflateEnd(&strm);
}

/* compresses ONE target file in relation to N reference files */
/*
 * same as above, but with dynamic memory allocation
 */
int ZEXPORT zd_compressN1(const Bytef *ref[], uLong rsize[], int rnum, 
			  const Bytef *tar, uLong tsize,
			  Bytef **delta, uLongf *dsize)
{
  int rval;
  int i;
  zd_stream strm;
  zd_mem_buffer dbuf;
  
  /* the zstream output buffer should have size greater than zero try to
   * guess buffer size, such that no memory realocation will be needed 
   */ 
  if(!(*dsize)) *dsize = tsize/EXPECTED_RATIO+64;/* *dsize should not be 0*/ 

  /* init reference data buffers */
  strm.refnum = rnum;
  for(i = 0; i<rnum; ++i){
    strm.base[i]  = (Bytef*) ref[i];
    strm.base_avail[i] = rsize[i];
    strm.base_out[i] = 0;
  }
  /* init target data buffers */
  strm.next_in  = (Bytef*) tar;
  strm.total_in = 0;
  strm.avail_in = tsize;

  /* allocate the output buffer */
  zd_alloc(&dbuf, *dsize);	
  
  strm.next_out  = dbuf.pos;
  strm.total_out = 0;
  strm.avail_out = *dsize; 

  strm.zalloc = (alloc_func)0;
  strm.zfree = (free_func)0;
  strm.opaque = (voidpf)0;

  /* init huffman coder */
  rval = zd_deflateInit(&strm, ZD_DEFAULT_COMPRESSION);
  if (rval != ZD_OK)
  {
    fprintf(stderr, "%s error: %d\n", "deflateInit", rval);
    return rval;
  }

  /* compress the data */
  while((rval = zd_deflate(&strm,ZD_FINISH)) == ZD_OK){

    /* set correctly the mem_buffef internal pointer */
    dbuf.pos = strm.next_out; 

    /* allocate more memory */
    zd_realloc(&dbuf,dbuf.size);

    /* restore zstream internal pointer */
    strm.next_out = (uch*)dbuf.pos;
    strm.avail_out = dbuf.size - strm.total_out;
  }

  /* set correcty the mem_buffer pointers */
  dbuf.pos = strm.next_out; 

  if(rval != ZD_STREAM_END){
    fprintf(stderr, "%s error: %d\n", "deflateInit", rval);
    zd_free(&dbuf);
    zd_deflateEnd(&strm);
    return rval;
  }

  *delta = dbuf.buffer;
  *dsize = (uLong) strm.total_out;

  /* release memory */
  return zd_deflateEnd(&strm);
}

#endif /* REFNUM>1 */

/*
 * DECOMPRESSOR WRAPPERS 
 */
int ZEXPORT zd_uncompress(const Bytef *ref, uLong rsize,
			  Bytef *tar, uLongf* tsize,
			  const Bytef *delta, uLong dsize)
{
  int rval;
  int f = ZD_FINISH;
  zd_stream strm;  
 
  /* init io buffers */
  strm.base[0]       = (Bytef*) ref;
  strm.base_avail[0] = rsize;
  strm.refnum        = 1;

  strm.next_out  = (Bytef*) tar;
  strm.total_out = 0;
  strm.avail_out = *tsize;

  strm.next_in  = (Bytef*) delta;
  strm.total_in = 0;
  strm.avail_in = dsize;

  strm.zalloc = (alloc_func)0;
  strm.zfree  = (free_func)0;
  strm.opaque = (voidpf)0;
  rval = zd_inflateInit(&strm);

  if (rval != ZD_OK)
  {
    fprintf(stderr, "%s error: %d\n", "zd_InflateInit", rval);
    return rval;
  }

  /* decompress the data */
  rval = zd_inflate(&strm,f);
 
  if(rval != ZD_STREAM_END){
    zd_inflateEnd(&strm);
    return rval;
  }
  *tsize = strm.total_out;
  return zd_inflateEnd(&strm);
}

/*
 * same as above but with dynamic memory allocation
 */
int ZEXPORT zd_uncompress1(const Bytef *ref, uLong rsize,
			   Bytef **tar,  uLongf *tsize,
			   const Bytef *delta, uLong dsize)
{
  int rval;
  int f = ZD_SYNC_FLUSH;
  zd_mem_buffer tbuf;  
  zd_stream strm;
  
  /* zdelta: tsize must not be 0; try to guess a good output buffer size */
  if(!(*tsize)) *tsize = rsize*2+64; /* *tsize should not be 0*/

  /* init io buffers */
  strm.base[0]       = (Bytef*) ref;
  strm.base_avail[0] = rsize;
  strm.refnum        = 1;

  /* allocate initial size delta buffer */
  zd_alloc(&tbuf,*tsize);
  strm.avail_out = *tsize;
  strm.next_out  = tbuf.buffer;
  strm.total_out = 0;

  strm.avail_in = dsize;
  strm.next_in  = (Bytef*) delta;
  strm.total_in = 0;

  strm.zalloc = (alloc_func)0;
  strm.zfree  = (free_func)0;
  strm.opaque = (voidpf)0;
  rval = zd_inflateInit(&strm);
  if (rval != ZD_OK)
  {
    fprintf(stderr, "%s error: %d\n", "zd_InflateInit", rval);
    return rval;
  }

  while((rval = zd_inflate(&strm,f)) == ZD_OK){
    /* add more output memory */
    /*
    if(strm.avail_out!=0){
      rval = ZD_STREAM_END;
      break;
    } 
    */
    /* set correctly the mem_buffer internal pointer */
    tbuf.pos = strm.next_out; 

    /* allocate more memory */
    zd_realloc(&tbuf,tbuf.size);      

    /* restore zstream internal pointer */
    strm.next_out = tbuf.pos; 
    strm.avail_out = tbuf.size - strm.total_out;
  }

  /* set correctly the mem_buffer internal pointer */
  tbuf.pos = strm.next_out; 

  if(rval != ZD_STREAM_END){
    zd_free(&tbuf);
    if(strm.msg!=NULL) fprintf(stderr,"%s\n",strm.msg);
    zd_inflateEnd(&strm);
    return rval;
  }

  *tar   = tbuf.buffer;
  *tsize = strm.total_out;

  /* free memory */
  return zd_inflateEnd(&strm);
}


#if REFNUM>1
/*
 * static memory allocation,  *** N *** reference files
 */
int ZEXPORT zd_uncompressN(Bytef *ref[], uLong rsize[], int rnum,
			   const Bytef *tar, uLongf *tsize,
			   const Bytef *delta, uLong dsize)

{
  int rval, i;
  int f = ZD_FINISH;
  zd_stream strm;  

  /* init io buffers */
  strm.refnum = rnum;
  /* init reference input buffers */
  for(i = 0; i < rnum; ++i){
    strm.base[i]  = (Bytef*) ref[i];
    strm.base_avail[i] = rsize[i];
    strm.base_out[i] = 0;
  }

  strm.next_out  = (Bytef*) tar;
  strm.total_out = 0;
  strm.avail_out = *tsize;

  strm.next_in  = (Bytef*) delta;
  strm.total_in = 0;
  strm.avail_in = dsize;

  strm.zalloc = (alloc_func)0;
  strm.zfree  = (free_func)0;
  strm.opaque = (voidpf)0;
  rval = zd_inflateInit(&strm);
  if (rval != ZD_OK)
  {
    fprintf(stderr, "%s error: %d\n", "zd_InflateInit", rval);
    return rval;
  }

  /* decompress the data */
  rval = zd_inflate(&strm,f);
  /* decompress the data */
  rval = zd_inflate(&strm,f);
 
  if(rval != ZD_STREAM_END){
    zd_inflateEnd(&strm);
    return rval;
  }
  *tsize = strm.total_out;
  return zd_inflateEnd(&strm);
}


/*
 * same as above with dynamic memory allocation
 */
int ZEXPORT zd_uncompressN1(const Bytef *ref[], uLong rsize[], int rnum,
			    Bytef **tar,  uLongf *tsize,
			    const Bytef *delta, uLong dsize)
{
  int rval;
  int f = ZD_SYNC_FLUSH;
  int i;
  zd_mem_buffer tbuf;  
  zd_stream strm;                        
  
  /* zdelta: tsize must not be 0; try to guess a good output buffer size */
  if(!(*tsize)) *tsize = rsize[0]*2+64;/* *tsize should not be 0*/

  /* init io buffers */
  strm.refnum = rnum;
  /* init reference input buffers */
  for(i = 0; i < rnum; ++i){
    strm.base[i]  = (Bytef*) ref[i];
    strm.base_avail[i] = rsize[i];
    strm.base_out[i] = 0;
  }

  /* allocate initial size delta buffer */
  zd_alloc(&tbuf,*tsize);
  strm.avail_out = *tsize;
  strm.next_out  = tbuf.buffer;
  strm.total_out = 0;

  strm.avail_in = dsize;
  strm.next_in  = (Bytef*) delta;
  strm.total_in = 0;

  strm.zalloc = (alloc_func)0;
  strm.zfree  = (free_func)0;
  strm.opaque = (voidpf)0;
  zd_inflateInit(&strm);

  while((rval = zd_inflate(&strm,f)) == ZD_OK){
    /* add more output memory */
    
    if(strm.avail_out!=0){
      rval = ZD_STREAM_END;
      break;
    } 
    
    /* set correctly the mem_buffer internal pointer */
    tbuf.pos = strm.next_out; 

    /* allocate more memory */
    zd_realloc(&tbuf,tbuf.size);      

    /* restore zstream internal pointer */
    strm.next_out  = tbuf.pos; 
    strm.avail_out = tbuf.size - strm.total_out;
  }

  /* set correctly the mem_buffer internal pointer */
  tbuf.pos = strm.next_out; 

  if(rval != ZD_STREAM_END){
    zd_free(&tbuf);
    if(strm.msg!=NULL) fprintf(stderr,"%s\n",strm.msg);
    zd_inflateEnd(&strm);
    return rval;
  }

  *tar   = tbuf.buffer;
  *tsize = strm.total_out;

  /* free memory */
  return zd_inflateEnd(&strm);
}

#endif /* REFNUM > 1 */
