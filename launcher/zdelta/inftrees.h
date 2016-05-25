/* inftrees.h -- header to use inftrees.c
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zdlib.h.
         
   file modified by Dimitre Trendafilov (2002)   
 */

/* zdelta:
 *
 * modified: 
 *          inflate_trees_fixed
 *          inflate_trees_dynamic
 * added:
 *          --
 * removed:
 *          --
 */

/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model). */

typedef struct inflate_huft_s FAR inflate_huft;

struct inflate_huft_s {
  union {
    struct {
      Byte Exop;        /* number of extra bits or operation */
      Byte Bits;        /* number of bits in this code or subcode */
    } what;
    uInt pad;           /* pad structure to a power of 2 (4 bytes for */
  } word;               /*  16-bit, 8 bytes for 32-bit int's) */
  uInt base;            /* literal, length base, distance base,
                           or table offset */
};

#define REFMATCH   (1<<14)
#define NREFMATCH  ((1<<15) | REFMATCH)

#define D_CODES  288     /* number of literal/distance codes */
#define L_CODES  111     /* number of length codes           */
#if REFNUM>1 || BUILDFIXED || GEN_TREES_H
# define Z_CODES    8     /* number of zdelta codes           */
#else
# define Z_CODES    2     /* number of zdelta codes           */
#endif
#define MAX_CODES D_CODES



/* Maximum size of dynamic tree.  The maximum found in a long but non-
   exhaustive search was 1004 huft structures (850 for length/literals
   and 154 for distances, the latter actually the result of an
   exhaustive search).  The actual maximum is not known, but the
   value below is more than safe. */
#define MANY 1440

extern int inflate_trees_bits OF((
    uIntf *,                    /* 19 code lengths                       */
    uIntf *,                    /* bits tree desired/actual depth        */
    inflate_huft * FAR *,       /* bits tree result                      */
    inflate_huft *,             /* space for trees                       */
    zd_streamp));                /* for messages                          */

extern int inflate_trees_dynamic OF((
    uInt,                       /* number of literal/length codes        */
    uInt,                       /* number of distance codes              */
    uInt,                       /* number of zdelta codes                */
    uIntf *,                    /* that many (total) code lengths        */
    uIntf *,                    /* literal desired/actual bit depth      */
    uIntf *,                    /* distance desired/actual bit depth     */
    uIntf *,                    /* zdelta codes desired/actual bit depth */
    inflate_huft * FAR *,       /* literal/length tree result            */
    inflate_huft * FAR *,       /* distance tree result                  */
    inflate_huft * FAR *,       /* zdelta codes tree result              */
    inflate_huft *,             /* space for trees                       */
    zd_streamp));                /* for messages                          */

extern int inflate_trees_fixed OF((
    uIntf *,                    /* literal desired/actual bit depth      */
    uIntf *,                    /* distance desired/actual bit depth     */
    uIntf *,                    /* zdelta flags desired/actual bit depth */
    inflate_huft * FAR *,       /* literal/length tree result            */
    inflate_huft * FAR *,       /* distance tree result                  */
    inflate_huft * FAR *,       /* zdekt flags tree result               */
    zd_streamp));                /* for memory allocation                 */
