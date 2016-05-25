/* file: zdlib.h; copyright Dimitre Trendafilov (2003)
 * definitions of zdelta interface functions
 */

#ifndef ZD_LIBZD_H
#define ZD_LIBZD_H

#include "zdconf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 * constant definitions        
 */

#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1 /* will be removed, use Z_SYNC_FLUSH instead */

#define ZD_SYNC_FLUSH    2
#define ZD_FULL_FLUSH    3
#define ZD_FINISH        4
/* Allowed flush values; see zd_deflate() below for details */

#define ZD_HUFFMAN_ONLY        0
#define ZD_FILTERED            5
#define ZD_DEFAULT_STRATEGY    7
/* compression strategy; see zd_deflateInit2() below for details */

#define ZD_NO_COMPRESSION         0
#define ZD_BEST_SPEED             1
#define ZD_BEST_COMPRESSION       9
#define ZD_DEFAULT_COMPRESSION  (-1)
/* compression levels */

#define Z_BINARY   0
#define Z_ASCII    1
#define Z_UNKNOWN  2
/* Possible values of the data_type field */

#define ZD_DEFLATED  5 /* randomly choosen 8,15 are reserved */
/* The deflate compression method (the only one supported in this version) */

#define ZD_NULL  0  /* for initializing zalloc, zfree, opaque */


/* Return codes for the compression/decompression functions. Negative
 * values are errors, positive values are used for special but normal events.
 *
 * zdelta: match the return codes in zlib 1.1.3; the prefix is changed to ZD
 */
#define ZD_OK            0
#define ZD_STREAM_END    1
#define ZD_ERRNO        (-1)
#define ZD_STREAM_ERROR (-2)
#define ZD_DATA_ERROR   (-3)
#define ZD_MEM_ERROR    (-4)
#define ZD_BUF_ERROR    (-5)
#define ZD_VERSION_ERROR (-6)

#ifndef REFNUM
#define REFNUM 1            /* Reference Window Number */
#endif
#if REFNUM < 1 || REFNUM > 4
#error: incorrect REFNUM value 
#endif 

#define ZDLIB_VERSION "2.1"
/* current libzd version */

/* 
     The 'libzd' compression library provides in-memory compression and
  decompression functions, including integrity checks of the uncompressed
  data.  This version of the library supports only one compression method
  (delta_deflation) 
  NOTE: 'libzd' complies with the ZLIB Compressed Data Format Specification

     Compression can be done in a single step if the buffers are large
  enough (for example if an input file is mmap'ed), or can be done by
  repeated calls of the compression function.  In the latter case, the
  application must provide more input and/or consume the output
  (providing more output space) before each call.

     The library does not install any signal handler. The decoder checks
  the consistency of the compressed data, so the library should never
  crash even in case of corrupted input.
*/

/* ==========================================================================
 * struct definitions        
 */

typedef voidpf (*alloc_func) OF((voidpf opaque, uInt items, uInt size));
typedef void   (*free_func)  OF((voidpf opaque, voidpf address));

struct zd_internal_state;

typedef struct zd_stream_s {
  Bytef    *next_in;  /* next input byte */
  uInt     avail_in;  /* number of bytes available at next_in */
  uLong    total_in;  /* total nb of input bytes read so far */
  
  Bytef    *next_out; /* next output byte should be put there */
  uInt     avail_out; /* remaining free space at next_out */
  uLong    total_out; /* total nb of bytes output so far */
  
  Bytef    *base[REFNUM];      /* pointer to the base window    */
  uLong    base_out[REFNUM];   /* total read bytes from the base window */
  uLong    base_avail[REFNUM];
  int      refnum;

  char     *msg;      /* last error message, NULL if no error */
  struct zd_internal_state FAR *state; /* not visible by applications */

  alloc_func zalloc;  /* used to allocate the internal state */
  free_func  zfree;   /* used to free the internal state */
  voidpf     opaque;  /* private data object passed to zalloc and zfree */

  int     data_type;  /* best guess about the data type: ascii or binary */
  uLong   adler;      /* adler32 value of the uncompressed data */
  uLong   reserved;   /* reserved for future use */
 
} zd_stream;

typedef zd_stream FAR *zd_streamp;

/*
   The application must update next_in and avail_in when avail_in has
   dropped to zero. It must update next_out and avail_out when avail_out
   has dropped to zero. The application must initialize zalloc, zfree and
   opaque before calling the init function. All other fields are set by the
   compression library and must not be updated by the application.

   The opaque value provided by the application will be passed as the first
   parameter for calls of zalloc and zfree. This can be useful for custom
   memory management. The compression library attaches no meaning to the
   opaque value.

   zalloc must return ZD_NULL if there is not enough memory for the object.
   If zdelta library is used in a multi-threaded application, zalloc and 
   zfree must be thread safe.

   The fields total_in and total_out can be used for statistics or
   progress reports. After compression, total_in holds the total size of
   the uncompressed data and may be saved for use in the decompressor
   (particularly if the decompressor wants to decompress everything in
   a single step).
*/

/* ==========================================================================
 * basic functions        
 */

ZEXTERN const char * ZEXPORT zdlibVersion OF((void));
/* 
   The application can compare zdlibVersion and ZDLIB_VERSION for consistency.
   If the first character differs, the library code actually used is
   not compatible with the zdlib.h header file used by the application.
   This check is automatically made by zd_deflateInit and zd_inflateInit.
 */

/* 
ZEXTERN int ZEXPORT zd_deflateInit OF((zd_streamp strm, int level));

     Initializes the internal stream state for compression. The fields
   zalloc, zfree and opaque must be initialized before by the caller.
   If zalloc and zfree are set to ZD_NULL, zd_deflateInit updates them to
   use default allocation functions.

     The compression level must be ZD_DEFAULT_COMPRESSION, or between 0 and 9:
   1 gives best speed, 9 gives best compression, 0 gives no compression at
   all (the input data is simply copied a block at a time).
   ZD_DEFAULT_COMPRESSION requests a default compromise between speed and
   compression (currently equivalent to level 6).

     zd_deflateInit returns ZD_OK if success, ZD_MEM_ERROR if there was not
   enough memory, ZD_STREAM_ERROR if level is not a valid compression level,
   ZD_VERSION_ERROR if the zdlib library version (zdlibVersion) is 
   incompatible with the version assumed by the caller (ZDLIB_VERSION).
   msg is set to null if there is no error message.  zd_deflateInit does not
   perform any compression: this will be done by zd_deflate().
*/

ZEXTERN int ZEXPORT zd_deflate OF((zd_streamp strm, int flush));
/*
    zd_deflate compresses as much data as possible, and stops when the input
  buffer becomes empty or the output buffer becomes full. It may introduce some
  output latency (reading input without producing any output) except when
  forced to flush. 

    The detailed semantics are as follows. zd_deflate performs one or both of 
  the following actions:

  - Compress more input starting at next_in and update next_in and avail_in
    accordingly. If not all input can be processed (because there is not
    enough room in the output buffer), next_in and avail_in are updated and
    processing will resume at this point for the next call of zd_deflate().

  - Provide more output starting at next_out and update next_out and avail_out
    accordingly. This action is forced if the parameter flush is non zero.
    Forcing flush frequently degrades the compression ratio, so this parameter
    should be set only when necessary (in interactive applications).
    Some output may be provided even if flush is not set.

  Before the call of zd_deflate(), the application should ensure that at least
  one of the actions is possible, by providing more input and/or consuming
  more output, and updating avail_in or avail_out accordingly; avail_out
  should never be zero before the call. The application can consume the
  compressed output when it wants, for example when the output buffer is full
  (avail_out == 0), or after each call of zd_deflate(). If deflate returns 
  ZD_OK and with zero avail_out, it must be called again after making room in 
  the output buffer because there might be more output pending.

    If the parameter flush is set to ZD_SYNC_FLUSH, all pending output is
  flushed to the output buffer and the output is aligned on a byte boundary, so
  that the decompressor can get all input data available so far. (In particular
  avail_in is zero after the call if enough output space has been provided
  before the call.)  Flushing may degrade compression for some compression
  algorithms and so it should be used only when necessary. 

    If flush is set to ZD_FULL_FLUSH, all output is flushed as with
  ZD_SYNC_FLUSH, and the compression state is reset so that decompression can
  restart from this point if previous compressed data has been damaged or if
  random access is desired. Using ZD_FULL_FLUSH too often can seriously degrade
  the compression.

    If zd_deflate returns with avail_out == 0, this function must be called 
  again with the same value of the flush parameter and more output space 
  (updated avail_out), until the flush is complete (deflate returns with 
  non-zero avail_out).

    If the parameter flush is set to ZD_FINISH, pending input is processed,
  pending output is flushed and deflate returns with ZD_STREAM_END if there
  was enough output space; if deflate returns with ZD_OK, this function must be
  called again with ZD_FINISH and more output space (updated avail_out) but no
  more input data, until it returns with ZD_STREAM_END or an error. After
  deflate has returned ZD_STREAM_END, the only possible operations on the
  stream are zd_deflateReset or zd_deflateEnd.
  
    ZD_FINISH can be used immediately after zd_deflateInit if all the 
  compression is to be done in a single step. In this case, 
  a recommended value for avail_out is at least 0.1% larger than avail_in plus
  12 bytes - on some rare occasions zdelta may expand the data more than this,
  the application should check the return codes for those cases!
  If deflate does not return  ZD_STREAM_END, then it must be called 
  again as described above.

    zd_deflate() sets strm->adler to the adler32 checksum of all input read
  so far (that is, total_in bytes).

     zd_deflate() returns ZD_OK if some progress has been made (more input
  processed or more output produced), ZD_STREAM_END if all input has been
  consumed and all output has been produced (only when flush is set to
  ZD_FINISH), ZD_STREAM_ERROR if the stream state was inconsistent (for example
  if next_in or next_out was NULL), ZD_BUF_ERROR if no progress is possible
  (for example avail_in or avail_out was zero).
*/


ZEXTERN int ZEXPORT zd_deflateEnd OF((zd_streamp strm));
/*
     All dynamically allocated data structures for this stream are freed.
   This function discards any unprocessed input and does not flush any
   pending output.

     zd_deflateEnd returns ZD_OK if success, ZD_STREAM_ERROR if the
   stream state was inconsistent, ZD_DATA_ERROR if the stream was freed
   prematurely (some input or output was discarded). In the error case,
   msg may be set but then points to a static string (which must not be
   deallocated).
*/


/* 
ZEXTERN int ZEXPORT inflateInit OF((z_streamp strm));

     Initializes the internal stream state for decompression. The fields
   next_in, avail_in, zalloc, zfree and opaque must be initialized before by
   the caller. If next_in is not ZD_NULL and avail_in is large enough (the exact
   value depends on the compression method), zd_inflateInit determines the
   compression method from the zdlib header and allocates all data structures
   accordingly; otherwise the allocation will be deferred to the first call of
   inflate.  If zalloc and zfree are set to ZD_NULL, zd_inflateInit updates them to
   use default allocation functions.

     inflateInit returns ZD_OK if success, ZD_MEM_ERROR if there was not enough
   memory, ZD_VERSION_ERROR if the zdelta library version is incompatible with the
   version assumed by the caller.  msg is set to null if there is no error
   message. inflateInit does not perform any decompression apart from reading
   the zdlib header if present: this will be done by zd_inflate().  (So next_in and
   avail_in may be modified, but next_out and avail_out are unchanged.)
*/



ZEXTERN int ZEXPORT zd_inflate OF((zd_streamp strm, int flush));
/*
    zd_inflate decompresses as much data as possible, and stops when the input
  buffer becomes empty or the output buffer becomes full. It may some
  introduce some output latency (reading input without producing any output)
  except when forced to flush.

  The detailed semantics are as follows. zd_inflate performs one or both of the
  following actions:

  - Decompress more input starting at next_in and update next_in and avail_in
    accordingly. If not all input can be processed (because there is not
    enough room in the output buffer), next_in is updated and processing
    will resume at this point for the next call of zd_inflate().

  - Provide more output starting at next_out and update next_out and avail_out
    accordingly.  zd_inflate() provides as much output as possible, until there
    is no more input data or no more space in the output buffer (see below
    about the flush parameter).

  Before the call of zd_inflate(), the application should ensure that at least
  one of the actions is possible, by providing more input and/or consuming
  more output, and updating the next_* and avail_* values accordingly.
  The application can consume the uncompressed output when it wants, for
  example when the output buffer is full (avail_out == 0), or after each
  call of zd_inflate(). If inflate returns ZD_OK and with zero avail_out, it
  must be called again after making room in the output buffer because there
  might be more output pending. 

    If the parameter flush is set to ZD_SYNC_FLUSH, inflate flushes as much
  output as possible to the output buffer. The flushing behavior of inflate is
  not specified for values of the flush parameter other than ZD_SYNC_FLUSH
  and ZD_FINISH, but the current implementation actually flushes as much output
  as possible anyway.

    zd_inflate() should normally be called until it returns ZD_STREAM_END or an
  error. However if all decompression is to be performed in a single step
  (a single call of inflate), the parameter flush should be set to
  ZD_FINISH. In this case all pending input is processed and all pending
  output is flushed; avail_out must be large enough to hold all the
  uncompressed data. (The size of the uncompressed data may have been saved
  by the compressor for this purpose.) The next operation on this stream must
  be inflateEnd to deallocate the decompression state. The use of ZD_FINISH
  is never required, but can be used to inform inflate that a faster routine
  may be used for the single zd_inflate() call.
  
    zd_inflate() returns ZD_OK if some progress has been made (more input 
  processed or more output produced), ZD_STREAM_END if the end of the 
  compressed data has been reached and all uncompressed output has been 
  produced, ZD_DATA_ERROR if the input data was corrupted (input stream not 
  conforming to the zlib/libzd format or incorrect adler32 checksum), 
  ZD_STREAM_ERROR if the stream structure was inconsistent (for example if 
  next_in or next_out was NULL), ZD_MEM_ERROR if there was not enough memory,
  ZD_BUF_ERROR if no progress is possible or if there was not enough room in
  the output buffer when ZD_FINISH is used.

  NOTE: there is no check sum for the reference data! 
*/


ZEXTERN int ZEXPORT zd_inflateEnd OF((zd_streamp strm));
/*
     All dynamically allocated data structures for this stream are freed.
   This function discards any unprocessed input and does not flush any
   pending output.

     zd_inflateEnd returns ZD_OK if success, ZD_STREAM_ERROR if the stream 
   state was inconsistent. In the error case, msg may be set but then points 
   to a static string (which must not be deallocated).
*/

/* ==========================================================================
 * advanced functions  
 */  

ZEXTERN int ZEXPORT zd_deflateReset OF((zd_streamp strm));
/*
     This function is equivalent to zd_deflateEnd followed by zd_deflateInit,
   but does not free and reallocate all the internal compression state.
   The stream will keep the same compression level and any other attributes
   that may have been set by zd_deflateInit2.

      deflateReset returns ZD_OK if success, or ZD_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being NULL).
*/

ZEXTERN int ZEXPORT zd_deflateParams OF((zd_streamp strm,
					 int level,
					 int strategy));
/*
     Dynamically update the compression level and compression strategy.  The
   interpretation of level and strategy is as in zd_deflateInit2.  This can be
   used to switch between compression and straight copy of the input data, or
   to switch to a different kind of input data requiring a different
   strategy. If the compression level is changed, the input available so far
   is compressed with the old level (and may be flushed); the new level will
   take effect only at the next call of zd_deflate().

     Before the call of zd_deflateParams, the stream state must be set as for
   a call of zd_deflate(), since the currently available input may have to
   be compressed and flushed. In particular, strm->avail_out must be non-zero.

     zd_deflateParams returns ZD_OK if success, ZD_STREAM_ERROR if the source
   stream state was inconsistent or if a parameter was invalid, ZD_BUF_ERROR
   if strm->avail_out was zero.
*/

ZEXTERN int ZEXPORT zd_inflateSync OF((zd_streamp strm));
/* 
    Skips invalid compressed data until a full flush point (see above the
  description of deflate with ZD_FULL_FLUSH) can be found, or until all
  available input is skipped. No output is provided.

    zd_inflateSync returns ZD_OK if a full flush point has been found, 
  ZD_BUF_ERROR if no more input was provided, ZD_DATA_ERROR if no flush point 
  has been found, or ZD_STREAM_ERROR if the stream structure was inconsistent. 
  In the success case, the application may save the current current value of 
  total_in which indicates where valid compressed data was found. In the error
  case, the application may repeatedly call zd_inflateSync, providing more 
  input each time, until success or end of the input data.
*/

ZEXTERN int ZEXPORT zd_inflateReset OF((zd_streamp strm));
/*
     This function is equivalent to zd_inflateEnd followed by zd_inflateInit,
   but does not free and reallocate all the internal decompression state.
   The stream will keep attributes that may have been set by zd_inflateInit2.

      inflateReset returns ZD_OK if success, or ZD_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being NULL).
*/

/* ==========================================================================
 * utility functions  
 */

/* computes zdelta difference between target data and reference data
 *
 * INPUT:
 * ref      pointer to reference data set
 * rsize    size of reference data set
 * tar      pointer to targeted data set
 * tsize    size of targeted data set
 * delta    pointer to delta buffer
 *	    the delta buffer IS allocated by the user
 * *dsize   size of delta buffer
 *
 *
 * OUTPUT parameters:
 * delta    pointer to zdelta difference 
 * *dsize   size of zdelta difference
 *
 * zd_compress returns ZD_OK on success,
 * ZD_MEM_ERROR if there was not enough memory,
 * ZD_BUF_ERROR if there was not enough room in the output
 * buffer.
 */
ZEXTERN int ZEXPORT zd_compress OF ((const Bytef *ref, uLong rsize,
				     const Bytef *tar, uLong tsize,
				     Bytef *delta, uLongf* dsize));



/* computes zdelta difference between target data and reference data
 *
 * INPUT:
 * ref      pointer to reference data set
 * rsize    size of reference data set
 * tar      pointer to targeted data set
 * tsize    size of targeted data set
 * delta    a pointer to NULL pointer
 *	    DO NOT allocate any memory for the output here
 *	    this will be done by libZD 
 *
 * OUTPUT parameters:
 * *delta   points to zdelta difference 
 *	    the memory space IS allocated by zdelta
 * *dsize   size ot zdelta difference
 *	    the memory space for this variable 
 *	    SHOULD be allocated by the user
 *
 * zd_compress1 returns ZD_OK on success
 * in case of failure an error code is returned, and
 * an error message is printed to stderr, delta is set to NULL
 */
ZEXTERN int ZEXPORT zd_compress1 OF ((const Bytef *ref, uLong rsize,
				      const Bytef *tar, uLong tsize,
				      Bytef **delta, uLongf *dsize));


/* computes zdelta difference between target data and N reference data sets
 *
 * INPUT:
 * ref      pointer to *N* reference data set
 * rsize    sizes of the *N* reference data set
 * rw       number of the reference data sets (*N*)
 *
 * tar      pointer to targeted data set
 * tsize    size of targeted data set
 * delta    a pointer to NULL pointer
 *	    DO NOT allocate any memory for the output here
 *	    this will be done by libZD 
 *
 * OUTPUT parameters:
 * delta    points to zdelta difference 
 *	    the memory space IS allocated by the user
 * *dsize   size ot zdelta difference
 *	    the memory space for this variable 
 *	    SHOULD be allocated by the user
 *
 * zd_compress1 returns ZD_OK on success
 * in case of failure an error code is returned, and
 * an error message is printed to stderr, delta is set to NULL
 */
ZEXTERN int ZEXPORT zd_compressN OF ((const Bytef *ref[],uLong rsize[],int rw,
				      const Bytef *tar, uLong tsize,
				      Bytef *delta, uLongf *dsize));




/* rebuilds target data from reference data and zdelta difference
 *
 * INPUT:
 * ref      pointer to reference data set
 * rsize    size of reference data set
 * tar      pointer to target buffer
 *          this buffer IS allocated by the user
 * tsize    size of target buffer
 * delta    pointer to zdelta difference
 * dsize    size of zdelta difference
 *
 *
 * OUTPUT parameters:
 * tar      pointer to recomputed target data
 * *tsize   size of recomputed target data
 *
 * zd_uncompress returns ZD_OK on success,
 * ZD_MEM_ERROR if there was not enough memory,
 * ZD_BUF_ERROR if there was not enough room in the output
 * buffer.
 */
ZEXTERN int ZEXPORT zd_uncompress OF ((const Bytef *ref, uLong rsize,
				       Bytef *tar, uLongf *tsize,
				       const Bytef *delta, uLong dsize));


/* rebuilds target data from reference data and zdelta difference
 *
 * INPUT:
 * ref      pointer to reference data set
 * rsize    size of reference data set
 * tar      pointer to NULL poiner
 *	    DO NOT allocate any memory for the output here;
 *	    this will be done by libZD 
 * 
 * delta    pointer to zdelta diference
 * dsize    size of zdelta difference
 *
 * OUTPUT parameters:
 * *tar     pointer to the rebuild target data 
 *	    the memory space IS allocated by zdelta
 * tsize    size ot the recomputed target
 *	    the memory space for this variable 
 *	    SHOULD be allocated by the user
 *
 * zd_uncompress1 returns ZD_OK on success
 * in case of failure an error code is returned, and
 * an error message is printed to the stderr, tar is set to NULL
 */
ZEXTERN int ZEXPORT zd_uncompress1 OF ((const Bytef *ref, uLong rsize,
					Bytef **tar, uLongf *tsize,
					const Bytef *delta, uLong dsize));



/* rebuilds target data from reference data and zdelta difference 
 *
 * INPUT:
 * ref      pointer to *N* reference data set
 * rsize    sizes of the *N* reference data set
 * rw       number of the reference data sets (*N*)
 * tar      pointer to target buffer
 *          this buffer IS allocated by the user
 * tsize    size of target buffer
 * delta    pointer to zdelta difference
 * dsize    size of zdelta difference
 *
 *
 * OUTPUT parameters:
 * tar      pointer to recomputed target data
 * *tsize   size of recomputed target data
 *
 * zd_uncompress returns ZD_OK on success,
 * ZD_MEM_ERROR if there was not enough memory,
 * ZD_BUF_ERROR if there was not enough room in the output
 * buffer.
 */
ZEXTERN int ZEXPORT 
zd_uncompressN OF((Bytef *ref[],uLong rsize[],int rw,
		   const Bytef *tar, uLong *tsize,
		   const Bytef *delta, uLongf dsize));
  


/* computes zdelta difference between target data and N reference data sets
 *
 * INPUT:
 * ref      pointer to *N* reference data set
 * rsize    sizes of the *N* reference data set
 * rw       number of the reference data sets (*N*)
 *
 * tar      pointer to targeted data set
 * tsize    size of targeted data set
 * delta    a pointer to NULL pointer
 *	    DO NOT allocate any memory for the output here
 *	    this will be done by libZD 
 *
 * OUTPUT parameters:
 * *delta   points to zdelta difference 
 *	    the memory space IS allocated by zdelta
 * *dsize   size ot zdelta difference
 *	    the memory space for this variable 
 *	    SHOULD be allocated by the user
 *
 * zd_compress1 returns ZD_OK on success
 * in case of failure an error code is returned, and
 * an error message is printed to stderr, delta is set to NULL
 */
ZEXTERN int ZEXPORT zd_compressN1 OF ((const Bytef *ref[],uLong rsize[],int rw,
				       const Bytef *tar, uLong tsize,
				       Bytef **delta, uLongf *dsize));



/* rebuilds target data from reference data and zdelta difference
 *
 * INPUT:
 * ref      pointer to *N* reference data set
 * rsize    sizes of the *N* reference data set
 * rw       number of the reference data sets (*N*)
 *
 * tar      pointer to NULL poiner
 *	    DO NOT allocate any memory for the output here;
 *	    this will be done by libZD 
 * 
 * delta    pointer to zdelta diference
 * dsize    size of zdelta difference
 *
 * OUTPUT parameters:
 * *tar     pointer to the rebuild target data 
 *	    the memory space IS allocated by zdelta
 * tsize    size ot the recomputed target
 *	    the memory space for this variable 
 *	    SHOULD be allocated by the user
 *
 * zd_uncompress1 returns ZD_OK on success
 * in case of failure an error code is returned, and
 * an error message is printed to the stderr, tar is set to NULL
 */
  ZEXTERN int ZEXPORT 
  zd_uncompressN1 OF ((const Bytef **ref, uLong* rsize,int rw,
		       Bytef **tar, uLongf *tsize,
		       const Bytef *delta, uLong dsize));



  /*ZEXTERN int ZEXPORT zd_inflateSync OF((zd_streamp strm)); */
/* 
    Skips invalid compressed data until a full flush point (see above the
  description of deflate with Z_FULL_FLUSH) can be found, or until all
  available input is skipped. No output is provided.

    zd_inflateSync returns ZD_OK if a full flush point has been found, 
  ZD_BUF_ERROR if no more input was provided, ZD_DATA_ERROR if no flush 
  point has been found, or ZD_STREAM_ERROR if the stream structure was 
  inconsistent. In the success case, the application may save the current 
  value of total_in which indicates where valid compressed data was found. 
  In the error case, the application may repeatedly call zd_inflateSync, 
  providing more input each time, until success or end of the input data.
*/


/* ==========================================================================
 * checksum functions  
 */  
ZEXTERN uLong ZEXPORT zd_adler32 OF((uLong adler, const Bytef *buf, uInt len));

/*
     Update a running Adler-32 checksum with the bytes buf[0..len-1] and
   return the updated checksum. If buf is NULL, this function returns
   the required initial value for the checksum.
   An Adler-32 checksum is almost as reliable as a CRC32 but can be computed
   much faster. Usage example:

   uLong adler = adler32(0L, ZD_NULL, 0);

   while (read_buffer(buffer, length) != EOF) {
     adler = adler32(adler, buffer, length);
   }
   if (adler != original_adler) error();
*/


/* ==========================================================================
 */
                        /* various hacks, don't look :) */

/* zd_deflateInit and zd_inflateInit are macros to allow checking the libzd
 * version and the compiler's view of zd_stream:
 */
ZEXTERN int ZEXPORT zd_deflateInit_ OF((zd_streamp strm, int level,
					const char *version, int stream_size));
ZEXTERN int ZEXPORT zd_inflateInit_ OF((zd_streamp strm,
					const char *version, int stream_size));
ZEXTERN int ZEXPORT zd_deflateInit2_ OF((zd_streamp strm, int  level, 
					 int  method, int windowBits,
					 int memLevel, int strategy, 
					 const char *version,int stream_size));
ZEXTERN int ZEXPORT zd_inflateInit2_ OF((zd_streamp strm, int  windowBits,
					const char *version, int stream_size));

#define zd_deflateInit(strm, level) \
        zd_deflateInit_((strm), (level),       ZDLIB_VERSION, sizeof(zd_stream))
#define zd_inflateInit(strm) \
        zd_inflateInit_((strm),                ZDLIB_VERSION, sizeof(zd_stream))
#define zd_deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
        zd_deflateInit2_((strm),(level),(method),(windowBits),(memLevel),\
                      (strategy),           ZDLIB_VERSION, sizeof(zd_stream))
#define zd_inflateInit2(strm, windowBits) \
        zd_inflateInit2_((strm), (windowBits), ZDLIB_VERSION, sizeof(zd_stream))

#if !defined(_ZD_UTIL_H) && !defined(ZD_NO_DUMMY_DECL)
    struct zd_internal_state {int dummy;}; /* hack for buggy compilers */
#endif


#ifdef __cplusplus
}
#endif

#endif /* ZD_LIBZD_H */

