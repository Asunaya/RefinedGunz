#ifndef MUTIL_H
#define MUTIL_H

#include "rutils.h"

// 일부 라인을 Pos, Dir(Normalize가 아닌 라인 길이 만큼)으로 구성, 이는 연산을 줄이기 위함이다.

// 3D에서 벡터와 구가 교차하는지 테스트. Dir는 라인을 의미함
bool MIsIntersectLineAndCircle(float* t, rvector& Pos, rvector& Dir, rvector& CircleCenter, float fCircleRadius);

// Point에서 Line로 수선을 내릴때 그 포인트 구하기
// pT : LineStart->LineEnd 사이의 값 (0~1.0)
void MGetPointFromPointToLine(rvector* pContactPoint, float* pT, rvector& Point, rvector& LineStart, rvector& LineEnd);

// 2D에서 어떤 점과 원의 접선의 접점 구하기
void MGetContactPoint(rvector ContactPoint[2], rvector& Point, rvector& CircleCenter, float fCircleRadius);

// 2D에서 라인과 라인이 겹치는지 구함
bool MIsIntersectLineAndLine(float *pT1, float *pT2, rvector& Pos, rvector& Dir, rvector& s1, rvector& s2);

// 2D에서 라인과 라인이 겹치는지 구함
bool MIsIntersectLineAndHalfLine(float *pT1, float *pT2, rvector& Pos, rvector& Dir, rvector& s1, rvector& s2);

// 2D에서 라인과 사각형이 겹치는지 구함
int MIsIntersectLineAndRectangle(float *pT,	// 매개 변수 리턴 ( 0.0 ~ 1.0 )
										 rvector& Pos, rvector& Dir,	// Line
										 float fLeft, float fTop, float fRight, float fBottom);	// Rect

/*
            (Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy)
        r = -----------------------------  (eqn 1)
            (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
            (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
        s = -----------------------------  (eqn 2)
            (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
*/

bool MFGetIntersectOfSegmentAndSegment(float *pT1, float *pT2, float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Dx, float Dy);
bool MFIsPointInSegment(float sx1, float sy1, float sx2, float sy2, float x, float y);
bool MFIsVisiblePoint(float x1, float y1, float x2, float y2, float x3, float y3, float px, float py);
bool MFIsEdgePoint(float x1, float y1, float x2, float y2, float x3, float y3, float px, float py);

#endif