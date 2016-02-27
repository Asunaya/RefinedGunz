#ifndef _XWHEELDLL_H_
#define _XWHEELDLL_H_

#pragma once
#pragma warning(disable: 4711)	// disable Level 4 warning: automatic inline expansion

/* Insert your headers here */
#define WIN32_LEAN_AND_MEAN		/* Exclude rarely-used stuff from Windows headers */
#include <windows.h>
#include <string.h>

#ifdef XWHEELDLL_EXPORTS
#define XWHEELDLL_API    __declspec(dllexport)
#else
#define XWHEELDLL_API    __declspec(dllimport)
#endif

void XWHEELDLL_API StartMouseHook();
void XWHEELDLL_API EndMouseHook();
LRESULT XWHEELDLL_API CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);

#endif /* _XWHEELDLL_H_ */