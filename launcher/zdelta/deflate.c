/* deflate.c -- compress data using the deflation algorithm
 * Copyright (C) 1995-1998 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
 
   file modified by Dimitre Trendafilov (2003)
 */

/*
 *  zdelta: the algorithm description is slightly changed from the original
 *  in zlib in order to match better zdelta deflation process.
 *
 *  ALGORITHM
 *
 *      The "deflation" process depends on being able to identify portions
 *      of the input text which are identical to earlier input (within a
 *      sliding window trailing behind the input currently being processed),
 *      or to reference data (within a separate sliding window, which movement
 *      is based on weighted average of the most recently used matches).
 *
 *      The most straightforward technique turns out to be the fastest for
 *      most input files: try all possible matches and select the longest one
 *      (if there is more than one matches with such length, a chose the match
 *      resulting in the smallest match offset).
 *      The key feature of this algorithm is that insertions into the string
 *      dictionaries (one for the already processed input, and one for the 
 *      reference data) are very simple and thus fast, and deletions are 
 *      avoided completely. Insertions are performed at each input character,
 *      whereas  string matches are performed only when the previous match 
 *      ends. So it is preferable to spend more time in matches to allow very 
 *      fast string insertions and avoid deletions. The matching algorithm for
 *      small strings is inspired from that of Rabin & Karp. A brute force 
 *      approach is used to find longer strings when a small match has been 
 *      found. A similar algorithm is used in comic (by Jan-Mark Wams) and 
 *      freeze (by Leonid Broukhis).
 *
 *  ACKNOWLEDGEMENTS
 *
 *      The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
 *      I found it in 'freeze' written by Leonid Broukhis.
 *      Thanks to many people for bug reports and testing.
 *
 *  REFERENCES
 *
 *      Deutsch, L.P.,"DEFLATE Compressed Data Format Specification".
 *      Available in ftp://ds.internic.net/rfc/rfc1951.txt
 *
 *      A description of the Rabin and Karp algorithm is given in the book
 *         "Algorithms" by R. Sedgewick, Addison-Wesley, p252.
 *
 *      Fiala,E.R., and Greene,D.H.
 *         Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595
 *
 */

/* zdelta:
 *
 * modified: 
 *          deflate()         -- added prefix zd
 *          deflateEnd()      -- added prefix zd
 *          deflateInit2_()   -- added prefix zd
 *          deflateReset()    -- added prefix zd
 *          lm_init()
 *          longest_match()
 *          deflate_stored()
 * added:
 *          delta_deflate_fast()
 *          delta_deflate_slow()
 *          fill_ref_window()
 *          init_ref_window()
 *          init_window()
 *          read_ref_buf()
 *          reference_longest_match()
 * removed:
 *          check_match()
 *          deflate_copy()
 *          deflate_fast()
 *          deflate_slow()
 *          deflateSetDictionary()
 */

/* @(#) $Id$ */

#include "deflate.h"
#include <limits.h>



const char zd_deflate_copyright[] =
" deflate 1.1.3 Copyright 1995-1998 Jean-loup Gailly ";
/*
  zdelta: zd prefix added.
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
*/


/* ===========================================================================
 *  Function prototypes.
 */
typedef enum {
  need_more,      /* block not completed, need more input or more output */
  block_done,     /* block flush performed */
  finish_started, /* finish started, need only more output at next deflate */
  finish_done     /* finish done, accept no more input or output */
} block_state;

typedef block_state (*compress_func) OF((deflate_state *s, int flush));
/* Compression function. Returns the block state after the call. */

local void fill_window     OF((deflate_state *s));
local void fill_ref_window OF((deflate_state *s, int rw)); /* zdelta: added  */
local void lm_init         OF((deflate_state *s)); /* zdelta: modified */
local void init_window     OF((deflate_state *s)); /* zdelta: added    */
local void init_ref_window OF((deflate_state *s, int rw)); /* zdelta: added  */

local block_state deflate_stored OF((deflate_state *s, int flush));
/* zdelta: added    */
local block_state delta_deflate_fast OF((deflate_state *s, int flush));
local block_state delta_deflate_slow OF((deflate_state *s, int flush));

local void putShortMSB    OF((deflate_state *s, uInt b));
local void flush_pending  OF((zd_streamp strm));

local int read_buf        OF((zd_streamp strm, Bytef *buf, unsigned size));
/* zdelta: added    */
local int read_ref_buf OF((zd_streamp strm, Bytef *buf, unsigned size, int rw));

local void target_longest_match     OF((deflate_state *s, IPos cur_match));
local void reference_longest_match  OF((deflate_state *s, 
					IPos cur_match, int rw));


/* ===========================================================================
 * Local data
 */

#define NIL 0
/* Tail of hash chains */

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
typedef struct config_s {
   ush good_length; /* reduce search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_length; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

local const config configuration_table[10] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */

/* 1 */ {64,  64, 128,   4, delta_deflate_fast}, /* maximum speed .. */
/* 2 */ {64,  80, 256,   8, delta_deflate_fast}, /* .. no lazy matches */
/* 3 */ {64,  96, 512,  32, delta_deflate_fast},

/* 4 */ {64,   64,  256,   16, delta_deflate_slow},  /* lazy matches */
/* 5 */ {128, 256,  512,   32, delta_deflate_slow},
/* 6 */ {128, 256, 2048,  128, delta_deflate_slow},
/* 7 */ {128, 512, 2048,  256, delta_deflate_slow},
/* 8 */ {256,2048, 4098, 1024, delta_deflate_slow},
/* 9 */ {256,4098, 4098, 4096, delta_deflate_slow}}; /* maximum compression */


/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

#define EQUAL 0
/* result of memcmp for equal strings */

/* zdelta: additional MACRO definitions */
#define ZD_DISTANCE(x,y) ((x)>(y)?(x)-(y):(y)-(x))

/*         reference sliding window      */
#define ZD_TIME_TO_CHECK  2
#define ZD_HIST_SIZE      1000
#define ZD_GET_NEXT_POS(ind)(++ind == ZD_HIST_SIZE?0:ind)

struct static_tree_desc_s {int dummy;}; /* for buggy compilers */

/* zdelta: no changes in hash tables; completely reuse zlib code; 
 *         add MACROs for the reference hash table
 */
/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.
 */
#define UPDATE_HASH(s,h,c) (h = (((h)<<s->hash_shift) ^ (c)) & s->hash_mask)

/* ===========================================================================
 * Insert string str in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * If this file is compiled with -DFASTEST, the compression level is forced
 * to 1, and no hash chains are maintained.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first MIN_MATCH bytes of str are valid
 *    (except for the last MIN_MATCH-1 bytes of the input file).
 */
#ifdef FASTEST
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_head = s->head[s->ins_h], \
    s->head[s->ins_h] = (Pos)(str))
#else
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    s->prev[(str) & s->w_mask] = match_head = s->head[s->ins_h], \
    s->head[s->ins_h] = (Pos)(str))
#endif


/* zdelta: added   */
#define INSERT_REF_STRING(s, str, rw) \
   (UPDATE_HASH(s, s->ref_h, s->ref_window[rw][(str) + (MIN_MATCH-1)]), \
    s->ref_prev[rw][((Pos) str) & s->rw_mask] = s->ref_head[rw][s->ref_h], \
    s->ref_head[rw][s->ref_h] = (Pos)(str))


/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#define CLEAR_HASH(s) \
    s->head[s->hash_size-1] = NIL; \
    zmemzero((Bytef *)s->head, (unsigned)(s->hash_size-1)*sizeof(*s->head));

#define CLEAR_REF_HASH(s, rw) \
    s->ref_head[rw][s->hash_size-1] = NIL; \
    zmemzero((Bytef *)s->ref_head[rw], \
             (unsigned)(s->hash_size-1)*sizeof(*s->ref_head[rw]));

/* ========================================================================= */
/* zdelta: modified
 *         added zd prefix to the name
 */
int ZEXPORT zd_deflateReset (strm)
    zd_streamp strm;
{
  deflate_state *s;
  int rw;

  if (strm == ZD_NULL || strm->state == ZD_NULL || 
      strm->zalloc == ZD_NULL || strm->zfree == ZD_NULL ||
      strm->refnum < 0 || strm->refnum > REFNUM) return ZD_STREAM_ERROR;

  strm->total_in = strm->total_out = 0;
  for(rw=0;rw<strm->refnum;++rw) strm->base_out[rw] = 0;

  strm->msg = ZD_NULL; /* use zfree if we ever allocate msg dynamically */
  strm->data_type = Z_UNKNOWN;

  s = (deflate_state *)strm->state;
  s->pending = 0;
  s->pending_out = s->pending_buf;

  if (s->noheader < 0) {
    s->noheader = 0; /* was set to -1 by deflate(..., ZD_FINISH); */
  }
  s->status = s->noheader ? BUSY_STATE : INIT_STATE;
  strm->adler = 1;
  s->last_flush = Z_NO_FLUSH;

  zd_tr_init(s);
  lm_init(s); 

  /* zdelta: initialize the reference hash tables here
   *         the first ref_window_size bytes are inserted 
   *         into the reference dictionary 
   */
  for(rw=0;rw<strm->refnum;++rw){
    if(strm->base_avail[rw] >= MIN_MATCH)  init_ref_window(s,rw);
  }

  /* zdelta: initialize the target hash table here;
   *         this will save us sliding an empty hash table!
   *         no insertions are done
   */
  
  if (strm->avail_in >= MIN_MATCH) 
    init_window(s);
  
  return ZD_OK;
}

/* ========================================================================= */
/* zdelta: modified
 *         added zd prefix to the name
 */
int ZEXPORT zd_deflateParams(strm, level, strategy)
    zd_streamp strm;
    int level;
    int strategy;
{
    deflate_state *s;
    compress_func func;
    int err = ZD_OK;

    if (strm == ZD_NULL || strm->state == ZD_NULL ||
	strm->refnum < 0 || strm->refnum > REFNUM) return ZD_STREAM_ERROR;
    s = strm->state;

    if (level == ZD_DEFAULT_COMPRESSION) {
	level = 7;
    }
    if (level < 0 || level > 9 || strategy < 0 || strategy > 8) {
	return ZD_STREAM_ERROR;
    }
    func = configuration_table[s->level].func;

    if (func != configuration_table[level].func && strm->total_in != 0) {
	/* Flush the last buffer: */
	err = zd_deflate(strm, Z_PARTIAL_FLUSH);
    }
    if (s->level != level) {
	s->level = level;
	s->max_lazy_match   = configuration_table[level].max_lazy;
	s->good_match       = configuration_table[level].good_length;
	s->nice_match       = configuration_table[level].nice_length;
	s->max_chain_length = configuration_table[level].max_chain;
    }
    s->strategy = strategy;
    return err;
}

/* ========================================================================= */
int ZEXPORT zd_deflateInit_(strm, level, version, stream_size)
    zd_streamp strm;
    int level;
    const char *version;
    int stream_size;
{
  return zd_deflateInit2_(strm, level, ZD_DEFLATED, MAX_WBITS, 
			  DEF_MEM_LEVEL, ZD_DEFAULT_STRATEGY, version, 
			  stream_size);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
/* zdelta: modified
 *         added zd prefix to the name
 */
int ZEXPORT zd_deflateInit2_(strm, level, method, windowBits, memLevel, 
			     strategy, version, stream_size)
    zd_streamp strm;
    int  level;
    int  method;
    int  windowBits;
    int  memLevel;
    int  strategy;
    const char *version;
    int stream_size;
{
  deflate_state *s;
  int noheader = 0;
  int refnum = strm->refnum;
  static const char* zd_version = ZDLIB_VERSION;
  int i;
  
  ushf *overlay;
  /* We overlay pending_buf and d_buf+l_buf. This works since the average
   * output size for (length,distance) codes is <= 24 bits.
   */
  
  if (version == ZD_NULL || version[0] != zd_version[0] ||
      stream_size != sizeof(zd_stream)) {
    return ZD_VERSION_ERROR;
  }
  if (strm == ZD_NULL) return ZD_STREAM_ERROR;

  strm->msg = ZD_NULL;
  if (strm->zalloc == ZD_NULL) {
    strm->zalloc = zd_zcalloc;
    strm->opaque = (voidpf)0;
  }
  if (strm->zfree == ZD_NULL) strm->zfree = zd_zcfree;

  if (level == ZD_DEFAULT_COMPRESSION) level = 7;

#ifdef FASTEST
  level = 1;
#endif

  if (windowBits < 0) { /* undocumented feature: suppress zdlib header */
    noheader = 1;
    windowBits = -windowBits;
  }

#ifdef NO_ERROR_CHECK /* suppress zdlib header at compile time */
  noheader = 1;
#endif

  if (memLevel < 1 || memLevel > MAX_MEM_LEVEL || method != ZD_DEFLATED ||
      windowBits < 8 || windowBits > 15 || level < 0 || level > 9 ||
      strategy < ZD_HUFFMAN_ONLY || strategy > 8 || 
      refnum < 0 || refnum > REFNUM)
  {
    return ZD_STREAM_ERROR;
  }

  s = (deflate_state *) ZALLOC(strm, 1, sizeof(deflate_state));
  if (s == ZD_NULL) return ZD_MEM_ERROR;

  strm->state = (struct zd_internal_state FAR *)s;
  s->strm = strm;

  s->noheader = noheader;
  s->w_bits = windowBits;
  s->w_size = 1 << s->w_bits;
  s->w_mask = s->w_size - 1;

  s->hash_bits  = memLevel + 7;
  s->hash_size  = 1 << s->hash_bits;
  s->hash_mask  = s->hash_size - 1;
  s->hash_shift =  ((s->hash_bits+MIN_MATCH-1)/MIN_MATCH);

  /* zdelta: reference window mask */
  s->rw_mask = (s->w_size << 1) - 1;

  /* zdelta: needed for the correctness of reference_longest_match() */
  s->ref_window[0]=(Bytef *) ZALLOC(strm, s->w_size*2*(refnum+1),sizeof(Byte));
  s->window       =(Bytef *) (s->ref_window[0]+(2*refnum*s->w_size)); 

  s->prev       = (Posf *)  ZALLOC(strm, s->w_size, sizeof(Pos));  
  s->head       = (Posf *)  ZALLOC(strm, s->hash_size, sizeof(Pos));

  /* zdelta: reference data hash table */ 
  s->ref_prev[0] = (Posf *)  ZALLOC(strm, s->w_size*(2*refnum), sizeof(Pos));  
  s->ref_head[0] = (Posf *)  ZALLOC(strm, s->hash_size*(refnum), sizeof(Pos));

  s->lit_bufsize = 1 << (memLevel + 6); /* 16K elements by default */

  overlay = (ushf *) ZALLOC(strm, s->lit_bufsize, 2*sizeof(ush)+2);
  s->pending_buf = (uchf *) overlay;
  s->pending_buf_size = (ulg)s->lit_bufsize * (sizeof(ush)+2L);

  if (s->ref_window  == ZD_NULL || s->prev        == ZD_NULL ||
      s->head        == ZD_NULL || s->ref_prev    == ZD_NULL ||
      s->ref_head    == ZD_NULL || s->pending_buf == ZD_NULL)
  {
    strm->msg = (char*)ERR_MSG(ZD_MEM_ERROR);
    zd_deflateEnd (strm);
    return ZD_MEM_ERROR;
  }

  s->d_buf = overlay + s->lit_bufsize/sizeof(ush);
  s->l_buf = s->d_buf + s->lit_bufsize;
  s->z_buf = s->pending_buf + (1+2*sizeof(ush))*s->lit_bufsize;

  /* zdelta: initialize reference window buffers */
  for(i=1;i<refnum;++i){
    s->ref_window[i] = (Bytef *) (s->ref_window[0] +(s->w_size)*2*i);
    s->ref_prev[i]   = (Posf *)  (s->ref_prev[0] + (s->w_size)*2*i);
    s->ref_head[i]   = (Posf *)  (s->ref_head[0] + i*(s->hash_size));
  }

  /* zdelta: these variables are not used; kept for future use */
  s->level = level;
  s->strategy = strategy;
  s->method = (Byte)method;

  if(zd_deflateReset(strm)!=ZD_OK){
    return ZD_STREAM_ERROR;
  };

  return ZD_OK;
}

/* =========================================================================
 * Put a short in the pending buffer. The 16-bit value is put in MSB order.
 * IN assertion: the stream state is correct and there is enough room in
 * pending_buf.
 */
local void putShortMSB (s, b)
     deflate_state *s;
     uInt b;
{
  put_byte(s, (Byte)(b >> 8));
  put_byte(s, (Byte)(b & 0xff));
}   

/* =========================================================================
 * Flush as much pending output as possible. All deflate() output goes
 * through this function so some applications may wish to modify it
 * to avoid allocating a large strm->next_out buffer and copying into it.
 * (See also read_buf()).
 */
local void flush_pending(strm)
     zd_streamp strm;
{
  unsigned len = strm->state->pending;
  
  if (len > strm->avail_out) len = strm->avail_out;
  if (len == 0) return;
  
  zmemcpy(strm->next_out, strm->state->pending_out, len);
  strm->next_out  += len;
  strm->state->pending_out  += len;
  strm->total_out += len;
  strm->avail_out  -= len;
  strm->state->pending -= len;
  if (strm->state->pending == 0) {
    strm->state->pending_out = strm->state->pending_buf;
  }
}

/* ========================================================================= */
/* zdelta: modified
 *         added prefix zd to the name
 *         no code for preset dictionaries
 */
int ZEXPORT zd_deflate (strm, flush)
    zd_streamp strm;
    int flush;
{
  int old_flush; /* value of flush param for previous deflate call */
  deflate_state *s;

  if (strm == ZD_NULL || strm->state == ZD_NULL ||
      flush > ZD_FINISH || flush < 0) {
    return ZD_STREAM_ERROR;
  }
  s = strm->state;

  if (strm->next_out == ZD_NULL ||
      (strm->next_in == ZD_NULL && strm->avail_in != 0) ||
      (s->status == FINISH_STATE && flush != ZD_FINISH) ||
      strm->refnum<0 || strm->refnum>REFNUM) {
    ERR_RETURN(strm, ZD_STREAM_ERROR);
  }
  if (strm->avail_out == 0) ERR_RETURN(strm, ZD_BUF_ERROR);

  s->strm = strm; /* just in case */
  old_flush = s->last_flush;
  s->last_flush = flush;

  /* Write the libzd header */
  if (s->status == INIT_STATE) {

    uInt header = (ZD_DEFLATED + ((s->w_bits-8)<<4)) << 8;
    uInt level_flags = (s->level-1) >> 1;

    if (level_flags > 3) level_flags = 3;
    header |= (level_flags << 6);
    header += 31 - (header % 31);
    
    s->status = BUSY_STATE;
    putShortMSB(s, header);
  }

  /* Flush as much pending output as possible */
  if (s->pending != 0) {
    flush_pending(strm);
    if (strm->avail_out == 0) {
      /* Since avail_out is 0, deflate will be called again with
       * more output space, but possibly with both pending and
       * avail_in equal to zero. There won't be anything to do,
       * but this is not an error situation so make sure we
       * return OK instead of BUF_ERROR at next call of deflate:
       */
      s->last_flush = -1;
      return ZD_OK;
    }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with ZD_FINISH, we keep
     * returning ZD_STREAM_END instead of ZD_BUFF_ERROR.
     */
  } else if (strm->avail_in == 0 && flush <= old_flush &&
	     flush != ZD_FINISH) {
    ERR_RETURN(strm, ZD_BUF_ERROR);
  }

  /* User must not provide more input after the first FINISH: */
  if (s->status == FINISH_STATE && strm->avail_in != 0) {
    ERR_RETURN(strm, ZD_BUF_ERROR);
  }

  /* Start a new block or continue the current one.
   */
  if (strm->avail_in != 0 || s->lookahead != 0 ||
      (flush != Z_NO_FLUSH && s->status != FINISH_STATE)) {
    block_state bstate;
    
    /* zdelta: currently only one level of compression */
    bstate = (*(configuration_table[s->level].func))(s, flush);
   
    if (bstate == finish_started || bstate == finish_done) {
      s->status = FINISH_STATE;
    }
    if (bstate == need_more || bstate == finish_started) {
      if (strm->avail_out == 0) {
	s->last_flush = -1; /* avoid BUF_ERROR next call, see above */
      }
      return ZD_OK;
      /* If flush != Z_NO_FLUSH && avail_out == 0, the next call
       * of deflate should use the same flush parameter to make sure
       * that the flush is complete. So we don't have to output an
       * empty block here, this will be done at next call. This also
       * ensures that for a very small output buffer, we emit at most
       * one empty block.
       */
    }
    if (bstate == block_done) {
      if (flush == Z_PARTIAL_FLUSH) {
	zd_tr_align(s);
      } else { /* FULL_FLUSH or SYNC_FLUSH */
	zd_tr_stored_block(s, (char*)0, 0L, 0);
	/* For a full flush, this empty block will be recognized
	 * as a special marker by inflate_sync().
	 */
	if (flush == ZD_FULL_FLUSH) {
	  CLEAR_HASH(s);             /* forget history */
	}
      }
      flush_pending(strm);
      if (strm->avail_out == 0) {
	s->last_flush = -1; /* avoid BUF_ERROR at next call, see above */
	return ZD_OK;
      }
    }
  }

  Assert(strm->avail_out > 0, "bug2");

  if (flush != ZD_FINISH) return ZD_OK;
  if (s->noheader) return ZD_STREAM_END;

  /* Write the libzd trailer (adler32) */
  putShortMSB(s, (uInt)(strm->adler >> 16));
  putShortMSB(s, (uInt)(strm->adler & 0xffff));
  flush_pending(strm);
  /* If avail_out is zero, the application will call deflate again
   * to flush the rest.
   */
  s->noheader = -1; /* write the trailer only once! */
  return s->pending != 0 ? ZD_OK : ZD_STREAM_END;
}

/* ========================================================================= */
/* zdelta: modified
 *         added prefix zd to the name
 *         modified to handle multiple reference files
 */
int ZEXPORT zd_deflateEnd (strm)
    zd_streamp strm;
{
  int status;

  if (strm == ZD_NULL || strm->state == ZD_NULL) return ZD_STREAM_ERROR;
  
  status = strm->state->status;
  if (status != INIT_STATE && status != BUSY_STATE &&
      status != FINISH_STATE) {
    return ZD_STREAM_ERROR;
  }

  /* Deallocate in reverse order of allocations: */
  /* zdelta: deallocate zdelta buffers           */
  TRY_FREE(strm, strm->state->pending_buf);
  TRY_FREE(strm, strm->state->ref_head[0]);
  TRY_FREE(strm, strm->state->ref_prev[0]);
  TRY_FREE(strm, strm->state->head);
  TRY_FREE(strm, strm->state->prev);

  /* zdelta: strm->state->window now points at the middle of
   *         strm->state->ref_window
   */
  TRY_FREE(strm, strm->state->ref_window[0]);
  ZFREE(strm, strm->state);
  strm->state = ZD_NULL;
  return status == BUSY_STATE ? ZD_DATA_ERROR : ZD_OK;
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local int read_buf(strm, buf, size)
     zd_streamp strm;
     Bytef *buf;
     unsigned size;
{
  unsigned len = strm->avail_in;

  if (len > size) len = size;
  if (len == 0) return 0;

  strm->avail_in  -= len;

  if (!strm->state->noheader) {
    strm->adler = zd_adler32(strm->adler, strm->next_in, len);
  }

  zmemcpy(buf, strm->next_in, len);
  strm->next_in  += len;
  strm->total_in += len;
  
  return (int)len;
}

/* ===========================================================================
 * zdelta:added
 * Read new data from the reference input stream;
 * updates the total number of bytes read. 
 */
local int read_ref_buf(strm, buf, size, rw)
     zd_streamp strm;
     Bytef *buf;
     unsigned size;
     int rw;
{
  unsigned len = strm->base_avail[rw]; 

  if (len > size) len = size;
  if (len == 0) return 0;
  strm->base_avail[rw]  -= len;

  zmemcpy(buf, strm->base[rw], len);

  strm->base[rw]     += len;
  strm->base_out[rw] += len;

  return (int)len;
}

/* ===========================================================================
 * zdelta:modified
 *        only one compression level
 *        Initialize the "longest match" routines for a new zlib stream
 */
local void lm_init (s)
    deflate_state *s;
{
  int i;
  int refnum = s->strm->refnum;
  s->window_size = (ulg)2L*s->w_size;
  /*s->ref_window_size = (ulg)2L*s->w_size;*/

  s->strstart = 0;
  s->block_start = 0L;
  s->lookahead = 0;
  s->match_length = s->prev_length = MIN_MATCH-1;
  s->match_available = 0;
  s->ins_h = 0;
  s->match_benefit   = s->prev_benefit = 0;

  /* Set the default configuration parameters:
   */
  s->max_lazy_match   = configuration_table[s->level].max_lazy;
  s->good_match       = configuration_table[s->level].good_length;
  s->max_chain_length = configuration_table[s->level].max_chain;
  s->nice_match       = configuration_table[s->level].nice_length;

  /* zdelta: initialize reference data pointers */
  for(i=0; i<refnum*ZD_RPN; ++i){ 
    s->rwptr[i] = s->rwptr_save[i] = 0;
    s->stable[i] = s->stable_save[i] = 0;
  }
  for(i=0; i<refnum; ++i){
    s->stored_allowed[i] = 1;
    s->ref_window_size[i]  = (ulg)2L*s->w_size;
  }

}

/* ===========================================================================
 * Fill the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead.
 *
 * IN assertion: lookahead < MIN_LOOKAHEAD
 * OUT assertions: strstart <= window_size-MIN_LOOKAHEAD
 *    At least one byte has been read, or avail_in == 0; reads are
 *    performed for at least two bytes (required for the zip translate_eol
 *    option -- not supported here).
 */
local void 
fill_window(s)
    deflate_state *s;
{
  register unsigned n, m;
  register Posf *p;
  unsigned more;    /* Amount of free space at the end of the window. */
  uInt wsize = s->w_size;

  do {
    more = (unsigned)(s->window_size -(ulg)s->lookahead -(ulg)s->strstart);

    /* Deal with !@#$% 64K limit: */
    if (more == 0 && s->strstart == 0 && s->lookahead == 0) {
      more = wsize;
      
    } else if (more == (unsigned)(-1)) {
      /* Very unlikely, but possible on 16 bit machine if strstart == 0
       * and lookahead == 1 (input done one byte at time)
       */
      more--;
	    
      /* If the window is almost full and there is insufficient lookahead,
       * move the upper half to the lower one to make room in the upper half.
       */
    } else if (s->strstart >= wsize+MAX_DIST(s)) {

      zmemcpy(s->window, s->window+wsize, (unsigned)wsize);
      /* the match may be in the refference window; dont change it always */
      if(s->match_ptr == ZD_TARP) s->match_start -= wsize;
      s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
      s->block_start -= (long) wsize;

      /* Slide the hash table (could be avoided with 32 bit values
	 at the expense of memory usage). We slide even when level == 0
	 to keep the hash table consistent if we switch back to level > 0
	 later. (Using level 0 permanently is not an optimal usage of
	 zlib, so we don't care about this pathological case.)
      */
      n = s->hash_size;
      p = &s->head[n];
      do {
	m = *--p;
	*p = (Pos)(m >= wsize ? m-wsize : NIL);
      } while (--n);

      n = wsize;
#ifndef FASTEST
      p = &s->prev[n];
      do {
	m = *--p;
	*p = (Pos)(m >= wsize ? m-wsize : NIL);
	/* If n is not on any hash chain, prev[n] is garbage but
	 * its value will never be used.
	 */
      } while (--n);
#endif
      more += wsize;
    }
    if (s->strm->avail_in == 0) return;

    /* If there was no sliding:
     *    strstart <= WSIZE+MAX_DIST-1 && lookahead <= MIN_LOOKAHEAD - 1 &&
     *    more == window_size - lookahead - strstart
     * => more >= window_size - (MIN_LOOKAHEAD-1 + WSIZE + MAX_DIST-1)
     * => more >= window_size - 2*WSIZE + 2
     * In the BIG_MEM or MMAP case (not yet supported),
     *   window_size == input_size + MIN_LOOKAHEAD  &&
     *   strstart + s->lookahead <= input_size => more >= MIN_LOOKAHEAD.
     * Otherwise, window_size == 2*WSIZE so more >= 2.
     * If there was sliding, more >= WSIZE. So in all cases, more >= 2.
     */
    Assert(more >= 2, "more < 2");
    
    n = read_buf(s->strm, s->window + s->strstart + s->lookahead, more);
    s->lookahead += n;
    
    /* Initialize the hash value now that we have some input: */
    if (s->lookahead >= MIN_MATCH) {
      s->ins_h = s->window[s->strstart];
      UPDATE_HASH(s, s->ins_h, s->window[s->strstart+1]);
#if MIN_MATCH != 3
      Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
        }
    /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
     * but this is not important since only literal bytes will be emitted.
     */
    
  } while (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_in != 0);
}

/* ===========================================================================
 * zdelta: added
 * fill reference window when too many matches are found at the end
 * of the current reference buffer (see delta_deflate for more details)
 *
 * OUT assertions:
 *    At least one byte has been read, or base_avail == 0;
 */
local void 
fill_ref_window(s, rw)
    deflate_state *s;
    int rw;
{
  uInt i;
  uInt wsize = s->w_size;
  uInt nb;


  /* zdelta: slide the window always by wsize  */
  zmemcpy((Bytef*) s->ref_window[rw], 
	  (Bytef*) (s->ref_window[rw]+wsize), 
	  (unsigned)wsize*sizeof(Bytef));
  nb = read_ref_buf(s->strm,s->ref_window[rw]+wsize, wsize, rw);
  
  /* zdelta: set the real ref_window_size      */
  s->ref_window_size[rw] = wsize+nb;
 
  /* zdelta: update reference window pointers  */
  /* if a given pointer is too far from the window, 
   * it will never be used again; stop updating it
   * TODO: needs better solution for this
   */ 
  if(s->rwptr[2*rw]  > (- ZD_UNREACHABLE)) s->rwptr[2*rw]   -=wsize;
  if(s->rwptr[2*rw+1]> (- ZD_UNREACHABLE)) s->rwptr[2*rw+1] -=wsize;
  
  /* zdelta: flush reference dictionary 	 */
  CLEAR_REF_HASH(s, rw);
  s->ref_h = s->ref_window[rw][0];
  /* zdelta: and rebuild the dictionary from the beginning
   * same speed if slided as the target trailing window
   * better compression since no hashes are lost
   */
  UPDATE_HASH(s, s->ref_h, s->ref_window[rw][1]);
  for(i = 0;i <= s->ref_window_size[rw] - MIN_MATCH; i++){
    INSERT_REF_STRING(s,i,rw);
  }
  s->stored_allowed[rw] = 0;
}

/* zdelta:added
 * called only once at the beginnig of deflatation
 */
local void 
init_ref_window(s, rw)
    deflate_state *s;
    int rw;
{
    uInt i;
    uInt n;

    n = read_ref_buf(s->strm, s->ref_window[rw], s->ref_window_size[rw], rw);
    s->ref_window_size[rw] = n;
    /* clear memory */
    CLEAR_REF_HASH(s,rw);	
    
    s->ref_h = s->ref_window[rw][0];
    UPDATE_HASH(s, s->ref_h, s->ref_window[rw][1]);

    for(i = 0;i <= n - MIN_MATCH ;i++){
        INSERT_REF_STRING(s, i, rw);
    }
}

/* ===========================================================================
 * zdelta:added
 * Initialize the target data window
 * This will save us the overhead of updating an empty hash table at the 
 * first call of deflate
 * Updates lookahead and ins_h.
 *
 * IN assertion: called before the first execution of deflate()
 * OUT assertions: strstart <= window_size-MIN_LOOKAHEAD
 *    At least one byte has been read, or avail_in == 0; 
 */
local void 
init_window(s)
    deflate_state *s;
{

    s->lookahead = read_buf(s->strm, s->window, (unsigned) s->window_size);

    /* zdelta: Initialize the hash value now that we have some input: */
    if (s->lookahead >= MIN_MATCH) {
        s->ins_h = s->window[s->strstart];
        UPDATE_HASH(s, s->ins_h, s->window[s->strstart+1]);
#if MIN_MATCH != 3
            Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
    }

    /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
     * but this is not important since only literal bytes will be emitted.
     */
}

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 *
 * zdelta: added zd prefix to the _tr_flush_block
 */
#define FLUSH_BLOCK_ONLY(s, eof) { \
   zd_tr_flush_block(s, (s->block_start >= 0L ? \
                   (charf *)&s->window[(unsigned)s->block_start] : \
                   (charf *)ZD_NULL), \
		(ulg)((long)s->strstart - s->block_start), \
		(eof)); \
   s->block_start = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#define FLUSH_BLOCK(s, eof) { \
   FLUSH_BLOCK_ONLY(s, eof); \
   if (s->strm->avail_out == 0) return (eof) ? finish_started : need_more; \
}


/* Approximate benefit given by a match with length len at a distance dist */
#define GetBenefit(len,dist) {  \
  int log = 0;                  \
  uInt d = dist;                \
  while(d>>=1 > 0) log++;       \
  benefit = (strategy*len - 11 - log); \
}


/* ===========================================================================
 * zdelta: added
 * locates the best match in the reference data
 *
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 *   at least one of the reference pointers has positive value
 * OUT assertion: the match length is not greater than s->lookahead.
 *   if a better match (longer than the s->match_length or with length 
 *   s->match_lenght but distance smaller than s->match_distance) is found
 *   s->match_length, s->match_distance, s->match_ptr, 
 *   s->match_sign, s->match_ptr 
 *   are set to pointet to it
 */

local void reference_longest_match(s, cur_match, rw)
     deflate_state *s;
     IPos cur_match;
     int rw;
{
  register Bytef *scan = s->window + s->strstart; /* current string */
  register Bytef *match;                          /* matched string */
  register uInt len;                              /* length of current match */
  register int  benefit;
  register int  strategy = s->strategy; /* expected length for literal codes */
  /* max hash chain length, we want to search in the reference data
   * no longer than in the target data
   */
  register uInt chain_length = s->max_chain_length;

  uInt cur_distance;	       /* the shortest distance to the current match */
  int  cur_ptr;  

  long min;                    /* value of the smaller ref pointer */ 
  long max;                    /* value of the greater ref pointer */ 
  long mid;                   
 
  int min_ptr;
  int max_ptr;

  uInt best_len      = s->match_length;   /* best match length so far   */
  uInt best_distance = s->match_distance; /* best match distance so far */
  int best_benefit   = s->match_benefit;

  uInt nice_match    = s->nice_match;     /* stop if match long enough */
  int best_ptr       = NO_PTR;
  Pos best_match     = 0;

  Pos limit;
    
  Pos *prev  = s->ref_prev[rw];
  uInt wmask = s->rw_mask;
  ulg  wsize =  s->ref_window_size[rw];  

  register Bytef *strend   = s->window + s->strstart + MAX_MATCH;
  register Byte scan_end1  = scan[best_len-1];

  /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
   * It is easy to get rid of this optimization if necessary.
   */

  /* set the search boundaries in the reference data window */
  if(s->rwptr[2*rw] < s->rwptr[2*rw+1]){
    min = s->rwptr[2*rw], max = s->rwptr[2*rw+1];
    min_ptr = 2*rw, max_ptr = 2*rw+1;
  }
  else{
    min = s->rwptr[2*rw+1], max = s->rwptr[2*rw];
    min_ptr = 2*rw+1, max_ptr = 2*rw;
  }
  mid = min >= 0 ? (max + min) / 2 : 0;
  
  limit = min >  ZD_UNREACHABLE ? (Pos) (min - ZD_UNREACHABLE) : NIL;
  while(cur_match > (IPos) max + ZD_UNREACHABLE) 
    cur_match = prev[cur_match & wmask];
  if(cur_match == NIL) return;
  
  /* Do not waste too much time if we already have a good match: */
  if (best_len >= s->good_match) {
    chain_length >>= 2;
  }
  /* Do not look for matches beyond the end of the input. This is necessary
   * to make deflate deterministic.
   */
  if ((uInt)nice_match > s->lookahead) nice_match = s->lookahead;

  do {
    match = s->ref_window[rw] + cur_match; 
    
    /* Skip to next match if the match length cannot increase
     * or if the match length is less than 2:
     */
    if ( match[best_len-1] != scan_end1 ||
	 *match            != *scan     ||
	 *++match          != scan[1])      continue;
    
    /* check if the potential match is accessible */
    if((IPos) mid <= cur_match){
      cur_ptr = max_ptr;
      cur_distance = ZD_DISTANCE((IPos)max,cur_match);
    }
    else{ /* min_ptr is closer */
      cur_ptr = min_ptr;
      cur_distance = ZD_DISTANCE((IPos)min,cur_match);
    }
    if(cur_distance >= ZD_UNREACHABLE) continue;
    
    /* The check at best_len-1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    scan += 2, match++;
    Assert(*scan == *match, "ref. match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart+258.
     */
    do {
    } while (*++scan == *++match && *++scan == *++match &&
	     *++scan == *++match && *++scan == *++match &&
	     *++scan == *++match && *++scan == *++match &&
	     *++scan == *++match && *++scan == *++match &&
	     scan < strend);
    
    Assert(scan <= s->window+(unsigned)(s->window_size-1), "wild scan");

    len     = MAX_MATCH - (int)(strend - scan);
    scan    = strend - MAX_MATCH;

    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    if (len > s->lookahead) { len = s->lookahead; }
    if (len > wsize - cur_match) { len = wsize - cur_match; }
    GetBenefit(len, cur_distance);

    if( (benefit > best_benefit) || \
	(benefit == best_benefit && cur_distance < best_distance) ){

      best_match    = cur_match;
      best_distance = cur_distance; 
      best_ptr      = cur_ptr;
      best_len      = len;
      best_benefit  = benefit;
      if (len >= nice_match) break;
      scan_end1     = scan[best_len-1];

    }
    
  } while ((cur_match = prev[cur_match & wmask]) > limit &&
	   --chain_length);
  
  if(best_ptr != NO_PTR){
    s->match_ptr      = best_ptr;
    s->match_length   = best_len;
    s->match_distance = best_distance;
    s->match_start    = best_match;
    s->match_sign     = (best_match < s->rwptr[best_ptr] ? ZD_MINUS:ZD_PLUS);
    s->match_benefit  = best_benefit;
  }
}


/* 
 * zdelta: modified
 *         changed name from longest_match to target_longest_match
 * locates the best available match in the target data
 *
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 *   at least one of the reference pointers has positive value
 * OUT assertion: the match length is not greater than s->lookahead.
 *   if a better match (longer than the s->match_length or with length 
 *   s->match_lenght but distance smaller than s->match_distance) is found
 *   s->match_length, s->match_distance, s->match_ptr, 
 *   s->match_sign, s->match_ptr 
 *   are set to pointet to it
 */
local void target_longest_match(s, cur_match)
     deflate_state *s;
     IPos cur_match;
{
  register Bytef *scan = s->window + s->strstart;/* current string */
  register Bytef *match;                         /* matched string */
  register uInt len;                             /* length of current match */
  register uInt chain_length = s->max_chain_length;/* max hash chain length */
  uInt nice_match            = s->nice_match;    /*stop if match long enough */
  register int benefit;
  register int strategy = s->strategy; /* expected length for literal codes */
  uInt best_len      = s->match_length;
  uInt best_distance = s->match_distance;
  int best_benefit   = s->match_benefit;
  IPos best_match = 0;
  
  uInt cur_distance ;	       /* the shortest distance to the current match */

  IPos limit;
  Posf *prev = s->prev;
  uInt wmask = s->w_mask;
    
  register Bytef *strend = s->window + s->strstart + MAX_MATCH;
  register Byte scan_end1  = scan[best_len-1];
    
  Assert((ulg)s->strstart <= s->window_size-MIN_LOOKAHEAD, "need lookahead");
  Assert(cur_match < s->strstart, "no future");

  limit = s->strstart > (IPos)ZD_UNREACHABLE ? 
    s->strstart - ((IPos) ZD_UNREACHABLE) : NIL;
    
  /* Do not waste too much time if we already have a good match: */
  if (best_len >= s->good_match) {
    chain_length >>= 2;
  }
  /* Do not look for matches beyond the end of the input. This is necessary
   * to make deflate deterministic.
   */
  if ((uInt)nice_match > s->lookahead) nice_match = s->lookahead;

  do {
    match = s->window + cur_match;
    /* Skip to next match if the match length cannot increase
     * or if the match length is less than 2:
     */
    
    if (match[best_len-1] != scan_end1 ||	
	*match            != *scan     ||
	*++match          != scan[1])      continue;
      
    /* The check at best_len-1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    scan += 2, match++;
    Assert(*scan == *match, "match[2]?");


    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart+258.
     */
    do {
    } while (*++scan == *++match && *++scan == *++match &&
	     *++scan == *++match && *++scan == *++match &&
	     *++scan == *++match && *++scan == *++match &&
	     *++scan == *++match && *++scan == *++match &&
	     scan < strend);
      
    Assert(scan <= s->window+(unsigned)(s->window_size-1), "wild scan");
    
    len = MAX_MATCH - (int)(strend - scan);
    if (len > s->lookahead) { len = s->lookahead; }
    scan = strend - MAX_MATCH;
 
    cur_distance = s->strstart - cur_match;
    GetBenefit(len, cur_distance);

    if (benefit > best_benefit || \
	(benefit == best_benefit && cur_distance < best_distance)) {

      best_match    = cur_match;
      best_distance = cur_distance;
      best_benefit   = benefit;

      if ((uInt)len >= nice_match){
	best_len=nice_match;
	break;
      } 
      best_len = len;
      scan_end1  = scan[best_len-1];	
    }
  } while ((cur_match = prev[cur_match & wmask]) > limit &&
	   --chain_length);

  if(best_benefit != s->match_benefit || best_distance != s->match_distance){
    s->match_ptr      = ZD_TARP;
    s->match_length   = best_len;
    s->match_distance = best_distance;
    s->match_sign     = ZD_MINUS;
    s->match_start    = best_match;
    s->match_benefit  = best_benefit;
  }
}


/* ===========================================================================
 * Copy without compression as much as possible from the input stream, return
 * the current block state.
 * This function does not insert new strings in the dictionary since
 * uncompressible data is probably not useful. This function is used
 * only for the level=0 compression option.
 * NOTE: this function should be optimized to avoid extra copying from
 * window to pending_buf.
 */
local block_state deflate_stored(s, flush)
    deflate_state *s;
    int flush;
{
    /* Stored blocks are limited to 0xffff bytes, pending_buf is limited
     * to pending_buf_size, and each stored block has a 5 byte header:
     */
    ulg max_block_size = 0xffff;
    ulg max_start;

    if (max_block_size > s->pending_buf_size - 5) {
        max_block_size = s->pending_buf_size - 5;
    }

    /* Copy as much as possible from input to output: */
    for (;;) {
        /* Fill the window as much as possible: */
        if (s->lookahead <= 1) {

            Assert(s->strstart < s->w_size+MAX_DIST(s) ||
		   s->block_start >= (long)s->w_size, "slide too late");

            fill_window(s);
            if (s->lookahead == 0 && flush == Z_NO_FLUSH) return need_more;

            if (s->lookahead == 0) break; /* flush the current block */
        }
	Assert(s->block_start >= 0L, "block gone");

	s->strstart += s->lookahead;
	s->lookahead = 0;

	/* Emit a stored block if pending_buf will be full: */
 	max_start = s->block_start + max_block_size;
        if (s->strstart == 0 || (ulg)s->strstart >= max_start) {
	    /* strstart == 0 is possible when wraparound on 16-bit machine */
	    s->lookahead = (uInt)(s->strstart - max_start);
	    s->strstart = (uInt)max_start;
            FLUSH_BLOCK(s, 0);
	}
	/* Flush if we may have to slide, otherwise block_start may become
         * negative and the data will be gone:
         */
        if (s->strstart - (uInt)s->block_start >= MAX_DIST(s)) {
            FLUSH_BLOCK(s, 0);
	}
    }
    FLUSH_BLOCK(s, flush == ZD_FINISH);
    return flush == ZD_FINISH ? finish_done : block_done;
}


#define UPDATE_REF_POINTERS(ptr, pos, len, dist) \
if(dist>ZD_FAR){                                 \
   if(s->stable[ptr])                            \
     s->stable[ptr] = 0;                         \
   else                                          \
     s->rwptr[ptr] = pos + len;                  \
}                                                \
else{                                            \
  s->stable[ptr] = 1;                            \
  s->rwptr[ptr] = pos + len;                     \
}                                                              
 

typedef struct pointer_history_s {
  ulg len_hist[REFNUM][ZD_HIST_SIZE];
  ulg ptr_hist[REFNUM][ZD_HIST_SIZE];
  ulg sum_len[REFNUM];
  ulg sum_ptr[REFNUM];

  /* ulg ptr_val, avg_pos; */
  ulg  avg_pos;
  int front_ptr[REFNUM];
  int back_ptr[REFNUM];
  int ch_flag[REFNUM];
} pointer_history;


local void slide_ref_window(deflate_state *s, pointer_history *h, 
			    int rw, int len, int match_start){
  ulg ptr_val;
  h->sum_len[rw] += len;       /* cumm. copy length */
  ptr_val = (s->ref_window_size[rw] - match_start - MAX_MATCH) * len;
  h->sum_ptr[rw] += ptr_val;              /* pointer average */
  h->len_hist[rw][h->front_ptr[rw]] = len;
  h->ptr_hist[rw][h->front_ptr[rw]] = ptr_val;
  h->front_ptr[rw] = ZD_GET_NEXT_POS(h->front_ptr[rw]);
  
  if ((h->front_ptr[rw] == h->back_ptr[rw]) || (h->sum_len[rw] > 5000)){
    h->sum_len[rw] -= h->len_hist[rw][h->back_ptr[rw]];
    h->sum_ptr[rw] -= h->ptr_hist[rw][h->back_ptr[rw]];
    h->back_ptr[rw] = ZD_GET_NEXT_POS(h->back_ptr[rw]);
  }
  
  /* check if base window should be slided*/
  if((++(h->ch_flag[rw])) == ZD_TIME_TO_CHECK){
    h->ch_flag[rw] = 0;
    h->avg_pos = h->sum_ptr[rw] / h->sum_len[rw];
    
    if((h->avg_pos < 22000) && (h->sum_len[rw] > 2000) && 
       (s->rwptr[2*rw]>(int)s->w_size ||
	s->rwptr[2*rw+1]>(int)s->w_size)) 
    { 
      fill_ref_window(s, rw);         /* slide */
      h->sum_ptr[rw]   = 0;
      h->sum_len[rw]   = 0;
      h->front_ptr[rw] = 0;
      h->back_ptr[rw]  = 0;
    }
  }
}




local block_state delta_deflate_fast(deflate_state *s, int flush){
  IPos hash_head = NIL;
  int bflush;           /* set if current block must be flushed */
  zd_streamp strm = s->strm;
  int refnum = strm->refnum;
  int rw; 

  /* data structures containing statistics for the sliding reference window */
  pointer_history ph;

  for(rw=0; rw<refnum; ++rw){
    ph.sum_len[rw]   = ph.sum_ptr[rw]  = 0;
    ph.front_ptr[rw] = ph.back_ptr[rw] = ph.ch_flag[rw] = 0;
  }
  
  for (;;){

    if (s->lookahead < MIN_LOOKAHEAD) {
      fill_window(s);
      if (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
	return need_more;
      }
      if (s->lookahead == 0) break; /* flush the current block */
    }
    /*
    s->match_length    = MIN_MATCH-1;
    s->match_distance  = ZD_UNREACHABLE;
    s->match_benefit   = 0;
    */
    if (s->lookahead >= MIN_MATCH && s->strategy!=ZD_HUFFMAN_ONLY) {
      INSERT_STRING(s, s->strstart, hash_head);
    
      /* Find the match giving the greates benefit */
      /* search in the reference data    */
      for(rw=0; rw<refnum; ++rw){
	if (s->ref_head[rw][s->ins_h] != NIL && 
	    s->match_length < s->max_lazy_match) 
	{
	  /* To simplify the code, we prevent matches with the string
	   * of window index 0 (in particular we have to avoid a match
	   * of the string with itself at the start of the input file).
	   */
	  reference_longest_match (s, s->ref_head[rw][s->ins_h], rw);
	  /* reference_longest_match() sets match_start, match_ptr, match_sign,
	   * match_length, match_distance
	   */
	}
      }
      /* search in the target data       */
      if (hash_head != NIL &&  s->match_length < s->max_lazy_match && 
	  s->strstart - hash_head <= ZD_UNREACHABLE) {
	/* To simplify the code, we prevent matches with the string
	 * of window index 0 (in particular we have to avoid a match
	 * of the string with itself at the start of the input file).
	 */
	target_longest_match (s, hash_head);
	/* target_longest_match() sets s->match_start, s->match_ptr,
	 * s->match_sign, s->match_length,  s->match_distance
	 */
      }
    }
    /* write the match
     * the minimum length is MIN_MATCH, so we can substract it and we'll
     * still have a positive number
     */
    if(s->match_benefit>0){ 
      /* update best_ptr, but only if it is a base copy */
      if(s->match_ptr != ZD_TARP){
	rw = s->match_ptr/2;
	/* output reference window match */
	_tr_tally_dist_ref(s, s->match_distance, s->match_length-3,
			   ZCODE(s->match_sign,s->match_ptr), bflush);
	/* update reference pointers */
	UPDATE_REF_POINTERS(s->match_ptr, s->match_start, 
			    s->match_length, s->match_distance);
 
	/* slide reference data here */
	if(strm->base_avail[rw] > 0) 
	  slide_ref_window(s, &ph, rw, s->match_length, s->match_start);
      }
      else{
	/* output target match */
	_tr_tally_dist_tar(s, s->match_distance, s->match_length-3, bflush);
      }

      s->lookahead -= s->match_length;

      /* Insert new strings in the hash table only if the match length
       * is not too large. This saves time but degrades compression.
       */
      if (s->match_length <= s->max_insert_length &&
	  s->lookahead >= MIN_MATCH) {
	s->match_length--;
	do {
	  s->strstart++;
	  INSERT_STRING(s, s->strstart, hash_head);
	} while (--s->match_length != 0);
	s->strstart++;   
	s->match_distance = ZD_UNREACHABLE;
	s->match_benefit  = 0;
      }
      else{
	s->strstart+=s->match_length;
	s->match_length   = MIN_MATCH-1;
	s->match_distance = ZD_UNREACHABLE;
	s->match_benefit  = 0;
	s->ins_h = s->window[s->strstart];
	UPDATE_HASH(s, s->ins_h, s->window[s->strstart+1]);
#if MIN_MATCH != 3
	Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
       /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
	* matter since it will be recomputed at next deflate call.
	*/
      }
    }
    else{
      _tr_tally_lit(s, s->window[s->strstart], bflush);
      s->strstart++;
      s->lookahead--;
    }
    if (bflush) FLUSH_BLOCK(s, 0);
  }
  
  FLUSH_BLOCK(s, flush == ZD_FINISH);
  return flush == ZD_FINISH ? finish_done : block_done;
}



local block_state delta_deflate_slow(deflate_state *s, int flush){

  IPos hash_head = NIL;
  int bflush;           /* set if current block must be flushed */
  zd_streamp strm = s->strm;
  int refnum = strm->refnum;
  int rw; 
  /* data structures containing statistics for the sliding reference window */
  pointer_history ph;

  for(rw=0; rw<refnum; ++rw){
    ph.sum_len[rw]   = ph.sum_ptr[rw]  = 0;
    ph.front_ptr[rw] = ph.back_ptr[rw] = ph.ch_flag[rw] = 0;
  }

  for (;;){

    if (s->lookahead < MIN_LOOKAHEAD) {
      fill_window(s);
      if (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
	return need_more;
      }
      if (s->lookahead == 0) break; /* flush the current block */
    }
    
    s->prev_length    = s->match_length;
    s->prev_match     = s->match_start;
    s->prev_ptr       = s->match_ptr;
    s->prev_sign      = s->match_sign;
    s->prev_distance  = s->match_distance;
    s->prev_benefit   = s->match_benefit;
    s->match_length   = MIN_MATCH - 1;
    s->match_distance = ZD_UNREACHABLE;
    s->match_benefit  = 0;
    if (s->lookahead >= MIN_MATCH && s->strategy!=ZD_HUFFMAN_ONLY) {
      INSERT_STRING(s, s->strstart, hash_head);
    
      /* Find the match giving the greates benefit */
      /* search in the reference data    */
      for(rw=0; rw<refnum; ++rw){
	if (s->ref_head[rw][s->ins_h] != NIL && 
	    s->match_length < s->max_lazy_match) 
	{
	  /* To simplify the code, we prevent matches with the string
	   * of window index 0 (in particular we have to avoid a match
	   * of the string with itself at the start of the input file).
	   */
	  reference_longest_match (s, s->ref_head[rw][s->ins_h], rw);
	  /* reference_longest_match() sets match_start, match_ptr, match_sign,
	   * match_length, match_distance
	   */
	}
      }
      /* search in the target data       */
      if (hash_head != NIL &&  s->match_length < s->max_lazy_match && 
	  s->strstart - hash_head <= ZD_UNREACHABLE) {
	/* To simplify the code, we prevent matches with the string
	 * of window index 0 (in particular we have to avoid a match
	 * of the string with itself at the start of the input file).
	 */
	target_longest_match (s, hash_head);
	/* target_longest_match() sets s->match_start, s->match_ptr,
	 * s->match_sign, s->match_length,  s->match_distance
	 */
      }
    }
    /* write the match
     * the minimum length is MIN_MATCH, so we can substract it and we'll
     * still have a positive number
     */
    if(s->prev_benefit>0 && s->prev_benefit>=s->match_benefit){ 
      /* the previous match is better than the current */

      uInt max_insert = s->strstart + s->lookahead - MIN_MATCH;
      /* Do not insert strings in hash table beyond this. */

      /* update best_ptr, but only if it is a base copy */
      if(s->prev_ptr != ZD_TARP){

	rw = s->prev_ptr/2;
	/* output reference window match */
	_tr_tally_dist_ref(s, s->prev_distance, s->prev_length-3,
			   ZCODE(s->prev_sign,s->prev_ptr), bflush);
	/* update reference pointers */
	UPDATE_REF_POINTERS(s->prev_ptr, s->prev_match, 
			    s->prev_length, s->prev_distance);

	/* slide reference data here */
	if(strm->base_avail[rw] > 0) 
	  slide_ref_window(s, &ph, rw, s->prev_length, s->prev_match);
      }
      else{
	/* output target match */
	_tr_tally_dist_tar(s, s->prev_distance, s->prev_length-3, bflush);
      }
      /* Insert in hash table all strings up to the end of the match.
       * strstart-1 and strstart are already inserted. If there is not
       * enough lookahead, the last two strings are not inserted in
       * the hash table.
       */
      s->lookahead -= s->prev_length-1;
      s->prev_length -= 2;
      do {
	if (++s->strstart <= max_insert) {
	  INSERT_STRING(s, s->strstart, hash_head);
	}
      } while (--s->prev_length != 0);
      s->match_available = 0;
      s->match_length   = MIN_MATCH-1;
      s->match_distance = ZD_UNREACHABLE;
      s->match_benefit  = 0;
      s->strstart++; 
      if (bflush) FLUSH_BLOCK(s, 0);
    }
    else if(s->match_available){
      /* If there was no match at the previous position, output a
       * single literal. If there was a match but the current match
       * is longer, truncate the previous match to a single literal.
       */
      _tr_tally_lit(s, s->window[s->strstart-1], bflush);
      if (bflush) {
	FLUSH_BLOCK_ONLY(s, 0);
      }
      s->strstart++;
      s->lookahead--;
      if (s->strm->avail_out == 0) return need_more;
    }
    else{
      /* There is no previous match to compare with, wait for
       * the next step to decide.
       */
      s->match_available = 1;
      s->strstart++;
      s->lookahead--;
    }
  }

  if (s->match_available) {
    _tr_tally_lit(s, s->window[s->strstart-1], bflush);
    s->match_available = 0;
  }
  
  FLUSH_BLOCK(s, flush == ZD_FINISH);
  return flush == ZD_FINISH ? finish_done : block_done;
}
