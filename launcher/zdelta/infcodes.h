/* infcodes.h -- header to use infcodes.c
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zdlib.h.

   file modified by Dimitre Trendafilov (2003)
 */

/* zdelta:
 *
 * modified: 
 *          inflate_codes_new
 *          inflate_codes
 * added:
 *          --
 * removed:
 *          --
 */


struct inflate_codes_state;
typedef struct inflate_codes_state FAR inflate_codes_statef;

extern inflate_codes_statef *inflate_codes_new OF((
    uInt, uInt, uInt,
    inflate_huft *, inflate_huft *, inflate_huft *,
    zd_streamp ));

extern int inflate_codes OF((
    inflate_blocks_statef *,
    zd_streamp ,
    int));

extern void inflate_codes_free OF((
    inflate_codes_statef *,
    zd_streamp ));

