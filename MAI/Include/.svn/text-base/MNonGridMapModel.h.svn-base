/*

  정형화되지 않은 최단 거리 검색 맵 모델
  --------------------------------------

  1999/10/26, lee jang ho
  All copyright (c) 1998, MAIET entertainment software, inc

*/
#ifndef MNONGRIDMAPMODEL_H
#define MNONGRIDMAPMODEL_H

#include "CMList.h"
#include "CMPtrList.h"
#include "MAStar.h"

/*
	세그먼트는 Right-Hand의 방향성을 가진다.

	   ->
	+-----+
	|  in |
	|     | out
	+-----+
	  <-
*/


// 검색 가능한 Point Node
// 폴리곤의 각 모서리가 검색의 노드가 된다.
class MPointNode : public MNodeModel{
public:
	float		m_fX, m_fY;
	CMPtrList<MPointNode>	m_VisibleList;	// 보이는 점 리스트
	MPointNode*		m_pPreviousPoint;		// Polygon에 속했을때 이전 포인트
	MPointNode*		m_pNextPoint;			// Polygon에 속했을때 이후 포인트
	void*			m_pTemp;				// 임시 포인터 ( 보장받지 않는다. )
	bool			m_bPreCalcVisiblePoint;
public:
	MPointNode(float x, float y);
	MPointNode(void);
	void ResetVisibleList(void);
	void ResetConnectPoint(void);
	bool IsVisible(MPointNode* pPN);
	virtual int GetSuccessorCount(void);
	virtual MNodeModel* GetSuccessor(int i);
	virtual float GetSuccessorCost(MNodeModel* pSuccessor);
	virtual float GetHeuristicCost(MNodeModel* pNode);
};


enum MPOLYGONTYPE{
	MPT_NORMAL = 0,		// 특정 모양이 없는 폴리곤
	MPT_RECT,			// 직사각형 모양의 폴리곤, 보이는 폴리곤 추출시에 잇점을 제공해 준다.
};


// 폴리곤 장애물
// Right-Handed이므로, 반대편으로는 이동 가능하다.
class MPolygonObstacle{
public:
	CMPtrList<MPointNode>	m_PointList;		// 각 모서리
	bool					m_bEnclosed;		// 폴린곤이 닫혀 있는가?
	MPOLYGONTYPE			m_nType;			// 타입
	bool					m_bInnerGoal;		// 목표 지점이 이 폴리곤 안쪽에 속해 있는 경우.
public:
	MPolygonObstacle(void);
	// Rect Type의 Polygon 생성
	MPolygonObstacle(MPointNode* lt, MPointNode* rt, MPointNode* rb, MPointNode* lb);
	void Add(MPointNode* pPointNode);
	void Delete(int i);
	void Enclose(bool bEnclosed);
	// 어떤 점이 이 폴리곤 안에 갇혀 있는가?
	bool IsInnerPoint(float x, float y);

	MPointNode* GetStartPoint(void);
	MPointNode* GetEndPoint(void);
	MPointNode* Get(int i);

	void Add(MPolygonObstacle* pPO);

	void GetPointNormal(float* x, float* y, int i);
	void Shrink(float fMargin);
};

// Point 추가시 폴리곤을 끝을 명시.
enum MPolygonObstacleADDTYPE{
	MPAT_CONTINUE = 0,	// 계속 폴리곤이 이어진다.
	MPAT_END,			// 폴리곤을 끝낸다.
	MPAT_ENCLOSE,		// 폴리곤을 닫고 끝낸다.
};


// 정형화되지 않은 맵 모델
// GenerateVisiblePoint()를 한 후에 최단 거리 검색을 해야 한다.
class MNonGridMapModel{
protected:
	MPolygonObstacle*	m_pCurrPolygon;			// Point가 추가되는 현재 폴리곤
	MPointNode*			m_pStartPoint;			// 시작 점
	MPointNode*			m_pEndPoint;			// 끝 점
	bool				m_bEndPointInPolygon;	// 끝 점이 어떤 폴리곤에 갇혀 있는가?
public:
	CMLinkedList<MPointNode>		m_PointList;
	CMLinkedList<MPolygonObstacle>	m_PolygonList;

protected:
	//bool IsVisiblePoint(MPointNode* p1, MPointNode* p2);
	int IsBiDirectionVisiblePoint(MPointNode* p1, MPointNode* p2);	// 0: 없음, 1: p1->p2, 2:p2->p1, 3:p1<->p2
public:
	MNonGridMapModel(void);
	void Destroy(void);

	MPointNode* AddPoint(float x, float y, bool bCheckDup=false);			// 일반점 추가 ( Point가 겹칠 수 있는 경우 중복 체크를 한다. )
	MPointNode* AddStartPoint(float x, float y);	// 시작점 추가
	MPointNode* AddEndPoint(float x, float y);		// 끝 점 추가
	MPolygonObstacle* AddPolygon(MPointNode* pNode, MPolygonObstacleADDTYPE nType=MPAT_CONTINUE);	// 현재 폴리곤에 점 추가
	MPolygonObstacle* AddRect(float x1, float y1, float x2, float y2, bool bInverse=false);	// 직 사각형 형태의 폴리곤 추가
	void GenerateVisiblePoint(void);			// 각 점에서 보이는 점 찾기
	void Reset(void);							// 초기화
	bool IsEndPointInPolygon(void);				// 끝 점이 폴리곤에 갇혀 있는가?
	void MergeBreakPolygon(void);				// 서로 다른 열려있는(MPAT_END) 폴리곤들중 시작점이나 끝점이 겹쳐서 연결이 가능한 폴리곤 합치기

	void Add(MNonGridMapModel* pNGMM, bool bVisibleTest=false);

	void Shrink(float fMargin);
};

#endif
