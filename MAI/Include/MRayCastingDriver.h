#ifndef MRAYCASTINGDRIVER_H
#define MRAYCASTINGDRIVER_H

#include "rutils.h"
#include "CMList.h"
#include "CMPtrList.h"

// 내부점에 기준한 추천 외부점에 곱해지는 상수. MObstacle::GetOutPos()
// 1.0f은 여유가 없이 정확하기 때문에, 외부점으로 패스파인딩 하는데 문제가 생긴다.
void SetOutPosConstant(float fConstant = 1.1f);

class MObstacle{
public:
	rvector					m_Pos;
	CMPtrList<MObstacle>	m_OverlappedObstacles;
	int						m_nTempUsed;
public:
	MObstacle(rvector& Pos);
	virtual ~MObstacle(void);
	virtual rvector GetOutPos(rvector& Pos) = 0;							// Pos방향으로 가장 가까운 외부점 구하기
	virtual bool IsCollision(rvector& Pos) = 0;								// 한점이 장애물에 속해 있는가?
	virtual bool IsCollision(rvector* pNewPos, rvector& Pos);				// 한점이 장애물에 속해 있는가?
	virtual bool IsIntersect(float* t, rvector& Pos, rvector& Dir, float fMagnitude) = 0;	// 교차 여부
	virtual void GetEdge(rvector* pLeft, rvector* pRight, rvector& Pos) = 0;				// Pos에서 바라보는 Edge 구하기
	bool IsOverlap(MObstacle* pObstacle);													// 두 Obstacle이 겹쳐지는가?
	virtual bool GetDiffFromSurface(rvector* pDiff, rvector& p) = 0;						// 외곽에서의 p까지 거리 구하기
	virtual float GetDistance(MObstacle* pObstacle) = 0;									// Obstacle과의 표면에서의 거리 구하기

	rvector GetCenter(void);				// 중심 위치 ( m_Pos ) 얻기
	virtual float GetArea(void) = 0;		// 면적 구하기

	virtual void EnLarge(float fAdd) = 0;	// fAdd만큼 면적 키우기

	void CheckOverlappedObstacleTemp(int nTemp);

	// 겹쳐진 오브젝트들을 포함한 좌우 Edge 구하기
	void GetOverlappedEdge(rvector* pLeft, rvector* pRight, rvector& Pos, CMPtrList<MObstacle>* pLeftObsList=NULL, CMPtrList<MObstacle>* pRightObsList=NULL);
protected:
	MObstacle* GetOverlappedLeftEdge(rvector* pLeft, rvector& Pos, MObstacle* pAncestor=NULL, CMPtrList<MObstacle>* pObsList=NULL);
	MObstacle* GetOverlappedRightEdge(rvector* pRight, rvector& Pos, MObstacle* pAncestor=NULL, CMPtrList<MObstacle>* pObsList=NULL);
};

class MSphereObstacle : public MObstacle{
};

class MBoxObstacle : public MObstacle{
};

// 2D 원
class MCircleObstacle : public MObstacle{
protected:
	float	m_fRadius;
public:
	MCircleObstacle(rvector& Pos, float fRadius);

	virtual rvector GetOutPos(rvector& Pos);							// Pos방향으로 가장 가까운 외부점 구하기
	virtual bool IsCollision(rvector& Pos);								// 한점이 장애물에 속해 있는가?
	virtual bool IsCollision(rvector* pNewPos, rvector& Pos);							// 한점이 장애물에 속해 있는가?
	virtual bool IsIntersect(float* t, rvector& Pos, rvector& Dir, float fMagnitude);	// 교차 여부
	virtual void GetEdge(rvector* pLeft, rvector* pRight, rvector& Pos);				// Pos에서 바라보는 Edge 구하기
	//virtual bool IsOverlap(MObstacle* pObstacle);										// 두 Obstacle이 겹쳐지는가?
	virtual bool GetDiffFromSurface(rvector* pDiff, rvector& p);						// 외곽에서의 p까지 거리 구하기 ( p가 외부에 있으면 true를 리턴 )
	virtual float GetDistance(MObstacle* pObstacle);									// Obstacle과의 표면에서의 거리 구하기

	virtual float GetArea(void);
	virtual void EnLarge(float fAdd);
};

// 2D 사각형
class MRectangleObstacle : public MObstacle{
protected:
	float	m_fWidth;		// 넓이 ( x )
	float	m_fHeight;		// 높이 ( y )

protected:
	int GetDivision(rvector& Pos);
public:
	MRectangleObstacle(rvector& Pos, float w, float h);
	MRectangleObstacle(float x1, float y1, float x2, float y2);

	virtual rvector GetOutPos(rvector& Pos);							// Pos방향으로 가장 가까운 외부점 구하기
	virtual bool IsCollision(rvector& Pos);								// 한점이 장애물에 속해 있는가?
	virtual bool IsIntersect(float* t, rvector& Pos, rvector& Dir, float fMagnitude);	// 교차 여부
	virtual void GetEdge(rvector* pLeft, rvector* pRight, rvector& Pos);				// Pos에서 바라보는 Edge 구하기
	virtual bool GetDiffFromSurface(rvector* pDiff, rvector& p);						// 외곽에서의 p까지 거리 구하기 ( p가 외부에 있으면 true를 리턴 )
	virtual float GetDistance(MObstacle* pObstacle);									// Obstacle과의 표면에서의 거리 구하기

	virtual float GetArea(void);
	virtual void EnLarge(float fAdd);
};

class MReverseRectangleObstacle{
public:
	rvector	m_Pos;
	float	m_fWidth;
	float	m_fHeight;
public:
	MReverseRectangleObstacle(rvector& p, float w, float h);
};

/*
class MTerrainObstacle : public MObstacle{
};
*/

// 시야에 보이는 장애물을 피해가는 패스 파인더
class MRayCastingDriver{
protected:
	MReverseRectangleObstacle*	m_pRRObstacle;
public:
	CMPtrList<MObstacle>		m_Obstacles;

protected:
	void CheckOverlappedObstacleTemp(MObstacle* p, int nTemp);
	void GetArea(float* pLeftArea, float* pRightArea, MObstacle* pColObstacle, rvector& Pos, rvector& Target);
	void GetArea(float* pLeftArea, float* pRightArea, MObstacle* pColObstacle, MObstacle* pAncestor, bool bAncestorLeft, rvector& Pos, rvector& Target, int nMagicNumber);
public:
	MRayCastingDriver(void);
	virtual ~MRayCastingDriver(void);

	void AddObstacle(MObstacle* pObstacle);			// 주위의 장애물로 추가
	void AddObstacle(CMPtrList<MObstacle>* pOL);	// 주위의 장애물로 추가
	void AddObstacle(CMLinkedList<MObstacle>* pOL);	// 주위의 장애물로 추가
	void DeleteObstacle(void);

	bool IsCollision(rvector* pNewPos, rvector& Pos);
	//MObstacle* IsCollision(rvector* pNewPos, rvector& Pos);

	void SetReverseRectangleObstacle(rvector& p, float w, float h);
	void RemoveReverseRectangleObstacle(void);

	rvector GetNextDir(rvector Pos, rvector Dir, float fMagnitude, rvector CurDir);
};

#endif
