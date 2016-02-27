/*

  정형화되지 않은 최단 거리 검색 맵 모델
  --------------------------------------

  1999/10/26, lee jang ho
  All copyright (c) 1998, MAIET entertainment software, inc

*/
#ifndef MFreeMapModel_H
#define MFreeMapModel_H

#include "CMList.h"
#include "CMPtrList.h"
#include "MAStar.h"
#include <list>

using namespace std;

/*
	세그먼트는 Right-Hand의 방향성을 가진다.

	   ->
	+-----+
	|  in |
	|     | out
	+-----+
	  <-
*/


/*
class MFMPoint;
typedef list<MFMPoint*>		MPNVList;
typedef MPNVList::iterator	MPNVListItor;
*/

class MFMPolygon;
class MFreeMapModel;

// 검색 가능한 Point Node
// 폴리곤의 각 모서리가 검색의 노드가 된다.
class MFMPoint : public MNodeModel{
protected:
friend class MFMPolygon;
friend class MFreeMapModel;
	MFreeMapModel*		m_pParentMap;
	float				m_fX, m_fY;
	CMPtrList<MFMPoint>	m_VisibleList;		// 보이는 점 리스트
	int					m_nLastChangeCount;	// 최근까지 반영이 된 Map의 ChangeCount
	MFMPoint*			m_pPrevPoint;		// Polygon에 속했을때 이전 포인트
	MFMPoint*			m_pNextPoint;		// Polygon에 속했을때 이후 포인트
	CMPtrList<MFMPoint>	m_VisibledList;		// 이점이 보이지는 다른 점 리스트
	void*				m_pTemp;			// 임시 포인터 ( 보장받지 않는다. )

protected:
	bool IsParentVisiblePoint(MFMPoint* p);	// Parent Point(Path설정중에..)에서 p가 보이는가 테스트
	void GenerateVisiblePoint(void);		// 보이는 점 찾기
	void ResetVisibleList(void);
	void AddVisiblePoint(MFMPoint* pPoint);

public:
	MFMPoint(float x, float y);
	MFMPoint(void);

	inline float GetX(void){ return m_fX; }
	inline float GetY(void){ return m_fY; }

	MFMPoint* GetPrevPoint(void){ return m_pPrevPoint; }
	MFMPoint* GetNextPoint(void){ return m_pNextPoint; }

	int GetVisiblePointCount(void);
	MFMPoint* GetVisiblePoint(int i);

	void ResetConnectPoint(void);
	bool IsVisible(MFMPoint* pPN);

	virtual int GetSuccessorCount(void);
	virtual MNodeModel* GetSuccessor(int i);

	virtual float GetSuccessorCost(MNodeModel* pSuccessor);
	virtual float GetHeuristicCost(MNodeModel* pNode);
	

	//virtual float GetTotalCostPriority(void){ return m_fCostToGoal; }	// Goal까지의 Cost만을 고려한다.
};


enum MFMPOLYGONTYPE{
	MFMPT_NORMAL = 0,		// 특정 모양이 없는 폴리곤
	MFMPT_RECT,				// 직사각형 모양의 폴리곤, 보이는 폴리곤 추출시에 잇점을 제공해 준다.
};

// 폴리곤의 방향
enum MFMPOLYGONDIR{
	MFMPD_NA = 0,				// Not Available
	MFMPD_CLOCKWISE = 1,		// Clockwise
	MFMPD_COUNTERCLOCKWISE = 2,	// Counter-Clockwise
};

// 폴리곤 장애물
// Right-Handed이므로, 반대편으로는 이동 가능하다.
class MFMPolygon{
public:
	CMPtrList<MFMPoint>		m_PointList;		// 각 모서리
	bool					m_bEnclosed;		// 폴린곤이 닫혀 있는가?
	MFMPOLYGONTYPE			m_nType;			// 타입
	bool					m_bInnerStartPoint;	// 목표 지점이 이 폴리곤 안쪽에 속해 있는 경우.
	bool					m_bInnerEndPoint;	// 목표 지점이 이 폴리곤 안쪽에 속해 있는 경우.
	MFMPOLYGONDIR			m_nPolygonDir;		// 폴리곤의 방향 ( 폴리곤이 닫혀 있는 경우만, m_bEnclosed==true )
protected:
	MFMPOLYGONDIR TestPolygonDir(void);			// 폴리곤 방향 테스트
public:
	MFMPolygon(void);
	// Rect Type의 Polygon 생성
	MFMPolygon(MFMPoint* lt, MFMPoint* rt, MFMPoint* rb, MFMPoint* lb);

	void Add(MFMPoint* pPointNode);
	void Delete(int i);
	void Enclose(bool bEnclosed);
	// 어떤 점이 이 폴리곤 안에 갇혀 있는가?
	bool IsInnerPoint(float x, float y, bool bTestClockwise);
	MFMPOLYGONDIR IsInnerPoint(float* t, float x, float y);

	MFMPoint* GetStartPoint(void);
	MFMPoint* GetEndPoint(void);
	MFMPoint* Get(int i);

	void Add(MFMPolygon* pPO);

	void GetPointNormal(float* x, float* y, int i);
	void Enlarge(float fMargin);

	void RebuildConnection(void);

	bool IsIntersect(MFMPoint** p1, MFMPoint** p2, float* t, float x1, float y1, float x2, float y2);	// p1->p2?
	float GetNearestContactPoint(float* px, float* py, float x, float y, bool bInverse=false);			// 가장 가까운 폴리곤 라인위에 점
	//float GetContactPointByLineTest(float* px, float* py, float x, float y, int nTestCount);			// 라인을 nTestCount만큼 그어봐서 가장 가까운 점 찾기, 가장 가까운 점을 찾긴 어렵지만... 제한된 테스트 횟수를 보장한다.

	MFMPOLYGONDIR GetDir(void);		// Winding Dir
};

// Point 추가시 폴리곤을 끝을 명시.
enum MFMPOLYGONADDTYPE{
	MFMPAT_CONTINUE = 0,	// 계속 폴리곤이 이어진다.
	MFMPAT_END,			// 폴리곤을 끝낸다.
	MFMPAT_ENCLOSE,		// 폴리곤을 닫고 끝낸다.
};

// PreparePathFinding()의 Return Value
enum MFMRESULT{
	MFMR_OK,
	MFMR_ENDPOINTCHANGED,
};

// 정형화되지 않은 맵 모델
class MFreeMapModel{
protected:
	MFMPolygon*					m_pCurrPolygon;			// Point가 추가되는 현재 폴리곤
	bool						m_bEndPointInPolygon;	// 끝 점이 어떤 폴리곤에 갇혀 있는가?
	int							m_nChangeCount;			// 맵이 변경된 카운트(변경 됐는지 알 수 있다.)
public:
	MFMPoint*					m_pStartPoint;			// 시작 점
	MFMPoint*					m_pEndPoint;			// 끝 점
	CMLinkedList<MFMPoint>		m_PointList;
	CMLinkedList<MFMPolygon>	m_PolygonList;

protected:
	void Change(void);									// 맵의 변화 지정
	void GetInnerClosedPolygon(CMPtrList<MFMPolygon>* pPolygons, int nPointType, MFMPOLYGONDIR dir);	// nPointType=0 Start, nPointType=1 End Point

	bool TestEndPointInPolygon(void);					// 끝점이 폴리곤 안에 갇혀 있는지 테스트, 미리 테스트함으로써 써치 타임에 성능 항상을 꽤할수 있다.
	bool TestStartPointInPolygon(void);					// 시작점이 폴리곤 안에 갇혀 있는지 테스트, 미리 테스트함으로써 써치 타임에 성능 항상을 꽤할수 있다.

	// 폴리곤 리스트중에 Non-Intersect하는 지점 추천(sx,sy 시작점, ex,ey 끝점)
	bool FindNonIntersectPosition(CMPtrList<MFMPolygon>* pPolygons, float* rx, float* ry, float sx, float sy, float ex, float ey);
	// 폴리곤 리스트중에 Non-Intersect하는 지점 추천(x,y에서 가까운 점)
	bool FindNonIntersectPosition(CMPtrList<MFMPolygon>* pPolygons, float* rx, float* ry, float x, float y);

	bool TestPointInPolygon(float* pNearestPointX, float* pNearestPointY, float x, float y);		// 갇혀있는지 테스트후, 가장 가까운 외부 지점 리턴
	bool RecommendPoint(float* pRecommendedPointX, float* pRecommendedPointY, float x, float y, float fCheckStepDistance, float fExponentialIncrease=1.0f);	// 충돌하지 않는 지점으로 추천
	bool RecommendEndPoint(float* pRecX, float* pRecY);	// 충돌하지 않는 지점으로 추천(return true면 재지정, 따라서.. TestEndPointInPolygon()을 해줘야 한다.

	friend class MFMPoint;
	void ClearVisibledList(void);				// MFMPoint::m_VisibledList Clear

	bool IsVisibleEdgePoint(MFMPoint* p1, MFMPoint* p2);												// p1->p2?	( Edge만 검사 )
	int IsBiDirectionVisiblePoint(float* pT1, float* pT2, MFMPoint* p1, MFMPoint* p2);					// 0: 없음, 1: p1->p2, 2:p2->p1, 3:p1<->p2

public:
	MFreeMapModel(void);
	virtual ~MFreeMapModel(void);

	void Destroy(void);

	MFMPoint* AddPoint(float x, float y, bool bCheckDup=false);							// 일반점 추가 ( Point가 겹칠 수 있는 경우 중복 체크를 한다. )
	MFMPolygon* AddPolygon(MFMPoint* pNode, MFMPOLYGONADDTYPE nType=MFMPAT_CONTINUE);	// 현재 폴리곤에 점 추가
	MFMPolygon* AddRect(float x1, float y1, float x2, float y2, bool bInverse=false);	// 직 사각형 형태의 폴리곤 추가

	MFMPoint* AddStartPoint(float x, float y);											// 시작점 추가
	MFMPoint* AddEndPoint(float x, float y);											// 끝 점 추가
	MFMPoint* GetStartPoint(void);
	MFMPoint* GetEndPoint(void);

	void SetStartPoint(float x, float y);												// 시작점
	void SetEndPoint(float x, float y);													// 끝 점

	// Low-Level Addition
	void AddPoint(MFMPoint* pPoint);													// 포인트 추가
	void AddPolygon(MFMPolygon* pPolygon);												// 폴리곤 추가
	void AddStartPoint(MFMPoint* pPoint);												// 시작점 추가
	void AddEndPoint(MFMPoint* pPoint);													// 끝 점 추가

	void Clear(void);																	// 추가된 Point, Polygon 초기화

	MFMRESULT PreparePathFinding(float* pRecX, float* pRecY);							// Path Finding Prepare ( End Point를 적절히 위치시킨다.)

	bool IsEndPointInPolygon(void);														// 끝 점이 폴리곤에 갇혀 있는가?
	void MergeBreakPolygon(void);														// 서로 다른 열려있는(MFMPAT_END) 폴리곤들중 시작점이나 끝점이 겹쳐서 연결이 가능한 폴리곤 합치기

	MFMPolygon* IsInnerClosedPolygon(float x, float y, bool bTestClockwise=true);		// 점이 폴리곤 안에 속해 있는가?
	void IsInnerClosedPolygon(CMPtrList<MFMPolygon>* pPolygons, float x, float y, bool bTestClockwise=true);	// 점이 폴리곤 안에 속해 있는가?

	bool IsIntersect(MFMPoint** p1, MFMPoint** p2, float* t, float x1, float y1, float x2, float y2);			// p1->p2?

	MFMPolygon* GetNearestInnerPolygon(CMLinkedList<MFMPolygon>* pPolygons, float x, float y);	// Inner Polygon중 가장 가까이에 있는 폴리곤( 방향에 상관 없음 ) (1, 0, 0) 방향이다.

	bool IsEnable(CMLinkedList<MFMPolygon>* pPolygons, float x, float y);	// Enable Position인가?
	bool IsEnable(float x, float y);									// Enable Position인가?

	void Add(MFreeMapModel* pNGMM);

	void Enlarge(float fMargin);

	int GetChangeCount(void);
};

#endif
