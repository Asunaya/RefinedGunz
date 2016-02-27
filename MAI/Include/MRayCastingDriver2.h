#ifndef MRAYCASTINGDRIVER2_H
#define MRAYCASTINGDRIVER2_H

#include "MRayCastingDriver.h"

class MRayCastingDriver2{
public:
	CMPtrList<MObstacle>		m_Obstacles;

protected:
	void CheckOverlappedObstacleTemp(MObstacle* p, int nTemp);
	void GetArea(float* pLeftArea, float* pRightArea, MObstacle* pColObstacle, rvector& Pos, rvector& Target);
	void GetArea(float* pLeftArea, float* pRightArea, MObstacle* pColObstacle, MObstacle* pAncestor, bool bAncestorLeft, rvector& Pos, rvector& Target, int nMagicNumber);
public:
	MRayCastingDriver2(void);
	virtual ~MRayCastingDriver2(void);

	void AddObstacle(MObstacle* pObstacle);			// 주위의 장애물로 추가
	void AddObstacle(CMPtrList<MObstacle>* pOL);	// 주위의 장애물로 추가
	void AddObstacle(CMLinkedList<MObstacle>* pOL);	// 주위의 장애물로 추가
	void DeleteObstacle(void);

	bool IsCollision(rvector* pNewPos, rvector& Pos);

	bool FindPath(rvector& Pos, rvector& Dir, float fMagnitude);
};

#endif