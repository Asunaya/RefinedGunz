# Makefile for libzd 
# Original zlib Makefile file, modified to match libzd.
# Dimitre Trendafilov (2003)


# Makefile for zlib
# Copyright (C) 1995-1998 Jean-loup Gailly.
# For conditions of distribution and use, see copyright notice in zlib.h


# To compile and test, type:
# make test
#
# To compile the command line delta compressor, type:
# make zdc 
#
# To compile the command line delta decompressor, type:
# make zdu 
#
# For multiple reference file support compile with REFNUM=N option
# where N is the desired number (1..4) of referemce files. The default
# value is 1. 
# NOTE: The number of reference files comes at a price! Based on
# the selected number of reference files the compression may degrade, 
# and/or the memory usage may be increase. Do NOT add support for more 
# reference files than you need!!!
#
# To suppress the zdelta header and checksum compile with -DNO_ERROR_CHECK

# To install /usr/local/lib/libzd.*, /usr/local/include/zdlib.h, and
# /usr/local/include/zdconf.h type:
#    make install
# To install in $HOME instead of /usr/local, use:
#    make install prefix=$HOME

CC=gcc

CFLAGS= -O2 -W -Wall -pedantic -ansi -g -DREFNUM=2

LDSHARED=$(CC)
CPP=$(CC) -E

VER=2.1
LIBS=libzd.a
SHAREDLIB=libzd.so

AR=ar rc
RANLIB=ranlib
TAR=tar
SHELL=/bin/sh

prefix = /usr/local
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include

OBJS	= adler32.o deflate.o  infblock.o infcodes.o inffast.o \
          inflate.o inftrees.o infutil.o zd_mem.o trees.o zdelta.o zutil.o

TEST_OBJS = example.o minigzip.o

#DISTFILES = README FAQ INDEX ChangeLog configure Make*[a-z0-9] *.[ch] *.mms \
#  algorithm.txt zlib.3 msdos/Make*[a-z0-9] msdos/zlib.def msdos/zlib.rc \
#  nt/Make*[a-z0-9] nt/zlib.dnt amiga/Make*.??? os2/M*.os2 os2/zlib.def \
#  contrib/RE*.contrib contrib/*.txt contrib/asm386/*.asm contrib/asm386/*.c \
#  contrib/asm386/*.bat contrib/asm386/zlibvc.d?? contrib/asm[56]86/*.?86 \
#  contrib/asm[56]86/*.S contrib/iostream/*.cpp \
#  contrib/iostream/*.h  contrib/iostream2/*.h contrib/iostream2/*.cpp \
#  contrib/untgz/Makefile contrib/untgz/*.c contrib/untgz/*.w32 \
#  contrib/minizip/[CM]*[pe] contrib/minizip/*.[ch] contrib/minizip/*.[td]?? \
#  contrib/delphi*/*.???

all: zdc zdu

test: all
	@LD_LIBRARY_PATH=.:$(LD_LIBRARY_PATH) ; export LD_LIBRARY_PATH; \
	echo 'libzd test, using Makefile as reference file';\
	echo '*** libzd test OK ***' | ./zdc Makefile | ./zdu Makefile

libzd: libzd.a

libzd.a: $(OBJS) $(OBJA)
	$(AR) $@ $(OBJS) 
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

$(SHAREDLIB).$(VER): $(OBJS)
	$(LDSHARED) -G -o $@ $(OBJS)
	rm -f $(SHAREDLIB)
	ln -s $@ $(SHAREDLIB)

zdc: zdc.o $(LIBS)
	$(CC) $(CFLAGS) -o $@ zdc.o $(LIBS)

zdu: zdu.o $(LIBS)
	$(CC) $(CFLAGS) -o $@ zdu.o $(LIBS)

_zdc: _zdc.o $(LIBS)
	$(CC) $(CFLAGS) -o $@ _zdc.o $(LIBS)

_zdu: _zdu.o $(LIBS)
	$(CC) $(CFLAGS) -o $@ _zdu.o $(LIBS)

install: $(LIBS)
	-@if [ ! -d $(includedir)  ]; then mkdir $(includedir); fi
	-@if [ ! -d $(libdir) ]; then mkdir $(libdir); fi
	cp zdlib.h $(includedir)
	cp zdconf.h $(includedir)
	chmod 644 $(includedir)/zdlib.h
	chmod 644 $(includedir)/zdconf.h
	cp $(LIBS) $(libdir)
	cd $(libdir); chmod 755 $(LIBS)
	-@(cd $(libdir); $(RANLIB) libzd.a || true) >/dev/null 2>&1
	cd $(libdir); if test -f $(SHAREDLIB).$(VER); then \
	  rm -f $(SHAREDLIB) $(SHAREDLIB).1; \
	  ln -s $(SHAREDLIB).$(VER) $(SHAREDLIB); \
	  ln -s $(SHAREDLIB).$(VER) $(SHAREDLIB).1; \
	  (ldconfig || true)  >/dev/null 2>&1; \
	fi
# The ranlib in install is needed on NeXTSTEP which checks file times
# ldconfig is for Linux


uninstall:
	cd $(includedir); \
	v=$(VER); \
	if test -f zdlib.h; then \
	  v=`sed -n '/VERSION "/s/.*"\(.*\)".*/\1/p' < zdlib.h`; \
          rm -f zdlib.h; \
	fi; \
	if test -f zdconf.h; then \
	  v=`sed -n '/VERSION "/s/.*"\(.*\)".*/\1/p' < zdconf.h`; \
          rm -f zdconf.h; \
	fi; \
	cd $(libdir); rm -f libzd.a; \
	if test -f $(SHAREDLIB).$$v; then \
	  rm -f $(SHAREDLIB).$$v $(SHAREDLIB) $(SHAREDLIB).1; \
	fi

clean:
	rm -f $(OBJS) *~ zdu zdu.o zdc zdc.o libzd.a libzd.so* so_locations \
	_zdc _zdc.o _zdu _zdu.o

distclean:	clean

#TODO: fix this
#dist:
#	mv Makefile Makefile~; cp -p Makefile.in Makefile
#	rm -f test.c ztest*.c contrib/minizip/test.zip
#	d=zlib-`sed -n '/VERSION "/s/.*"\(.*\)".*/\1/p' < zdlib.h`;\
#	rm -f $$d.tar.gz; \
#	if test ! -d ../$$d; then rm -f ../$$d; ln -s `pwd` ../$$d; fi; \
#	files=""; \
#	for f in $(DISTFILES); do files="$$files $$d/$$f"; done; \
#	cd ..; \
#	GZIP=-9 $(TAR) chofz $$d/$$d.tar.gz $$files; \
#	if test ! -d $$d; then rm -f $$d; fi
#	mv Makefile~ Makefile

tags:	
	etags *.[ch]

depend:
	makedepend -- $(CFLAGS) -- *.[ch]

# DO NOT DELETE THIS LINE -- make depend depends on it.

adler32.o:  zdconf.h zdlib.h
compress.o:  zdconf.h zdlib.h
deflate.o: deflate.h zutil.h  zdconf.h zdlib.h
infblock.o: infblock.h inftrees.h infcodes.h infutil.h
infblock.o: zutil.h  zdconf.h zdlib.h
infcodes.o: zutil.h  zdconf.h zdlib.h
infcodes.o: inftrees.h infblock.h infcodes.h infutil.h inffast.h zdlib.h
inffast.o: zutil.h  zdconf.h inftrees.h zdlib.h
inffast.o: infblock.h infcodes.h infutil.h inffast.h zdlib.h
inflate.o: zutil.h  zdconf.h infblock.h zdlib.h
inftrees.o: zutil.h  zdconf.h inftrees.h zdlib.h
infutil.o: zutil.h  zdconf.h infblock.h inftrees.h
infutil.o: infcodes.h infutil.h zdlib.h
trees.o: deflate.h zutil.h  zdconf.h trees.h zdlib.h
zutil.o: zutil.h  zdconf.h zdlib.h
zdelta.o: zutil.h tailor.h  zdconf.h zdlib.h
zdc.o: zd_mem.h zdlib.h
zdu.o: zd_mem.h zdlib.h

_zdc.o: zd_mem.h zdlib.h
_zdu.o: zd_mem.h zdlib.h
