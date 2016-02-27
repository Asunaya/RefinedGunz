#include "MUtil.h"

#define square(_f)		(_f*_f)
/*
bool MIsIntersectLineAndCircle(float* t, rvector& Pos, rvector& Dir, rvector& CircleCenter, float fCircleRadius)
{

	float Tx = Pos.x - CircleCenter.x;
	float Ty = Pos.y - CircleCenter.y;
	float Tz = Pos.z - CircleCenter.z;

	float A = square(Dir.x) + square(Dir.y) + square(Dir.z);
	float B = 2 * (Dir.x*Tx + Dir.y*Ty + Dir.z*Tz);
	float C = square(Tx) + square(Ty) + square(Ty) - square(fCircleRadius);

	float D = square(B) - 4 * A * C;	// 판별식
	if(A<0.0f && D<0.0f){			// ??? 문제의 부분이다. 레이트레이싱 책에는 단순히 D가 0 이하일 경우만이라고 한다.
		*t = 0;
		return true;
	}
	if(D<0.0f) return false;			// 겹치는 점이 없음

	float sqrtD = (float)sqrt(D);
	float T1 = ( -B + sqrtD ) / (2 * A);
	float T2 = ( -B - sqrtD ) / (2 * A);

	// t가 양수인 경우만.

	if(T1<0.0f && T2<0.0f) return false;

	if(T1>=0.0f && T2>=0.0f) *t = min(T1, T2);
	else *t = max(T1, T2);

	return true;
}
*/
bool MIsIntersectLineAndCircle(float* t, rvector& Pos, rvector& Dir, rvector& CircleCenter, float fCircleRadius)
{
	rvector Diff = Pos - CircleCenter;
	float A = DotProduct(Dir, Dir);
	float B = 2 * DotProduct(Dir, (Diff));
	float C = DotProduct(Diff, Diff) - square(fCircleRadius);

	float D = square(B) - 4 * A * C;
	if(D<0.0f){
		*t = 0;
		return false;
	}

	float sqrtD = 0;
	if(D>0) sqrtD = (float)sqrt(D);
	float T1 = ( -B + sqrtD ) / (2 * A);
	float T2 = ( -B - sqrtD ) / (2 * A);

	if(T1<0.0f && T2<0.0f) return false;

	//if(T1>=0.0f && T2>=0.0f) *t = min(T1, T2);
	if(T1>0.0f && T2>0.0f) *t = min(T1, T2);
	else *t = max(T1, T2);

	return true;
}

// Point에서 Line로 수선을 내릴때 그 포인트 구하기
// pT : LineStart->LineEnd 사이의 값 (0~1.0)
void MGetPointFromPointToLine(rvector* pContactPoint, float* pT, rvector& Point, rvector& LineStart, rvector& LineEnd)
{
	rvector Line = LineEnd - LineStart;
	rvector ToPoint = Point - LineStart;
	float LineLengthPow = Line.x*Line.x + Line.y*Line.y + Line.z*Line.z;
	float t = DotProduct(Line, ToPoint) / LineLengthPow;
	t *= 1.5f;
	*pContactPoint = LineStart + Line * t;
	*pT = t;
}

// 2D에서 어떤 점과 원의 접선의 접점 구하기
void MGetContactPoint(rvector ContactPoint[2], rvector& Point, rvector& CircleCenter, float fCircleRadius)
{
	rvector ToCircle = CircleCenter - Point;
	float fLengthToCircle = (float)sqrt(ToCircle.x*ToCircle.x + ToCircle.y*ToCircle.y);
	float fAngle = (float)asin(min(max(fCircleRadius/fLengthToCircle, -1.0f), 1.0f));

	rvector LineEnd;
	float t;
	
	rvector ToContact1 = TransformVector(ToCircle, RotateZMatrix(fAngle));
	LineEnd = Point + ToContact1;
	MGetPointFromPointToLine(&(ContactPoint[0]), &t, CircleCenter, Point, LineEnd);	// Left

	rvector ToContact2 = TransformVector(ToCircle, RotateZMatrix(-fAngle));
	LineEnd = Point + ToContact2;
	MGetPointFromPointToLine(&(ContactPoint[1]), &t, CircleCenter, Point, LineEnd);	// Right
}


// 2D에서 어떤 점과 원의 접선의 접점 구하기
void MGetLeftContactPoint(rvector* pLeftContactPoint, rvector& Point, rvector& CircleCenter, float fCircleRadius)
{
	rvector ToCircle = CircleCenter - Point;
	float fLengthToCircle = (float)sqrt(ToCircle.x*ToCircle.x + ToCircle.y*ToCircle.y);
	float fAngle = (float)asin(min(max(fCircleRadius/fLengthToCircle, -1.0f), 1.0f));

	rvector LineEnd;
	float t;
	
	rvector ToContact1 = TransformVector(ToCircle, RotateZMatrix(fAngle));
	LineEnd = Point + ToContact1;
	MGetPointFromPointToLine(pLeftContactPoint, &t, CircleCenter, Point, LineEnd);	// Left
}

// 2D에서 어떤 점과 원의 접선의 접점 구하기
void MGetRightContactPoint(rvector* pRightContactPoint, rvector& Point, rvector& CircleCenter, float fCircleRadius)
{
	rvector ToCircle = CircleCenter - Point;
	float fLengthToCircle = (float)sqrt(ToCircle.x*ToCircle.x + ToCircle.y*ToCircle.y);
	float fAngle = (float)asin(min(max(fCircleRadius/fLengthToCircle, -1.0f), 1.0f));

	rvector LineEnd;
	float t;
	
	rvector ToContact2 = TransformVector(ToCircle, RotateZMatrix(-fAngle));
	LineEnd = Point + ToContact2;
	MGetPointFromPointToLine(pRightContactPoint, &t, CircleCenter, Point, LineEnd);	// Right
}


/*
            (Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy)
        r = -----------------------------  (eqn 1)
            (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
            (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
        s = -----------------------------  (eqn 2)
            (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
*/
bool MIsIntersectLineAndLine(float *pT1, float *pT2, float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Dx, float Dy)
{
	float T1 = Bx-Ax;
	float T2 = Dy-Cy;
	float T3 = By-Ay;
	float T4 = Dx-Cx;
	float T5 = Ay-Cy;
	float T6 = Ax-Cx;

	float D = T1*T2 - T3*T4;

	if(D==0) return false;

	float t1 = (T5*T4 - T6*T2) / (float) D;
	float t2 = (T5*T1 - T6*T3) / (float) D;
	//if(t1>=0.0f && t1<=1.0f && t2>=0.0f && t2<=1.0f){
	if(t1>0.0f && t1<1.0f && t2>0.0f && t2<1.0f){
	//if(t1>=0.0f && t1<=1.0f && t2>0.0f && t2<1.0f){		// 타겟 선분의 양 끝은 포함시키지 않음.
		*pT1 = t1;
		*pT2 = t2;
		/*
		float x1 = -T6;
		float y1 = -T5;
		float x2 = Dx - Bx;
		float y2 = Dy - By;
		float z = x1*y2 - y1*x2;
		if(z>=0) return true;
		*/
		return true;
	}

	return false;
}

#define min(a, b)  (((a) < (b)) ? (a) : (b)) 

int MIsIntersectLineAndRectangle(float *pT,	// 매개 변수 리턴 ( 0.0 ~ 1.0 )
										 float fXa1, float fYa1, float fXa2, float fYa2,// Line
										 float fLeft, float fTop, float fRight, float fBottom)	// Rect
{
 	bool bIntersection;
	float fNearestT = 2.0f;
	float t[4];
	/*

	+-- x
	|
	y

          3
		+---+
	  0	|   | 2
		+---+
		  1
	*/
	float t2[4];
	if(fXa2>fXa1){
		bIntersection = MIsIntersectLineAndLine(&t[0], &t2[0], fXa1, fYa1, fXa2, fYa2, fLeft, fTop, fLeft, fBottom);
		if(bIntersection==true) fNearestT = min(fNearestT, t[0]);
	}
	if(fYa2<fYa1){
		bIntersection = MIsIntersectLineAndLine(&t[1], &t2[0], fXa1, fYa1, fXa2, fYa2, fLeft, fBottom, fRight, fBottom);
		if(bIntersection==true) fNearestT = min(fNearestT, t[1]);
	}
	if(fXa2<fXa1){
		bIntersection = MIsIntersectLineAndLine(&t[2], &t2[0], fXa1, fYa1, fXa2, fYa2, fRight, fBottom, fRight, fTop);
		if(bIntersection==true) fNearestT = min(fNearestT, t[2]);
	}
	if(fYa2>fYa1){
		bIntersection = MIsIntersectLineAndLine(&t[3], &t2[0], fXa1, fYa1, fXa2, fYa2, fRight, fTop, fLeft, fTop);
		if(bIntersection==true) fNearestT = min(fNearestT, t[3]);
	}

	if(fNearestT<=1.0f){
		*pT = fNearestT;
		if(fNearestT==t[0]) return 0;
		else if(fNearestT==t[1]) return 1;
		else if(fNearestT==t[2]) return 2;
		else if(fNearestT==t[3]) return 3;
	}

	return -1;
}


/*
            (Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy)
        r = -----------------------------  (eqn 1)
            (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
            (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
        s = -----------------------------  (eqn 2)
            (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
*/
bool MIsIntersectLineAndLine(float *pT1, float *pT2, rvector& Pos, rvector& Dir, rvector& s1, rvector& s2)
{
	float Bx = Pos.x + Dir.x;
	float By = Pos.y + Dir.y;
	float T1 = Bx-Pos.x;
	float T2 = s2.y-s1.y;
	float T3 = By-Pos.y;
	float T4 = s2.x-s1.x;
	float T5 = Pos.y-s1.y;
	float T6 = Pos.x-s1.x;

	float D = T1*T2 - T3*T4;

	if(D==0) return false;

	float t1 = (T5*T4 - T6*T2) / (float) D;
	float t2 = (T5*T1 - T6*T3) / (float) D;

	*pT1 = t1;
	*pT2 = t2;
	if(t1>=0.0f && t1<=1.0f && t2>=0.0f && t2<=1.0f){
		return true;
	}

	return false;
}

bool MIsIntersectLineAndHalfLine(float *pT1, float *pT2, rvector& Pos, rvector& Dir, rvector& s1, rvector& s2)
{
	float Bx = Pos.x + Dir.x;
	float By = Pos.y + Dir.y;
	float T1 = Bx-Pos.x;
	float T2 = s2.y-s1.y;
	float T3 = By-Pos.y;
	float T4 = s2.x-s1.x;
	float T5 = Pos.y-s1.y;
	float T6 = Pos.x-s1.x;

	float D = T1*T2 - T3*T4;

	if(D==0) return false;

	float t1 = (T5*T4 - T6*T2) / (float) D;
	float t2 = (T5*T1 - T6*T3) / (float) D;

	*pT1 = t1;
	*pT2 = t2;
	if(t1>=0.0f && t2>=0.0f && t2<=1.0f){
		return true;
	}

	return false;
}

#define min(a, b)  (((a) < (b)) ? (a) : (b)) 

int MIsIntersectLineAndRectangle(float *pT,	// 매개 변수 리턴 ( 0.0 ~ 1.0 )
										 rvector& Pos, rvector& Dir,	// Line
										 float fLeft, float fTop, float fRight, float fBottom)	// Rect
{
 	bool bIntersection;
	float fNearestT = 2.0f;
	float t[4];
	/*

	+-- x
	|
	y

          3
		+---+
	  0	|   | 2
		+---+
		  1
	*/
	float fXa1 = Pos.x;
	float fYa1 = Pos.y;
	float fXa2 = Pos.x + Dir.x;
	float fYa2 = Pos.y + Dir.y;

	float t2[4];
	if(fXa2>fXa1){
		bIntersection = MIsIntersectLineAndLine(&t[0], &t2[0], Pos, Dir, rvector(fLeft, fTop, 0), rvector(fLeft, fBottom, 0));
		if(bIntersection==true) fNearestT = min(fNearestT, t[0]);
	}
	if(fYa2<fYa1){
		bIntersection = MIsIntersectLineAndLine(&t[1], &t2[0], Pos, Dir, rvector(fLeft, fBottom, 0), rvector(fRight, fBottom, 0));
		if(bIntersection==true) fNearestT = min(fNearestT, t[1]);
	}
	if(fXa2<fXa1){
		bIntersection = MIsIntersectLineAndLine(&t[2], &t2[0], Pos, Dir, rvector(fRight, fBottom, 0), rvector(fRight, fTop, 0));
		if(bIntersection==true) fNearestT = min(fNearestT, t[2]);
	}
	if(fYa2>fYa1){
		bIntersection = MIsIntersectLineAndLine(&t[3], &t2[0], Pos, Dir, rvector(fRight, fTop, 0), rvector(fLeft, fTop, 0));
		if(bIntersection==true) fNearestT = min(fNearestT, t[3]);
	}

	if(fNearestT<=1.0f){
		*pT = fNearestT;
		if(fNearestT==t[0]) return 0;
		else if(fNearestT==t[1]) return 1;
		else if(fNearestT==t[2]) return 2;
		else if(fNearestT==t[3]) return 3;
	}

	return -1;
}

bool MFGetIntersectOfSegmentAndSegment(float *pT1, float *pT2, float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Dx, float Dy)
{
	float T1 = Bx-Ax;
	float T2 = Dy-Cy;
	float T3 = By-Ay;
	float T4 = Dx-Cx;
	float T5 = Ay-Cy;
	float T6 = Ax-Cx;

	float D = T1*T2 - T3*T4;

	if(D==0) return false;

	float fDD = 1.0f / (float) D;
	float t1 = (T5*T4 - T6*T2) * fDD;
	float t2 = (T5*T1 - T6*T3) * fDD;
	if(t1>=0.0f && t1<=1.0f && t2>=0.0f && t2<=1.0f){
	//if(t1>0.0f && t1<1.0f && t2>0.0f && t2<1.0f){
	//if(t1>=0.0f && t1<=1.0f && t2>0.0f && t2<1.0f){		// 타겟 선분의 양 끝은 포함시키지 않음.
		*pT1 = t1;
		*pT2 = t2;
		return true;
	}

	return false;
}

bool MFIsPointInSegment(float sx1, float sy1, float sx2, float sy2, float x, float y)
{
	float x1 = sx1 - x;
	float y1 = sy1 - y;
	float x2 = sx2 - x;
	float y2 = sy2 - y;
	float z1 = x1*y2 - y1*x2;
	if(z1>0) return true;
	return false;
}
bool MFIsVisiblePoint(float x1, float y1, float x2, float y2, float x3, float y3, float px, float py)
{
	float sx1 = x1 - x2;
	float sy1 = y1 - y2;
	float sx2 = px - x2;
	float sy2 = py - y2;
	float z1 = sx1*sy2 - sy1*sx2;
	float sx3 = px - x2;
	float sy3 = py - y2;
	float sx4 = x3 - x2;
	float sy4 = y3 - y2;
	float z2 = sx3*sy4 - sy3*sx4;

	float sx5 = x1 - x2;
	float sy5 = y1 - y2;
	float sx6 = x3 - x2;
	float sy6 = y3 - y2;
	if(sx5*sy6-sy5*sx6<=0){
		if(z1<=0 && z2<=0) return false;	// 안쪽이면 보이지 않는다.
	}
	else{
		if(z1<=0 || z2<=0) return false;	// 안쪽이면 보이지 않는다.
	}
	return true;
}

bool MFIsEdgePoint(float x1, float y1, float x2, float y2, float x3, float y3, float px, float py)
{
	float sx1 = x1 - x2;
	float sy1 = y1 - y2;
	float sx2 = px - x2;
	float sy2 = py - y2;
	float z1 = sx1*sy2 - sy1*sx2;
	float sx3 = px - x2;
	float sy3 = py - y2;
	float sx4 = x3 - x2;
	float sy4 = y3 - y2;
	float z2 = sx3*sy4 - sy3*sx4;

	if((z1>0 && z2<0) || (z1<0 && z2>0)) return true;
	return false;
}

