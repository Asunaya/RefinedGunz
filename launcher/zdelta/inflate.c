/* inflate.c -- zlib interface to inflate modules
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
         
   file modified by Dimitre Trendafilov (2003)   
 */

/* zdelta:
 *
 * modified: 
 *          inflate() - added prefix zd; removed code for handling dictionaries
 *          
 * added:
 *          --
 * removed:
 *          inflateSetDictionary()
 *          inflateSync()
 *          inflateSyncPoint()
 */

#include "zutil.h"
#include "infblock.h"

struct inflate_blocks_state {int dummy;}; /* for buggy compilers */

typedef enum {
  METHOD,   /* waiting for method byte */
  FLAG,     /* waiting for flag byte */
  DICT4,    /* four dictionary check bytes to go */
  DICT3,    /* three dictionary check bytes to go */
  DICT2,    /* two dictionary check bytes to go */
  DICT1,    /* one dictionary check byte to go */
  DICT0,    /* waiting for inflateSetDictionary */
  BLOCKS,   /* decompressing blocks */
  CHECK4,   /* four check bytes to go */
  CHECK3,   /* three check bytes to go */
  CHECK2,   /* two check bytes to go */
  CHECK1,   /* one check byte to go */
  DONE,     /* finished check, done */
  BAD}      /* got an error--stay here */
inflate_mode;

/* inflate private state */
struct zd_internal_state {

  /* mode */
  inflate_mode  mode;   /* current inflate mode */

  /* mode dependent information */
  union {
    uInt method;        /* if FLAGS, method byte */
    struct {
      uLong was;                /* computed check value */
      uLong need;               /* stream check value */
    } check;            /* if CHECK, check values to compare */
    uInt marker;        /* if BAD, inflateSync's marker bytes count */
  } sub;         /* submode */

  /* mode independent information */
  int  nowrap;          /* flag for no wrapper */
  uInt wbits;           /* log2(window size)  (8..15, defaults to 15) */
  inflate_blocks_statef 
  *blocks;              /* current inflate_blocks state */

};


int ZEXPORT zd_inflateReset(z)
     zd_streamp z;
{
  if (z == ZD_NULL || z->state == ZD_NULL ||
      z->refnum<0 || z->refnum>REFNUM)
    return ZD_STREAM_ERROR;
  z->total_in = z->total_out = 0;
  z->msg = ZD_NULL;
  z->state->mode = z->state->nowrap ? BLOCKS : METHOD;
  inflate_blocks_reset(z->state->blocks, z, ZD_NULL);
  Tracev((stderr, "inflate: reset\n"));
  return ZD_OK;
}


int ZEXPORT zd_inflateEnd(z)
     zd_streamp z;
{
  if (z == ZD_NULL || z->state == ZD_NULL || z->zfree == ZD_NULL)
    return ZD_STREAM_ERROR;
  if (z->state->blocks != ZD_NULL)
    inflate_blocks_free(z->state->blocks, z);
  ZFREE(z, z->state);
  z->state = ZD_NULL;
  Tracev((stderr, "inflate: end\n"));
  return ZD_OK;
}

int ZEXPORT zd_inflateInit2_(z, w, version, stream_size)
     zd_streamp z;
     int w;
     const char *version;
     int stream_size;
{
  if (version == ZD_NULL || version[0] != ZDLIB_VERSION[0] ||
      stream_size != sizeof(zd_stream))
    return ZD_VERSION_ERROR;
  
  /* initialize state */
  if (z == ZD_NULL || z->refnum<0 || z->refnum>REFNUM)
    return ZD_STREAM_ERROR;
  z->msg = ZD_NULL;
  if (z->zalloc == ZD_NULL)
  {
    z->zalloc = zd_zcalloc;
    z->opaque = (voidpf)0;
  }
  if (z->zfree == ZD_NULL) z->zfree = zd_zcfree;
  if ((z->state = (struct zd_internal_state FAR *)
       ZALLOC(z,1,sizeof(struct zd_internal_state))) == ZD_NULL)
    return ZD_MEM_ERROR;
  z->state->blocks = ZD_NULL;

  /* suppress zdlib header or check at compile time */
#ifdef NO_ERROR_CHECK
  z->state->nowrap = 1;
#else
  z->state->nowrap = 0;
#endif

  /* handle undocumented nowrap option (no zdlib header or check) */
  if (w < 0)
  {
    w = - w;
    z->state->nowrap = 1;
  }

  /* set window size */
  if (w < 8 || w > 15)
  {
    zd_inflateEnd(z);
    return ZD_STREAM_ERROR;
  }
  z->state->wbits = (uInt)w;

  /* create inflate_blocks state */
  if ((z->state->blocks =
       inflate_blocks_new(z, z->state->nowrap ? ZD_NULL : zd_adler32,
			  (uInt)1 << w)) == ZD_NULL)
  {
    zd_inflateEnd(z);
    return ZD_MEM_ERROR;
  }
  Tracev((stderr, "inflate: allocated\n"));
  
  /* reset state */
  zd_inflateReset(z);

  return ZD_OK;
}


int ZEXPORT zd_inflateInit_(z, version, stream_size)
     zd_streamp z;
     const char *version;
     int stream_size;
{
  return zd_inflateInit2_(z, DEF_WBITS, version, stream_size);
}


#define NEEDBYTE {if(z->avail_in==0)return r;r=f;}
#define NEXTBYTE (z->avail_in--,z->total_in++,*z->next_in++)

/* 
 * zdelta: modified
 */
int ZEXPORT zd_inflate(z, f)
     zd_streamp z;
     int f;
{
  int r;
  uInt b;

  if (z == ZD_NULL || z->state == ZD_NULL || z->next_in == ZD_NULL ||
      z->next_out == ZD_NULL || z->refnum<0 || z->refnum>REFNUM)
    return ZD_STREAM_ERROR;
  f = f == ZD_FINISH ? ZD_BUF_ERROR : ZD_OK;
  r = ZD_BUF_ERROR;
  while (1) switch (z->state->mode)
  {
  case METHOD:
    NEEDBYTE
    if (((z->state->sub.method = NEXTBYTE) & 0xf) != ZD_DEFLATED)
    {
      z->state->mode = BAD;
      z->msg = (char*)"unknown compression method";
      z->state->sub.marker = 5;       /* can't try inflateSync */
      break;
    }

    if ((z->state->sub.method >> 4) + 8 > z->state->wbits)
    {
      z->state->mode = BAD;
      z->msg = (char*)"invalid window size";
      z->state->sub.marker = 5;       /* can't try inflateSync */
      break;
    }

    z->state->mode = FLAG;
  case FLAG:
    NEEDBYTE
    b = NEXTBYTE;
    if (((z->state->sub.method << 8) + b) % 31)
    {
      z->state->mode = BAD;
      z->msg = (char*)"incorrect header check";
      z->state->sub.marker = 5;       /* can't try inflateSync */
      break;
    }
    /* zdelta: no dictionary */
    z->state->mode = BLOCKS;

  case BLOCKS:
    r = inflate_blocks(z->state->blocks, z, r);
    if (r == ZD_DATA_ERROR)
    {
      z->state->mode = BAD;
      z->state->sub.marker = 0;       /* can try inflateSync */
      break;
    }
    if (r == ZD_OK)
      r = f;
    if (r != ZD_STREAM_END)
      return r;
    r = f;
    inflate_blocks_reset(z->state->blocks, z, &z->state->sub.check.was);
    if (z->state->nowrap)
    {
      z->state->mode = DONE;
      break;
    }
    z->state->mode = CHECK4;
  case CHECK4:
    NEEDBYTE
    z->state->sub.check.need = (uLong)NEXTBYTE << 24;
    z->state->mode = CHECK3;
  case CHECK3:
    NEEDBYTE
    z->state->sub.check.need += (uLong)NEXTBYTE << 16;
    z->state->mode = CHECK2;
  case CHECK2:
    NEEDBYTE
    z->state->sub.check.need += (uLong)NEXTBYTE << 8;
    z->state->mode = CHECK1;
  case CHECK1:
    NEEDBYTE
    z->state->sub.check.need += (uLong)NEXTBYTE;
    if (z->state->sub.check.was != z->state->sub.check.need)
    {
      z->state->mode = BAD;
      z->msg = (char*)"incorrect data check";
      z->state->sub.marker = 5;       /* can't try inflateSync */
      break;
    }
    Tracev((stderr, "inflate: zlib check ok\n"));
    z->state->mode = DONE;
  case DONE:
    return ZD_STREAM_END;
  case BAD:
    return ZD_DATA_ERROR;
  default:
    return ZD_STREAM_ERROR;
  }
#ifdef NEED_DUMMY_RETURN
  return ZD_STREAM_ERROR;  /* Some dumb compilers complain without this */
#endif
}


int ZEXPORT zd_inflateSync(z)
zd_streamp z;
{
  uInt n;       /* number of bytes to look at */
  Bytef *p;     /* pointer to bytes */
  uInt m;       /* number of marker bytes found in a row */
  uLong r, w;   /* temporaries to save total_in and total_out */

  /* set up */
  if (z == ZD_NULL || z->state == ZD_NULL)
    return ZD_STREAM_ERROR;
  if (z->state->mode != BAD)
  {
    z->state->mode = BAD;
    z->state->sub.marker = 0;
  }
  if ((n = z->avail_in) == 0)
    return ZD_BUF_ERROR;
  p = z->next_in;
  m = z->state->sub.marker;

  /* search */
  while (n && m < 4)
  {
    static const Byte mark[4] = {0, 0, 0xff, 0xff};
    if (*p == mark[m])
      m++;
    else if (*p)
      m = 0;
    else
      m = 4 - m;
    p++, n--;
  }

  /* restore */
  z->total_in += p - z->next_in;
  z->next_in = p;
  z->avail_in = n;
  z->state->sub.marker = m;

  /* return no joy or set up to restart on a new block */
  if (m != 4)
    return ZD_DATA_ERROR;
  r = z->total_in;  w = z->total_out;
  zd_inflateReset(z);
  z->total_in = r;  z->total_out = w;
  z->state->mode = BLOCKS;
  return ZD_OK;
}

