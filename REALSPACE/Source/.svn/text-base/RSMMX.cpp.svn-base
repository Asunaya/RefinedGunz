/*
	RSMMX.cpp
	---------

	MMX helper functions

	Programming by Joongpil Cho
	All copyright (c) 1999, MAIET entertainment Inc.
*/
#include "RealSpace.h"
#include <memory.h>

// FPU혹은 MMX 의 특수한 전송코드를 사용하기 위한 최소한의 정렬된 바이트수 이보다 작음 memcpy를 써버린다.
#define THIN_THRESHOLD	46

#define FPU_ALIGN		16		// FPU는 16바이트 단위로 전송. 그런데도 MMX보다는 느리다는군.
#define MMX_ALIGN		8		// MMX는 8바이트 단위로 전송

static	void*	MMX_Copy(void *pDest ,const void *pSrc,size_t size);

void*	(*RSMemCopy)(void *pDest, const void *pSrc, size_t size) = memcpy;


/*
	ruIsMMX()
	: MMX를 판별한다. 당연히 return 값이 true이면 MMX instruction 사용가능
*/
bool __cdecl InitMMXFunction()
{
    SYSTEM_INFO si;
    int nCPUFeatures=0;
	bool bRet;

    GetSystemInfo(&si);

    if (si.dwProcessorType != PROCESSOR_INTEL_386 && si.dwProcessorType != PROCESSOR_INTEL_486){
        try {
            __asm
            {
                push eax
                push ebx
                push ecx
                push edx

                mov eax,1
                _emit 0x0f
                _emit 0xa2
                mov nCPUFeatures,edx

                pop edx
                pop ecx
                pop ebx
                pop eax
            }
        }catch(...){
			return false;
        }
    }

    bRet = ((nCPUFeatures&0x00800000)!=0);
	if(bRet){
		// MMX라면
		RSMemCopy = MMX_Copy;
	} else {
		// MMX가 아니면?
		RSMemCopy = memcpy;
	}
	return bRet;
}


/*
	MMX_Copy
	MMX 전송을 이용해서 8바이트 전송을 한다.
*/
static	void* MMX_Copy(void *pDest ,const void *pSrc,size_t size)
{
	unsigned int prebytes,qdwords,postbyte = size;

	if( postbyte	>=	THIN_THRESHOLD )
	{
		prebytes = (MMX_ALIGN-(((long)pDest)%MMX_ALIGN))%MMX_ALIGN;
		if( prebytes>postbyte ) prebytes=postbyte;
		postbyte = postbyte-prebytes;
		qdwords	= postbyte/8;
		postbyte = postbyte-qdwords*8;

		__asm
		{
			MOV		ESI,[pSrc]
			MOV		EDI,[pDest]

			MOV		ECX,[prebytes]
			JCXZ	QWORDSTART
		REP	MOVS	[EDI],[ESI]

QWORDSTART:

			MOV		ECX,[qdwords]
			JCXZ	POSTBYTES

ALIGN 16
QWORDLOOP:
			MOVQ    MM0,[ESI]
			MOVQ    [EDI],MM0
			ADD     ESI, 8
			ADD     EDI, 8
			LOOP	QWORDLOOP

POSTBYTES:
			MOV		ECX,[postbyte]
			JCXZ	DONE
		REP	MOVS	[EDI],[ESI]

DONE:
		}
	}
	else memcpy( pDest, pSrc, postbyte );
	__asm
	{
		EMMS
	}
	return NULL;
}



