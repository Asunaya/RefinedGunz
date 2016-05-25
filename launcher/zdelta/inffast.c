/* inffast.c -- process literals and length/distance pairs fast
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
     
  file modified by Dimitre Trendafilov (2003)
 */

/* zdelta:
 *
 * modified: 
 *          inflate_fast
 * added:
 *          --
 * removed:
 *          --
 */

#include "zutil.h"
#include "inftrees.h"
#include "infblock.h"
#include "infcodes.h"
#include "infutil.h"
#include "inffast.h"

/*  struct inflate_codes_state {int dummy;}; */ /* for buggy compilers */

/* simplify the use of the inflate_huft type with some defines */
#define exop word.what.Exop
#define bits word.what.Bits

/* macros for bit input with no checking and for returning unused bytes */
#define GRABBITS(j) {while(k<(j)){b|=((uLong)NEXTBYTE)<<k;k+=8;}}
#define NEEDBITS(j) {while(k<(j)){NEEDBYTE;b|=((uLong)NEXTBYTE)<<k;k+=8;}}
#define UNGRAB {c=z->avail_in-n;c=(k>>3)<c?k>>3:c;n+=c;p-=c;k-=c<<3;}

/* Called with number of bytes left to write in window at least 258
   (the maximum string length) and number of input bytes available
   at least ten.  The ten bytes are six bytes for the longest length/
   distance pair plus four bytes for overloading the bit buffer. */

/*
 * zdelta: modified
 */
int inflate_fast(bl, bd, bzd, tl, td, tzd, s, z)
uInt bl, bd, bzd;
inflate_huft *tl;
inflate_huft *td;
inflate_huft *tzd; /* need separate declaration for Borland C++ */
inflate_blocks_statef *s;
zd_streamp z;
{
  inflate_huft *t;      /* temporary pointer */
  uInt e;               /* extra bits or operation */
  uLong b;              /* bit buffer */
  uInt k;               /* bits in bit buffer */
  Bytef *p;             /* input data pointer */
  uInt n;               /* bytes available there */
  Bytef *q;             /* output window write pointer */
  uInt m;               /* bytes to end of window or read pointer */
  uInt ml;              /* mask for literal/length tree */
  uInt md;              /* mask for distance tree */
  uInt mzd;             /* mask for zdelta code tree */
  uInt c;               /* bytes to copy */
  uInt d;               /* distance back to copy from */
  Bytef *r;             /* copy source pointer */

  /* zdelta local variables, needed to handle the reference window ptrs */
  uInt sign;
  uInt best_ptr;
  ulg ptr;
  ulg *rwptr = s->rwptr;         /* zdelta: reference window ptr */
  /* uInt zd_bcode; */                /* zdelta: byte code */
  /* load input, output, bit values */
  LOAD

  /* initialize masks */
  ml = inflate_mask[bl];
  md = inflate_mask[bd];
  mzd = inflate_mask[bzd];

  /* do until not enough input or output space for fast loop */
  do {                          /* assume called with m >= 258 && n >= 10 */
    /* get literal/distance code */
    GRABBITS(15) /* max bits for literal/distance code */
    if ((e = (t = td + ((uInt)b & md))->exop) == 0)
    {
      Assert(t->bits <= k, "dumping too many bits (literal)");
      DUMPBITS(t->bits)
      Tracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
                "inflate:        * literal '%c'\n" :
                "inflate:        * literal 0x%02x\n", t->base));
      /* output the literal here */
      *q++ = (Byte)t->base;
      TraceHuf( (stderr, "d:%u\n", t->base) );
      m--;
      continue;
    }
    do {
      Assert(t->bits <= k, "dumping too many bits (dist)");
      DUMPBITS(t->bits)
      if (e & 16)
      {
	e &= 15;
	GRABBITS(e) /* get extra bits for distance (up to 13) */
	d = t->base + ((uInt)b & inflate_mask[e]);
	Assert(e <= k, "dumping too many bits (dist extra)");
	DUMPBITS(e)
        Tracevv((stderr, "inflate:         * distance %u\n", c));

        /* decode length base of block to copy */
        GRABBITS(24);           /* max bits for length code and extra */
        e = (t = tl + ((uInt)b & ml))->exop;
        do {
	  Assert(t->bits <= k, "dumping too many bits (len)");
          DUMPBITS(t->bits)
          if (e & 16)
          {
            /* get extra bits to add to distance base */
            e &= 15;
            c = t->base + ((uInt)b & inflate_mask[e]);
	    Assert(e <= k, "dumping too many bits (len extra)");
            DUMPBITS(e)
            Tracevv((stderr, "inflate:         * length %u\n", d));

	    if( c & REFMATCH ) {             /* if reference copy          */
	      sign = (c >> 8) & ZD_MINUS;    /* get distance sign          */
	      c   &= (REFMATCH-1);
	      
	      GRABBITS(15)                  /* max bits for ZDELTA flag code */
	      e = (t = tzd + ((uInt)b & mzd))->exop;
	      do{
		Assert(t->bits <= k, "dumping too many bits (zdelta)");
		DUMPBITS(t->bits);
		if(e == 0){
		  best_ptr = (Byte)t->base;  /* get reference pointer      */

		  /* jump to the beginning of the copy */
		  ptr = rwptr[best_ptr] + (sign == ZD_PLUS ? d : -d);   

		  /* do the copy */
		  memcpy(q,z->base[best_ptr/2]+ptr,c);
		  q += c;                    /* update pointers            */
		  m -= c;   /* update the number of available output bytes */

		  if(d > ZD_FAR){ 	     /* update reference pointers  */
		    if(s->stable[best_ptr])
		      s->stable[best_ptr] = 0;
		    else
		      rwptr[best_ptr] = ptr + c;
		  }
		  else{
		    s->stable[best_ptr] = 1;
		    rwptr[best_ptr] = ptr + c;
		  }
		  break;
		}
		if( (e & 64) == 0 ){
		  t += t->base;
		  e = (t += ((uInt)b & inflate_mask[e]))->exop;
		}
		else{
		  z->msg = (char*)"invalid zdelta code";
		  UNGRAB
		  UPDATE
		  return ZD_DATA_ERROR;
		}
	      } while(1); /* zdelta code */
	    }
	    else{
	      m -= c;    /* update the number of available output bytes     */
	      if ((uInt)(q - s->window) >= d)   /*    offset before dest    */
	      {                                 /*    just copy             */
		r = q - d;
		*q++ = *r++;  c--;              /* minimum count is three,  */
		*q++ = *r++;  c--;              /* so unroll loop a little  */
	      }
	      else                        /* else offset after destination  */
	      {
		e = d - (uInt)(q - s->window);  /* bytes from offset to end */
		r = s->end - e;                 /* pointer to offset        */
		if (c > e)                      /* if source crosses,       */
		{	
		  c -= e;                       /* copy to end of window    */
		  do {
		    *q++ = *r++;
		  } while (--e);
		  r = s->window;          /* copy rest from start of window */
		}
	      }
	      do {                              /* copy all or what's left  */
		*q++ = *r++;
	      } while (--c);
	    }
	    break;
          }
          else if ((e & 64) == 0)
	  {
	    t += t->base;
	    e = (t += ((uInt)b & inflate_mask[e]))->exop;
	  }
          else
	  {
	    z->msg = (char*)"invalid length code";
	    UNGRAB
	    UPDATE
	    return ZD_DATA_ERROR;
          }
        } while (1);                                        /* length code */
        break;
      }
      if ((e & 64) == 0)
      {
	t += t->base;
	if ((e = (t += ((uInt)b & inflate_mask[e]))->exop) == 0)
        {
	  DUMPBITS(t->bits)
	  Tracevv((stderr, t->base >= 0x20 && t->base < 0x7f ?
		   "inflate:         * literal '%c'\n" :
		   "inflate:         * literal 0x%02x\n", t->base));
	  *q++ = (Byte)t->base;
	  m--;
	  break;
	}
      }
      else if (e & 32)
      {
	Tracevv((stderr, "inflate:         * end of block\n"));
	UNGRAB
        UPDATE
	return ZD_STREAM_END;
      }
      else
      {
	z->msg = (char*)"invalid literal/distance code";
	UNGRAB
        UPDATE
	return ZD_DATA_ERROR;
      }
    } while (1);                                          /* distance code */
/* zdelta: maximum match length is 1026 
 * } while (m >= 258 && n >= 10); 
 */
  } while (m >= MAX_MATCH && n >= 10); 
  /* not enough input or output--restore pointers and return */
  UNGRAB
  UPDATE
  return ZD_OK;
}


#ifdef NOT_DEFINED
	    TraceHuf( (stderr, "d:%u\n", d) );
	    TraceHuf( (stderr, "l:%u\n", c-3) );
	    TraceHuf( (stderr, "z:%u\n",
		       best_ptr==ZD_RPN?ZD_MINUS|best_ptr:best_ptr) );

#endif
