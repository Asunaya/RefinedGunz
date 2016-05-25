/* inffast.h -- header to use inffast.c
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* WARNING: this file should *not* be used by applications. It is
 * part of the implementation of the compression library and is
 * subject to change. Applications should only use zdlib.h.
 *    
 * file modified by Dimitre Trendafilov (2003)
 */

/* zdelta:
 *
 * modified: 
 *          --
 * added:
 *
 * removed:
 *          --
 */

#ifndef ZD_INFFAST_H
#define ZD_INFFAST_H
extern int inflate_fast OF((
    uInt,
    uInt,
    uInt,
    inflate_huft *,
    inflate_huft *,
    inflate_huft *,
    inflate_blocks_statef *,
    zd_streamp ));

#endif
