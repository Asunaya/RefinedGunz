#ifndef MTYPES_H
#define MTYPES_H

/// 위치
struct MPOINT{
	int x, y;
public:
	MPOINT(void){}
	MPOINT(int x, int y){ MPOINT::x = x, MPOINT::y = y; }

	void Scale(float x, float y);
	void ScaleRes(void);				///< 648*480기준값을 현재 해상도에 따른 스케일링
	void TranslateRes(void);			///< 648*480 중심점과의 거리만큼 현재 해상도에 맞게 이동
};

/// 영역
struct MRECT{
	int	x, y;	///< 위치
	int w, h;	///< 크기
public:
	MRECT(void){}
	MRECT(int x, int y, int w, int h){
		MRECT::x = x, MRECT::y = y;
		MRECT::w = w, MRECT::h = h;
	}
	bool InPoint(MPOINT& p){
		if(p.x>=x && p.x<=x+w && p.y>=y && p.y<=y+h) return true;
		return false;
	}

	void ScalePos(float x, float y);	///< 시작점에 대한 스케일링
	void ScaleArea(float x, float y);	///< 시작점과 넓이에 대한 스케일링
	void ScalePosRes(void);				///< 648*480기준값을 현재 해상도에 따른 스케일링
	void ScaleAreaRes(void);			///< 648*480기준값을 현재 해상도에 따른 스케일링
	void TranslateRes(void);			///< 648*480 중심점과의 거리만큼 현재 해상도에 맞게 이동

	void EnLarge(int w);				///< 좌우 상하를 w만큼씩 크기 키우기
	void Offset(int x, int y);			///< 포지션 이동

	bool Intersect(MRECT* pIntersect, const MRECT& r);	///< 두 사각형의 공통 영역 얻어내기
};

/// 크기
struct MSIZE{
	int w, h;
public:
	MSIZE(void){}
	MSIZE(int w, int h){
		MSIZE::w = w;
		MSIZE::h = h;
	}
};

/// a,r,g,b값을 unsigned long int값으로 변환
#define MINT_ARGB(a,r,g,b)		( ((((unsigned long int)a)&0xFF)<<24) | ((((unsigned long int)r)&0xFF)<<16) | ((((unsigned long int)g)&0xFF)<<8) | (((unsigned long int)b)&0xFF) )
/// r,g,b값을 unsigned long int값으로 변환
#define MINT_RGB(r,g,b)			( ((((unsigned long int)r)&0xFF)<<16) | ((((unsigned long int)g)&0xFF)<<8) | (((unsigned long int)b)&0xFF) )

/// r, g, b, a 컬러
struct MCOLOR{
public:
	unsigned char	r;		///< Red
	unsigned char	g;		///< Green
	unsigned char	b;		///< Blue
	unsigned char	a;		///< Alpha
public:
	MCOLOR(void){
		r = g = b = a = 0;
	}
	MCOLOR(unsigned char r, unsigned char g, unsigned char b, unsigned char a=255){
		MCOLOR::r = r, MCOLOR::g = g, MCOLOR::b = b, MCOLOR::a = a;
	}
	MCOLOR(unsigned long int argb){
		a = unsigned char( (argb & 0xFF000000) >> 24 );
		r = unsigned char( (argb & 0x00FF0000) >> 16 );
		g = unsigned char( (argb & 0x0000FF00) >> 8 );
		b = unsigned char( (argb & 0x000000FF) );
	}
	
	unsigned long int GetARGB(void){
		return MINT_ARGB(a, r, g, b);
	}
	unsigned long int GetRGB(void){
		return MINT_RGB(r, g, b);
	}
};


#endif
