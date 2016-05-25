/* deflate.h -- internal compression state
 * Copyright (C) 1995-1998 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zdlib.h.
  
   file modified by Dimitre Trendafilov (2003)
 */

/* @(#) $Id$ */

/* zdelta:
 *
 * modified: 
 *       struct zd_internal_state - added structures to handle reference window
 *        _tr_tally_dist() - added extra zdelta code
 * added:
 *          --
 * removed:
 *          --
 */

#ifndef _DEFLATE_H
#define _DEFLATE_H

#include "zutil.h"
/* ===========================================================================
 * Internal compression state.
 */

#define LITERALS  256
/* number of literal bytes 0..255 */

#define DIST_CODES 30
/* number of distance codes, not countint the special END_OF_BLOCK code */

#define LENGTH_CODES 37
/* number of length codes in one length code Huffman tree */

#define L_NUM   3 
/* number of length Huffman trees */

#define L_CODES (L_NUM*LENGTH_CODES)
/* number of Length codes */

#if REFNUM > 1 || GEN_TREES_H || BUILDFIXED
#define Z_CODES 8
#else
#define Z_CODES 2
#endif

#define D_CODES   (LITERALS+1+DIST_CODES)
/* number of Literal or Distance codes, including the END_OF_BLOCK code */

#define BL_CODES  19
/* number of codes used to transfer the bit lengths */

#define DIST_CODE_LEN  512 /* see definition of array dist_code below */
#define LENGTH_CODE_LEN 321 /* see definition of array zd_length_code below */

#define HEAP_SIZE (2*D_CODES+1)
/* maximum heap size */

#define MAX_BITS 15
/* All codes must not exceed MAX_BITS bits */

#define INIT_STATE    42
#define BUSY_STATE   113
#define FINISH_STATE 666

/* examine only the first ZD_XXX_CHAIN_LEN elements of a hash chain */

#define LITERAL      64  /* zdelta flag for literals */
#define TARGET       32  /* zdelta flag for target matches */

/* Stream status */


/* Data structure describing a single value and its code string. */
typedef struct ct_data_s {
  union {
    ush  freq;       /* frequency count */
    ush  code;       /* bit string */
  } fc;
  union {
    ush  dad;        /* father node in Huffman tree */
    ush  len;        /* length of bit string */
  } dl;
} FAR ct_data;

#define Freq fc.freq
#define Code fc.code
#define Dad  dl.dad
#define Len  dl.len

typedef struct static_tree_desc_s  static_tree_desc;

typedef struct tree_desc_s {
  ct_data *dyn_tree;           /* the dynamic tree */
  int     max_code;            /* largest code with non zero frequency */
  static_tree_desc *stat_desc; /* the corresponding static tree */
  ulg     opt_len;             /* compressed size of data - if dyn_tree used */
  ulg     static_len;          /* compr. size of data if stat_desc used */
} FAR tree_desc;

typedef ush Pos;
typedef Pos FAR Posf;
typedef unsigned IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

typedef struct zd_internal_state {
  zd_streamp strm;      /* pointer back to this zlib stream */
  int   status;        /* as the name implies */
  Bytef *pending_buf;  /* output still pending */
  ulg   pending_buf_size; /* size of pending_buf */
  Bytef *pending_out;  /* next pending byte to output to the stream */
  int   pending;       /* nb of bytes in the pending buffer */
  int   noheader;      /* suppress zlib header and adler32 */
  Byte  data_type;     /* UNKNOWN, BINARY or ASCII */
  Byte  method;        /* STORED (for zip only) or DEFLATED */
  int   last_flush;    /* value of flush param for previous deflate call */

  /* used by deflate.c: */
  
  uInt  w_size;         /* LZ77 window size (32K by default) */
  uInt  w_bits;         /* log2(w_size)  (8..16) */
  uInt  w_mask;         /* w_size - 1 */
  uInt  rw_mask;        /* zdelta: 2*w_size - 1 */
  

  Bytef *window;
  /* Sliding window. Input bytes are read into the second half of the window,
   * and move to the first half later to keep a dictionary of at least wSize
   * bytes. With this organization, matches are limited to a distance of
   * wSize-MAX_MATCH bytes, but this ensures that IO is always
   * performed with a length multiple of the block size. Also, it limits
   * the window size to 64K, which is quite useful on MSDOS.
   * To do: use the user input buffer as sliding window.
   */

  Bytef *ref_window[REFNUM];	/* zdelta: reference file buffer */
  /* reference window is read and its dictionary is built at once
   * the window is always slided by half its size towards the end of the data
   * when the window is slided its dictionary is completely flushed and then
   * rebuilt from scratch
   */
  
  ulg window_size;
  /* Actual size of window: 2*wSize, except when the user input buffer
   * is directly used as sliding window.
   */
  ulg ref_window_size[REFNUM];
  /* zdelta: reference window size */

  Posf *prev;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */
  Posf *ref_prev[REFNUM];
  /* zdelta: reference file hash table */
  
  Posf *head;     /* Heads of the hash chains or NIL.             */
  Posf *ref_head[REFNUM]; /* zdelta: Heads the reference file hash chains */

  uInt  ins_h;    /* hash index of string to be inserted          */
  uInt  ref_h;          
  /* zdelta: hash index of string to be inserted into reference hash table */
  uInt  hash_size;      /* number of elements in hash table */
  uInt  hash_bits;      /* log2(hash_size)                  */
  uInt  hash_mask;      /* hash_size-1                      */

  uInt  hash_shift;
  /* Number of bits by which ins_h must be shifted at each input
   * step. It must be such that after MIN_MATCH steps, the oldest
   * byte no longer takes part in the hash key, that is:
   *   hash_shift * MIN_MATCH >= hash_bits
   */

  long block_start;
  /* Window position at the beginning of the current output block. Gets
   * negative when the window is moved backwards.
   */
  

  uInt match_length;           /* length of best match */
  IPos prev_match;             /* previous match */
  int  match_available;        /* set if previous match exists */
  uInt strstart;               /* start of string to insert */
  uInt match_start;            /* start of matching string */
  uInt lookahead;              /* number of valid bytes ahead in window */

  int  match_ptr;	       /* zdelta: match pointer */
  int  prev_ptr;	       /* zdelta: previous match pointer */
  uInt match_sign;             /* zdelta: distance sign for the match */
  uInt prev_sign;              /* zdelta: distance sign for the prev. match */
  /* zdelta reference matches can have positive and negative distances */
  ulg  match_distance;         /* zdelta: absolute match distance */
  ulg  prev_distance;          /* zdelta: absolute prev. match distance */
  int  match_benefit;          /* zdelta: benefit in bits given by 
				* the best available match */ 
  uInt prev_length;
  /* Length of the best match at previous step. Matches not greater than this
   * are discarded. This is used in the lazy match evaluation.
   */
  int prev_benefit;
  /* zdelta: benefit in bits given by the best match at the previous step */ 

  uInt max_chain_length;
  /* To speed up deflation, hash chains are never searched beyond this
   * length.  A higher limit improves compression ratio but degrades the
   * speed.
   */

  uInt max_lazy_match;
  /* Attempt to find a better match only when the current match is strictly
   * smaller than this value. This mechanism is used only for compression
   * levels >= 4.
   */
#   define max_insert_length  max_lazy_match
  /* Insert new strings in the hash table only if the match length is not
   * greater than this length. This saves time but degrades compression.
   * max_insert_length is used only for compression levels <= 3.
   */


  /* zdelta: the following 4 variables are not used; kept for future use */
  int level;    /* compression level (1..9) */
  int strategy; /* favor or force Huffman coding*/
  uInt good_match;
  /* Use a faster search when the previous match is longer than this */
  int nice_match; /* Stop searching when current match exceeds this */


  /* used by trees.c: */
  /* Didn't use ct_data typedef below to supress compiler warning */
  struct ct_data_s dyn_ltree[2*L_CODES+1];   /* length tree trees            */
  struct ct_data_s dyn_dtree[2*D_CODES+1];   /* literal/distance tree        */
  struct ct_data_s dyn_ztree[2*Z_CODES+1];   /* zdelta: extra flag           */
  struct ct_data_s bl_tree[2*BL_CODES+1];    /* Huffman tree for bit lengths */
  
  struct tree_desc_s l_desc;               /* desc. for literal tree       */
  struct tree_desc_s d_desc;               /* desc. for distance tree      */
  struct tree_desc_s z_desc;               /* desc. for zdelta extra tree  */
  struct tree_desc_s bl_desc;              /* desc. for bit length tree    */

  ush bl_count[MAX_BITS+1];
  /* number of codes at each bit length for an optimal tree */

  int heap[2*D_CODES+1];      /* heap used to build the Huffman trees */
  int heap_len;               /* number of elements in the heap       */
  int heap_max;               /* element of largest frequency         */
  /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
   * The same heap array is used to build all trees.
   */

  uch depth[2*D_CODES+1];
  /* Depth of each subtree used as tie breaker for trees of equal frequency
   */

  ushf *l_buf;          /* buffer for literals or lengths */

  uInt  lit_bufsize;
  /* Size of match buffer for literals/lengths.  There are 4 reasons for
   * limiting lit_bufsize to 64K:
   *   - frequencies can be kept in 16 bit counters
   *   - if compression is not successful for the first block, all input
   *     data is still in the window so we can still emit a stored block even
   *     when input comes from standard input.  (This can also be done for
   *     all blocks if lit_bufsize is not greater than 32K.)
   *   - if compression is not successful for a file smaller than 64K, we can
   *     even emit a stored file instead of a stored block (saving 5 bytes).
   *     This is applicable only for zip (not gzip or zlib).
   *   - creating new Huffman trees less frequently may not provide fast
   *     adaptation to changes in the input data statistics. (Take for
   *     example a binary file with poorly compressible code followed by
   *     a highly compressible string table.) Smaller buffer sizes give
   *     fast adaptation but have of course the overhead of transmitting
   *     trees more frequently.
   *   - I can't count above 4
   */

  uInt last_lit;      /* running index in l_buf */

  ushf *d_buf;
  /* Buffer for distances. To simplify the code, d_buf and l_buf have
   * the same number of elements. To use different lengths, an extra flag
   * array would be necessary.
   */
  
  uchf     *z_buf;   
  /* zdelta: buffer storing the extra bits for zdelta extra codes
   * coding ofset pointers and directions 
   */
  
  ulg opt_len;        /* bit length of current block with optimal trees */
  ulg static_len;     /* bit length of current block with static trees */
  uInt matches;       /* number of string matches in current block */
  int last_eob_len;   /* bit length of EOB code for last block */

#ifdef DEBUG
  ulg compressed_len; /* total bit length of compressed file mod 2^32 */
  ulg bits_sent;      /* bit length of compressed data sent mod 2^32 */
#endif

  ush bi_buf;
  /* Output buffer. bits are inserted starting at the bottom (least
   * significant bits).
   */
  int bi_valid;
  /* Number of valid bits in bi_buf.  All bits above the last valid bit
   * are always zero.
   */
  
  /* zdelta: all following variables are added */

  /* base pointers; getting negative when base window shifts */ 
  long rwptr[ZD_RPN*REFNUM];   
  int  stable[ZD_RPN*REFNUM]; /* reference pointer states, used for the 
			       * reference pointer movement strategy
			       */

  /* reference widnow pointer values at the time of the most recent flush
   * used for restoring the pointer values in case of uncompressible
   * block (stored_block)
   */
  long rwptr_save[ZD_RPN*REFNUM];
  int stable_save[ZD_RPN*REFNUM]; /*  pinter states at the time 
				      of the most recent flush */
  uch stored_allowed[REFNUM];

} FAR deflate_state;

/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#define put_byte(s, c) {s->pending_buf[s->pending++] = (c);}

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#define MAX_DIST(s)  ((s)->w_size-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

/* in trees.c */
/* zdelta: prefix zd is added to the following functions */
void zd_tr_init         OF((deflate_state *s));
/* NOTUSED
 * int  zd_tr_tally        OF((deflate_state *s, unsigned dist, unsigned lc));
 */
void zd_tr_flush_block  OF((deflate_state *s, charf *buf, ulg stored_len,
			  int eof));
void zd_tr_align        OF((deflate_state *s));
void zd_tr_stored_block OF((deflate_state *s, charf *buf, ulg stored_len,
                          int eof));

#define d_code(dist) \
   ((dist) < 256 ? zd_dist_code[dist] : zd_dist_code[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. zd_dist_code[256] and zd_dist_code[257] are
 * never used.
 */

#define z_code(z) (z & 0x1f)

#define l_code(len) \
   ((len) < 256 ? zd_length_code[len] : \
    (len) == (MAX_MATCH-MIN_MATCH) ? zd_length_code[LENGTH_CODE_LEN-1] : \
    zd_length_code[256+((len)>>6)])

/* Mapping from a len to a length code. len is the length - 3 and
 * must not have side effects. zd_lenght_code[286] and zd_length_code[287] are
 * never used.
 */
/* zdelta: calculate zdelta byte code */
#define ZCODE(sgn,ptr) (sgn|ptr)

/* Inline versions of _tr_tally for speed: */
#if defined(GEN_TREES_H) || !defined(STDC)
  extern uch zd_length_code[];
  extern uch zd_dist_code[];
#else
  extern const uch zd_length_code[];
  extern const uch zd_dist_code[];
#endif

# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->z_buf[s->last_lit]   = LITERAL; \
    s->d_buf[s->last_lit++] = cc; \
    s->dyn_dtree[cc].Freq++; \
    flush = (s->last_lit == s->lit_bufsize-1); \
   }


#define OFFSET(z) (z&ZD_MINUS ? (LENGTH_CODES) : (2*LENGTH_CODES))

/* zdelta: modified to handle the extra zdelta code */
# define _tr_tally_dist_ref(s, distance, length, zd, flush) \
  { ush len = (length); \
    ush dist = (distance); \
    s->d_buf[s->last_lit]   = dist; \
    s->z_buf[s->last_lit]   = zd; \
    s->l_buf[s->last_lit++] = len; \
    s->dyn_ltree[l_code(len)+OFFSET(zd)].Freq++; \
    s->dyn_dtree[LITERALS+1+d_code(dist)].Freq++; \
    s->dyn_ztree[z_code(zd)].Freq++; \
    flush = (s->last_lit == s->lit_bufsize-1); \
  }

# define _tr_tally_dist_tar(s, distance, length, flush) \
  { ush len = (length); \
    ush dist = (distance); \
    s->d_buf[s->last_lit]   = dist; \
    s->z_buf[s->last_lit]   = TARGET; \
    s->l_buf[s->last_lit++] = len; \
    s->dyn_ltree[l_code(len)].Freq++; \
    s->dyn_dtree[LITERALS+1+d_code(dist)].Freq++; \
    flush = (s->last_lit == s->lit_bufsize-1); \
  }
#endif
