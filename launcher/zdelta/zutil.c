/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-1998 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
         
   file modified by Dimitre Trendafilov (2003)   
 */

/* zdelta:
 *
 * modified: 
 *          zlibVersion() - renamed
 *          
 * added:
 *          --
 * removed:
 *          --
 */

/* @(#) $Id$ */

#include "zutil.h"

/*
#include <unistd.h>


#include <ctype.h>
#include <errno.h>
#include <sys/types.h>

#include "tailor.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifndef NO_FCNTL_H
#  include <fcntl.h>
#endif

*/

struct zd_internal_state      {int dummy;}; /* for buggy compilers */

#ifndef STDC
extern void exit OF((int));
#endif

const char *zd_z_errmsg[9] = {
"stream end",          /* ZD_STREAM_END      1  */
"",                    /* ZD_OK              0  */
"file error",          /* ZD_ERRNO         (-1) */
"stream error",        /* ZD_STREAM_ERROR  (-2) */
"data error",          /* ZD_DATA_ERROR    (-3) */
"insufficient memory", /* ZD_MEM_ERROR     (-4) */
"buffer error",        /* ZD_BUF_ERROR     (-5) */
"incompatible version",/* ZD_VERSION_ERROR (-6) */
""};

/* 
 * zdelta: modified
 *         renamed from zlibVersion()
 */
const char * ZEXPORT zdlibVersion()
{
    return ZDLIB_VERSION;
}

#ifdef DEBUG

#  ifndef verbose
#    define verbose 0
#  endif
int z_verbose = verbose;

/*
 * zdelta: added prefix zd
 */
void zd_z_error (m)
    char *m;
{
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
/*
 * zdelta: added prefix zd
 */const char * ZEXPORT zd_zError(err)
    int err;
{
    return ERR_MSG(err);
}


#ifndef HAVE_MEMCPY

void zmemcpy(dest, source, len)
    Bytef* dest;
    const Bytef* source;
    uInt  len;
{
    if (len == 0) return;
    do {
        *dest++ = *source++; /* ??? to be unrolled */
    } while (--len != 0);
}

int zmemcmp(s1, s2, len)
    const Bytef* s1;
    const Bytef* s2;
    uInt  len;
{
    uInt j;

    for (j = 0; j < len; j++) {
        if (s1[j] != s2[j]) return 2*(s1[j] > s2[j])-1;
    }
    return 0;
}

void zmemzero(dest, len)
    Bytef* dest;
    uInt  len;
{
    if (len == 0) return;
    do {
        *dest++ = 0;  /* ??? to be unrolled */
    } while (--len != 0);
}
#endif


#ifdef __TURBOC__
#if (defined( __BORLANDC__) || !defined(SMALL_MEDIUM)) && !defined(__32BIT__)
/* Small and medium model in Turbo C are for now limited to near allocation
 * with reduced MAX_WBITS and MAX_MEM_LEVEL
 */
#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#define MAX_PTR 10
/* 10*64K = 640K */

local int next_ptr = 0;

typedef struct ptr_table_s {
    voidpf org_ptr;
    voidpf new_ptr;
} ptr_table;

local ptr_table table[MAX_PTR];
/* This table is used to remember the original form of pointers
 * to large buffers (64K). Such pointers are normalized with a zero offset.
 * Since MSDOS is not a preemptive multitasking OS, this table is not
 * protected from concurrent access. This hack doesn't work anyway on
 * a protected system like OS/2. Use Microsoft C instead.
 */
/*
 * zdelta: added prefix zd
 */
voidpf zd_zcalloc (voidpf opaque, unsigned items, unsigned size)
{
    voidpf buf = opaque; /* just to make some compilers happy */
    ulg bsize = (ulg)items*size;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    if (bsize < 65520L) {
        buf = farmalloc(bsize);
        if (*(ush*)&buf != 0) return buf;
    } else {
        buf = farmalloc(bsize + 16L);
    }
    if (buf == NULL || next_ptr >= MAX_PTR) return NULL;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    return buf;
}
/*
 * zdelta: added prefix zd
 */
void  zd_zcfree (voidpf opaque, voidpf ptr)
{
    int n;
    if (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        return;
    }
    /* Find the original pointer */
    for (n = 0; n < next_ptr; n++) {
        if (ptr != table[n].new_ptr) continue;

        farfree(table[n].org_ptr);
        while (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        return;
    }
    ptr = opaque; /* just to make some compilers happy */
    Assert(0, "zd_zcfree: ptr not found");
}
#endif
#endif /* __TURBOC__ */


#if defined(M_I86) && !defined(__32BIT__)
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#if (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif
/*
 * zdelta: added prefix zd
 */
voidpf zd_zcalloc (voidpf opaque, unsigned items, unsigned size)
{
    if (opaque) opaque = 0; /* to make compiler happy */
    return _halloc((long)items, size);
}
/*
 * zdelta: added prefix zd
 */
void  zd_zcfree (voidpf opaque, voidpf ptr)
{
    if (opaque) opaque = 0; /* to make compiler happy */
    _hfree(ptr);
}

#endif /* MSC */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
extern voidp  calloc OF((uInt items, uInt size));
extern void   free   OF((voidpf ptr));
#endif
/*
 * zdelta: added prefix zd
 */
voidpf zd_zcalloc (opaque, items, size)
    voidpf opaque;
    unsigned items;
    unsigned size;
{
    if (opaque) items += size - size; /* make compiler happy */
    return (voidpf)calloc(items, size);
}
/*
 * zdelta: added prefix zd
 */
void  zd_zcfree (opaque, ptr)
    voidpf opaque;
    voidpf ptr;
{
    free(ptr);
    if (opaque) return; /* make compiler happy */
}

#endif /* MY_ZCALLOC */
