#pragma once

#include "GlobalTypes.h"

struct MPOINT {
	int x, y;
public:
	MPOINT() = default;
	MPOINT(int x, int y){ MPOINT::x = x, MPOINT::y = y; }

	void Scale(float x, float y);

	// Stretches the values from a 640x480 resolution to the actual workspace size
	void ScaleRes();
	// Centers the values from a 640x480 resolution to the actual workspace size
	void TranslateRes();
};

struct MRECT {
	int	x, y;
	int w, h;
public:
	MRECT() = default;
	MRECT(int x, int y, int w, int h) : x{ x }, y{ y }, w{ w }, h{ h } {}

	bool InPoint(const MPOINT& p){
		if (p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h) return true;
		return false;
	}

	void ScalePos(float x, float y);
	void ScaleArea(float x, float y);
	void ScalePosRes();
	void ScaleAreaRes();
	void TranslateRes();

	void EnLarge(int w);
	void Offset(int x, int y);

	bool Intersect(MRECT* pIntersect, const MRECT& r);
};

struct MSIZE {
	int w, h;
public:
	MSIZE() = default;
	MSIZE(int w, int h){
		MSIZE::w = w;
		MSIZE::h = h;
	}
};

#define MINT_ARGB(a,r,g,b)		( ((((unsigned long int)a)&0xFF)<<24) | ((((unsigned long int)r)&0xFF)<<16) | ((((unsigned long int)g)&0xFF)<<8) | (((unsigned long int)b)&0xFF) )
#define MINT_RGB(r,g,b)			( ((((unsigned long int)r)&0xFF)<<16) | ((((unsigned long int)g)&0xFF)<<8) | (((unsigned long int)b)&0xFF) )

struct MCOLOR {
public:
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
public:
	MCOLOR(){
		r = g = b = a = 0;
	}
	MCOLOR(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
		MCOLOR::r = r, MCOLOR::g = g, MCOLOR::b = b, MCOLOR::a = a;
	}
	MCOLOR(unsigned long int argb){
		a = u8( (argb & 0xFF000000) >> 24 );
		r = u8( (argb & 0x00FF0000) >> 16 );
		g = u8( (argb & 0x0000FF00) >> 8 );
		b = u8( (argb & 0x000000FF) );
	}
	
	unsigned long int GetARGB(){
		return MINT_ARGB(a, r, g, b);
	}
	unsigned long int GetRGB(){
		return MINT_RGB(r, g, b);
	}
};
