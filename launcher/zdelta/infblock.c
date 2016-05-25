/* infblock.c -- interpret and process block types to last block
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
  
   file modified by Dimitre Trendafilov (2003)
 */

/* zdelta:
 *
 * modified: 
 *          inflate_blocks_reset()
 *          inflate_blocks()
 * added:
 *          --
 * removed:
 *          inflate_set_dictionary()
 *          inflate_blocks_sync_point()
 */

#include "zutil.h"
#include "infblock.h"
#include "inftrees.h"
#include "infcodes.h"
#include "infutil.h"

#define LBL   7   /* lengths codes number bit length    */
#define DBL   5   /* distances codes number bit length  */
#if REFNUM>1
# define ZBL   3   /* zdelta codes number bit length    */
#else
# define ZBL   0   /* zdelta codes number bit length    */
#endif
#define BBL   4   /* bit length codes number bit length */
#define MASK(x) ( (1<<(x)) - 1 )

/*  struct inflate_codes_state {int dummy;}; */ /* for buggy compilers */

/* simplify the use of the inflate_huft type with some defines */
#define exop word.what.Exop
#define bits word.what.Bits

/* Table for deflate from PKZIP's appnote.txt. */
local const uInt border[] = { /* Order of the bit length code lengths */
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};


/*
  zdelta: the following paragraphs are updated to match libzd features

   Notes beyond the 1.93a appnote.txt:

   1. Distance pointers never point before the beginning of the output
      stream.
   2. Distance pointers can point back across blocks, up to 32k away.
   3. There is an implied maximum of 7 bits for the bit length table and
      15 bits for the actual data.
   4. If only one code exists, then it is encoded using one bit.  (Zero
      would be more efficient, but perhaps a little confusing.)  If two
      codes exist, they are coded using one bit each (0 and 1).
   5. There is no way of sending zero distance codes--a dummy must be
      sent if there are none.  (History: a pre 2.0 version of PKZIP would
      store blocks with no distance codes, but this was discovered to be
      too harsh a criterion.)  Valid only for 1.93a.  2.04c does allow
      zero distance codes, which is sent as one code of zero bits in
      length.
   6. There are up to 286 literal/length codes.  Code 256 represents the
      end-of-block.  Note however that the static length tree defines
      288 codes just to fill out the Huffman codes.  Codes 286 and 287
      cannot be used though, since there is no length base or extra bits
      defined for them.  Similarily, there are up to 30 distance codes.
      However, static trees define 32 codes (all 5 bits) to fill out the
      Huffman codes, but the last two had better not show up in the data.
   7. Unzip can check dynamic Huffman blocks for complete code sets.
      The exception is that a single code would not be complete (see #4).
   8. The five bits following the block type is really the number of
      literal codes sent minus 257.
   9. Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
      (1+6+6).  Therefore, to output three times the length, you output
      three codes (1+1+1), whereas to output four times the same length,
      you only need two codes (1+3).  Hmm.
  10. In the tree reconstruction algorithm, Code = Code + Increment
      only if BitLength(i) is not zero.  (Pretty obvious.)
  11. Correction: 4 Bits: # of Bit Length codes - 4     (4 - 19)
  12. Note: length code 284 can represent 227-258, but length code 285
      really is 258.  The last length deserves its own, short code
      since it gets used a lot in very redundant files.  The length
      258 is special since 258 - 3 (the min match length) is 255.
  13. The literal/length and distance code bit lengths are read as a
      single stream of lengths.  It is possible (and advantageous) for
      a repeat code (16, 17, or 18) to go across the boundary between
      the two sets of lengths.

zdelta modifications:
   1. The maximum match length is increased up to 1026. The lenght is encoded
      as L = c*256 + l, where l is between 3-258 (this is the original zlib
      match length). The coeficient c is encoded into the zdelta byte code and
      is further Huffman encoded using zlib code.
   2. There are no 0 distances for matches in the taget data, but there could
      be 0 distances for matches found in the reference data. To handle this 
      1 is always added to the distance when the match distance is encoded,
      and later on is substracted when the distance is decoded. This descreases
      the effective maximum distance by 1.
 */


/* 
 * zdelta: modified
 */
void inflate_blocks_reset(s, z, c)
inflate_blocks_statef *s;
zd_streamp z;
uLongf *c;
{
  int i;
  if (c != ZD_NULL)
    *c = s->check;
  if (s->mode == BTREE || s->mode == DTREE)
    ZFREE(z, s->sub.trees.blens);
  if (s->mode == CODES)
    inflate_codes_free(s->sub.decode.codes, z);
  s->mode = TYPE;
  s->bitk = 0;
  s->bitb = 0;
  s->read = s->write = s->window;
  if (s->checkfn != ZD_NULL)
    z->adler = s->check = (*s->checkfn)(0L, (const Bytef *)ZD_NULL, 0);
  Tracev((stderr, "inflate:   blocks reset\n"));

  /* zdelta: initialize reference window pointers 
   * this should be called only ONCE during the inflation process
   */
  for(i=0; i< 2*z->refnum; ++i){
    s->rwptr[i]  = 0;
    s->stable[i] = 0;
  }
}


inflate_blocks_statef *inflate_blocks_new(z, c, w)
zd_streamp z;
check_func c;
uInt w;
{
  inflate_blocks_statef *s;

  if ((s = (inflate_blocks_statef *)ZALLOC
       (z,1,sizeof(struct inflate_blocks_state))) == ZD_NULL)
    return s;
  if ((s->hufts =
       (inflate_huft *)ZALLOC(z, sizeof(inflate_huft), MANY)) == ZD_NULL)
  {
    ZFREE(z, s);
    return ZD_NULL;
  }
  if ((s->window = (Bytef *)ZALLOC(z, 1, w)) == ZD_NULL)
  {
    ZFREE(z, s->hufts);
    ZFREE(z, s);
    return ZD_NULL;
  }
  s->end = s->window + w;
  s->checkfn = c;
  s->mode = TYPE;
  Tracev((stderr, "inflate:   blocks allocated\n"));
   
  inflate_blocks_reset(s, z, ZD_NULL);
  return s;
}


int inflate_blocks(s, z, r)
inflate_blocks_statef *s;
zd_streamp z;
int r;
{
  uInt t;               /* temporary storage */
  uLong b;              /* bit buffer */
  uInt k;               /* bits in bit buffer */
  Bytef *p;             /* input data pointer */
  uInt n;               /* bytes available there */
  Bytef *q;             /* output window write pointer */
  uInt m;               /* bytes to end of window or read pointer */

  /* copy input/output information to locals (UPDATE macro restores) */
  LOAD

  /* process input based on current state */
  while (1) switch (s->mode)
  {
  case TYPE:
    NEEDBITS(3)
    t = (uInt)b & 7;
    s->last = t & 1;
    switch (t >> 1)
    {
    case 0:                         /* stored */
      DUMPBITS(3)
      t = k & 7;                    /* go to byte boundary */
      DUMPBITS(t)
      s->mode = LENS;               /* get length of stored block */
      break;
    case 1:                         /* fixed */
      {
	uInt bl, bd, bw;
	inflate_huft *tl, *td, *tw;

	inflate_trees_fixed(&bl, &bd, &bw, &tl, &td, &tw, z);
	s->sub.decode.codes = inflate_codes_new(bl, bd, bw, tl, td, tw, z);
	if (s->sub.decode.codes == ZD_NULL)
	{
	  r = ZD_MEM_ERROR;
	  LEAVE
        }
      }
      DUMPBITS(3)
      s->mode = CODES;
      break;
    case 2:                         /* dynamic */
      DUMPBITS(3)
	s->mode = TABLE;
      break;
    case 3:                         /* illegal */
      DUMPBITS(3)
      s->mode = BAD;
      z->msg = (char*)"invalid block type";
      r = ZD_DATA_ERROR;
      LEAVE
    }
    break;
  case LENS:
    NEEDBITS(32)
    if ((((~b) >> 16) & 0xffff) != (b & 0xffff))
    {
      s->mode = BAD;
      z->msg = (char*)"invalid stored block lengths";
      r = ZD_DATA_ERROR;
      LEAVE
    }
    s->sub.left = (uInt)b & 0xffff;
    b = k = 0;                      /* dump bits */
    s->mode = s->sub.left ? STORED : (s->last ? DRY : TYPE);
    break;
  case STORED:
    if (n == 0)
      LEAVE
    NEEDOUT
    t = s->sub.left;
    if (t > n) t = n;
    if (t > m) t = m;
      
    zmemcpy(q, p, t);
    p += t;  n -= t;
    q += t;  m -= t;
    if ((s->sub.left -= t) != 0)
      break;
    s->mode = s->last ? DRY : TYPE;
    break;
  case TABLE:
    NEEDBITS(LBL+DBL+ZBL+BBL)
    s->sub.trees.table = t = (uInt)b & MASK(LBL+DBL+ZBL+BBL);
#ifndef PKZIP_BUG_WORKAROUND

    if( ((t              ) & MASK(LBL)) > (L_CODES-1) || 
	((t >> (LBL)     ) & MASK(DBL)) > (D_CODES-1) || 
	((t >> (LBL+DBL) ) & MASK(ZBL)) > (Z_CODES-1) )

    {
      s->mode = BAD;
      z->msg = (char*)"too many length, distance or zdelta symbols";
      r = ZD_DATA_ERROR;
      LEAVE
    }
    Tracev((stderr, "\n # len codes     :%u",((t)               &MASK(LBL))));
    Tracev((stderr, "\n # lit/dist codes:%u",((t>>LBL)          &MASK(DBL))));
    Tracev((stderr, "\n # zdelta codes  :%u",((t>>(LBL+DBL))    &MASK(ZBL))));
    Tracev((stderr, "\n # bl codes      :%u",((t>>(LBL+DBL+ZBL))&MASK(BBL))));
#endif
    /* zdelta: there is one more table to decode*/
    t = 259 + (t&MASK(LBL))+((t>>(LBL))&MASK(DBL))+((t>>(LBL+DBL))&MASK(ZBL));

    if ((s->sub.trees.blens = (uIntf*)ZALLOC(z, t, sizeof(uInt))) == ZD_NULL)
    {
      r = ZD_MEM_ERROR;
      LEAVE
    }
    DUMPBITS(LBL+DBL+ZBL+BBL)
    s->sub.trees.index = 0;
    Tracev((stderr, "\ninflate:       table sizes ok\n"));
    s->mode = BTREE;
  case BTREE:
    while (s->sub.trees.index < 4 + (s->sub.trees.table >> (LBL+DBL+ZBL)) )
    {
      NEEDBITS(3)
      s->sub.trees.blens[border[s->sub.trees.index++]] = (uInt)b & 7;
      Tracev((stderr, "\nbl code %2d len %2d ", 
	      border[s->sub.trees.index-1], (uInt)b & 7) );
      DUMPBITS(3)
    }
    while (s->sub.trees.index < 19)
      s->sub.trees.blens[border[s->sub.trees.index++]] = 0;
    s->sub.trees.bb = 7;
    t = inflate_trees_bits(s->sub.trees.blens, &s->sub.trees.bb,
			   &s->sub.trees.tb, s->hufts, z);
    if (t != ZD_OK)
    {
      ZFREE(z, s->sub.trees.blens);
      r = t;
      if (r == ZD_DATA_ERROR)
	s->mode = BAD;
      LEAVE
    }
    s->sub.trees.index = 0;
    Tracev((stderr, "\ninflate:       bits tree ok\n"));
    s->mode = DTREE;
  case DTREE:
    /* zdelta: there is an additional zdelta tree */
#if REFNUM > 1
    while (t = s->sub.trees.table,
	   s->sub.trees.index < 259 + (t & MASK(LBL)) + 
	   ((t >> LBL) & MASK(DBL)) + ((t >> (LBL+DBL)) & MASK(ZBL)) )
#else
    while (t = s->sub.trees.table,
	   s->sub.trees.index < 258 + (t & MASK(LBL)) + 
	   ((t >> LBL) & MASK(DBL)) + ((t >> (LBL+DBL)) & MASK(ZBL)) )
#endif
    {
      inflate_huft *h;
      uInt i, j, c;
      
      t = s->sub.trees.bb;
      NEEDBITS(t)
      h = s->sub.trees.tb + ((uInt)b & inflate_mask[t]);
      t = h->bits;
      c = h->base;
      if (c < 16)
      {
	DUMPBITS(t)
        s->sub.trees.blens[s->sub.trees.index++] = c;
	Tracevv((stderr, "code: %3u len: %3u\n", 
	       s->sub.trees.index-1,s->sub.trees.blens[s->sub.trees.index-1]));
      }
      else /* c == 16..18 */
      {
	i = c == 18 ? 7 : c - 14;
	j = c == 18 ? 11 : 3;
	NEEDBITS(t + i)
        DUMPBITS(t)
        j += (uInt)b & inflate_mask[i];
        DUMPBITS(i)
        i = s->sub.trees.index;
	t = s->sub.trees.table;
#if REFNUM>1
	if (i + j > 259 + (t & MASK(LBL)) + ((t >> LBL) & MASK(DBL)) + 
	    ((t >> (LBL+DBL) ) & MASK(ZBL)) || (c == 16 && i < 1)) 
#else
	if (i + j > 258 + (t & MASK(LBL)) + ((t >> LBL) & MASK(DBL)) + 
	    ((t >> (LBL+DBL) ) & MASK(ZBL)) || (c == 16 && i < 1)) 
#endif
	{
	  ZFREE(z, s->sub.trees.blens);
	  s->mode = BAD;
	  z->msg = (char*)"invalid bit length repeat";
	  r = ZD_DATA_ERROR;
	  LEAVE
	}
	c = c == 16 ? s->sub.trees.blens[i - 1] : 0;
	do {
	  s->sub.trees.blens[i++] = c;
	  Tracevv((stderr, "code: %3u len: %3u\n", i-1, c));
	} while (--j);
	s->sub.trees.index = i;
      }
    }
    s->sub.trees.tb = ZD_NULL;
    {
      uInt bl, bd, bzd;
      inflate_huft *tl, *td, *tzd;
      inflate_codes_statef *c;

      bl  = 8;         /* must be <= 9 for lookahead assumptions */
      bd  = 9;         /* must be <= 9 for lookahead assumptions */
      bzd = 4;         /* zdelta: must be <= 9 for lookahead assumptions */
      t = s->sub.trees.table;
      /* zdelta:
       * t = inflate_trees_dynamic(257 + (t & 0x1f), 1 + ((t >> 5) & 0x1f),
       *                           s->sub.trees.blens, &bl, &bd, &tl, &td, 
       *                           s->hufts, z); 
       */
      t = inflate_trees_dynamic(1   + (t & MASK(LBL)), 
				257 + ((t>>LBL) & MASK(DBL)),
				1   + ((t>>(LBL+DBL)) & MASK(ZBL)),
				s->sub.trees.blens, &bl, &bd, &bzd, &tl, &td, 
				&tzd, s->hufts, z);
      ZFREE(z, s->sub.trees.blens);
      if (t != ZD_OK)
      {
	if (t == (uInt)ZD_DATA_ERROR)
	  s->mode = BAD;
	r = t;
	LEAVE
      }
      Tracev((stderr, "inflate:       trees ok\n"));

      if ((c = inflate_codes_new(bl, bd, bzd, tl, td, tzd, z)) == ZD_NULL)
      {
	r = ZD_MEM_ERROR;
	LEAVE
      }
      s->sub.decode.codes = c;
    }
    s->mode = CODES;
  case CODES:
    UPDATE
    if ((r = inflate_codes(s, z, r)) != ZD_STREAM_END){
      return inflate_flush(s, z, r);
    }
    r = ZD_OK;
    inflate_codes_free(s->sub.decode.codes, z);
    LOAD
    if (!s->last)
    {
      s->mode = TYPE;
      break;
    }
    s->mode = DRY;
  case DRY:
    FLUSH
    if (s->read != s->write)
    LEAVE
    s->mode = DONE;
  case DONE:
    r = ZD_STREAM_END;
    LEAVE
  case BAD:
    r = ZD_DATA_ERROR;
    LEAVE
  default:
    r = ZD_STREAM_ERROR;
    LEAVE
  }
}


int inflate_blocks_free(s, z)
     inflate_blocks_statef *s;
     zd_streamp z;
{
  inflate_blocks_reset(s, z, ZD_NULL);
  ZFREE(z, s->window);
  ZFREE(z, s->hufts);
  ZFREE(z, s);
  Tracev((stderr, "inflate:   blocks freed\n"));
  return ZD_OK;
}
