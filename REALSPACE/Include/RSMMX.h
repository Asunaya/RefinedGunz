/*
	RSMMX.h
	-------

	Programming by Joongpil Cho
	All copyright (c) 1999, MAIET entertainment Inc.
*/
#ifndef __RSMMX_HEADER__
#define __RSMMX_HEADER__


/* MMX 판별 함수 */
bool	__cdecl InitMMXFunction();
extern	void*	(*RSMemCopy)(void *pDest, const void *pSrc, size_t size);


#endif // __RSMMX_HEADER__

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////





