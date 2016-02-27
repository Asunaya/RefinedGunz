/*

  3차원 폴리곤의 연속된 리스트로 구성되는 맵 모델
  평면에 프로젝션(z==0)해서 2차원 맵으로 구성, 패스를 찾는다.

  Room은 볼록 다각형으로 구성되어 있다.

  All copyright (c) 1998, MAIET entertainment software, inc

*/
#ifndef MPolygonMapModel_H
#define MPolygonMapModel_H

#include "CMList.h"
#include "CMPtrList.h"
#include "MAStar.h"
#include <list>
#include <map>

using namespace std;

/*
	세그먼트는 Right-Hand의 방향성을 가진다.

	   ->
	    /\
	   /  \
	  / in \
	 /      \ out
	+--------+
        <-
*/


/*
class MPMPoint;
typedef list<MPMPoint*>		MPNVList;
typedef MPNVList::iterator	MPNVListItor;
*/

class MPMPolygon;
class MPMPortal;
class MPolygonMapModel;

// 검색 가능한 Point Node
// 폴리곤의 각 모서리가 검색의 노드가 된다.
class MPMPoint : public MNodeModel{
protected:
friend class MPMPolygon;
friend class MPMPortal;
friend class MPolygonMapModel;
	MPolygonMapModel*		m_pParentMap;
	float				m_fX, m_fY, m_fZ;
	CMPtrList<MPMPoint>	m_VisibleList;		// 보이는 점 리스트
	int					m_nLastChangeCount;	// 최근까지 반영이 된 Map의 ChangeCount
	MPMPoint*			m_pPrevPoint;		// Polygon에 속했을때 이전 포인트
	MPMPoint*			m_pNextPoint;		// Polygon에 속했을때 이후 포인트
	CMPtrList<MPMPoint>	m_VisibledList;		// 이점이 보이지는 다른 점 리스트
	void*				m_pTemp;			// 임시 포인터 ( 보장받지 않는다. )

	//void*				m_pRoomIndex;
	list<int>			m_RoomIndexs;
	CMPtrList<MPMPortal>	m_InPortals;
	
protected:
	bool IsParentVisiblePoint(MPMPoint* p);	// Parent Point(Path설정중에..)에서 p가 보이는가 테스트
	void GenerateVisiblePoint(void);		// 보이는 점 찾기
	void ResetVisibleList(void);
	void AddVisiblePoint(MPMPoint* pPoint);

public:
	MPMPoint(float x, float y, float z, int nRoomIndex);
	MPMPoint(float x, float y, float z, list<int>& RoomIndexs);
	MPMPoint(void);

	inline float GetX(void){ return m_fX; }
	inline float GetY(void){ return m_fY; }
	inline float GetZ(void){ return m_fZ; }

	MPMPoint* GetPrevPoint(void){ return m_pPrevPoint; }
	MPMPoint* GetNextPoint(void){ return m_pNextPoint; }

	int GetVisiblePointCount(void);
	MPMPoint* GetVisiblePoint(int i);

	void ResetConnectPoint(void);
	bool IsVisible(MPMPoint* pPN);

	virtual int GetSuccessorCount(void);
	virtual MNodeModel* GetSuccessor(int i);

	virtual float GetSuccessorCost(MNodeModel* pSuccessor);
	virtual float GetHeuristicCost(MNodeModel* pNode);

	/// 이 노드가 속한 Room중에 가장 작은 인덱스
	int GetMinRoomIndex(void);
	/// 이 노드가 속한 Room중에 가장 큰 인덱스
	int GetMaxRoomIndex(void);

	//virtual float GetTotalCostPriority(void){ return m_fCostToGoal; }	// Goal까지의 Cost만을 고려한다.
};


enum MPMPOLYGONTYPE{
	MPMPT_NORMAL = 0,		// 특정 모양이 없는 폴리곤
	MPMPT_RECT,				// 직사각형 모양의 폴리곤, 보이는 폴리곤 추출시에 잇점을 제공해 준다.
};

// 폴리곤의 방향
enum MPMPOLYGONDIR{
	MPMPD_NA = 0,				// Not Available
	MPMPD_CLOCKWISE = 1,		// Clockwise
	MPMPD_COUNTERCLOCKWISE = 2,	// Counter-Clockwise
};

// 폴리곤 장애물
// Right-Handed이므로, 반대편으로는 이동 가능하다.
class MPMPolygon{
public:
	CMPtrList<MPMPoint>		m_PointList;		// 각 모서리
	bool					m_bEnclosed;		// 폴린곤이 닫혀 있는가?
	MPMPOLYGONTYPE			m_nType;			// 타입
	bool					m_bInnerStartPoint;	// 목표 지점이 이 폴리곤 안쪽에 속해 있는 경우.
	bool					m_bInnerEndPoint;	// 목표 지점이 이 폴리곤 안쪽에 속해 있는 경우.
	MPMPOLYGONDIR			m_nPolygonDir;		// 폴리곤의 방향 ( 폴리곤이 닫혀 있는 경우만, m_bEnclosed==true )
protected:
	// Merge Support
	CMPtrList<MPMPoint>		m_TempMergedPointAddList;		// 머지를 위한 임시 폴리곤 리스트
	CMPtrList<MPMPoint>		m_TempMergedPointDupList;		// 머지를 위한 임시 폴리곤 리스트, 겹치는 포인트
	int						m_nLatestFindIndex;
	MPMPoint*				m_pMergeTestBegin;
	MPMPoint*				m_pMergeTestEnd;
	//int						m_nMergeBeginIndex, m_nMergeEndIndex;	// 머지테스트할 시작과 끝 인덱스
protected:
	MPMPOLYGONDIR TestPolygonDir(void);			// 폴리곤 방향 테스트

	void Initialize(void);
public:
	MPMPolygon(void);
	// Rect Type의 Polygon 생성
	MPMPolygon(MPMPoint* lt, MPMPoint* rt, MPMPoint* rb, MPMPoint* lb);

	void Add(MPMPoint* pPointNode);
	void Delete(int i);
	void Enclose(bool bEnclosed);

	// 하나 이상의 연속된 에지를 공유하는 두 폴리곤을 머지하는 인터페이스, 머지하려는 폴리곤의 포인트를 하나씩 추가해 나가는 방식
	void Merge(MPMPoint* pPointNode);
	void MergeReset(void);	// Merge에 관련된 임시 데이터를 리셋한다.

	void InsertBefore(int nIndex, MPMPoint* pPointNode);
	void AddAfter(int nIndex, MPMPoint* pPointNode);
	void InsertBefore(int nIndex, CMPtrList<MPMPoint>* pList);
	void AddAfter(int nIndex, CMPtrList<MPMPoint>* pList);

	MPMPoint* Find(float x, float y, float z, MPMPoint* pTestBegin, MPMPoint* pTestEnd);
	int FindIndex(float x, float y, float z, MPMPoint* pTestBegin, MPMPoint* pTestEnd);
	int GetIndex(MPMPoint* pTest);

	// 어떤 점이 이 폴리곤 안에 갇혀 있는가?
	bool IsInnerPoint(float x, float y, bool bTestClockwise);
	MPMPOLYGONDIR IsInnerPoint(float* t, float x, float y);

	MPMPoint* GetStartPoint(void);
	MPMPoint* GetEndPoint(void);
	MPMPoint* Get(int i);

	void Add(MPMPolygon* pPO);

	void GetPointNormal(float* x, float* y, int i);
	void Enlarge(float fMargin);

	void RebuildConnection(void);

	bool IsIntersect(MPMPoint** p1, MPMPoint** p2, float* t, float x1, float y1, float x2, float y2);	// p1->p2?
	float GetNearestContactPoint(float* px, float* py, float x, float y, bool bInverse=false);			// 가장 가까운 폴리곤 라인위에 점
	//float GetContactPointByLineTest(float* px, float* py, float x, float y, int nTestCount);			// 라인을 nTestCount만큼 그어봐서 가장 가까운 점 찾기, 가장 가까운 점을 찾긴 어렵지만... 제한된 테스트 횟수를 보장한다.

	MPMPOLYGONDIR GetDir(void);		// Winding Dir
};

// Point 추가시 폴리곤을 끝을 명시.
enum MPMPOLYGONADDTYPE{
	MPMPAT_CONTINUE = 0,	// 계속 폴리곤이 이어진다.
	MPMPAT_END,			// 폴리곤을 끝낸다.
	MPMPAT_ENCLOSE,		// 폴리곤을 닫고 끝낸다.
};

// PreparePathFinding()의 Return Value
enum MPMRESULT{
	MPMR_OK,
	MPMR_ENDPOINTCHANGED,
};

struct MPOLYGONIDPAIR{
	int a, b;
	inline friend bool operator == (const MPOLYGONIDPAIR& a, const MPOLYGONIDPAIR& b){
		if(a.a==b.a && a.b==b.b) return true;
		return false;
	}
	inline friend bool operator > (const MPOLYGONIDPAIR& a, const MPOLYGONIDPAIR& b){
		if(a.a>b.a) return true;
		if(a.a<b.a) return false;
		if(a.b>b.b) return true;
		return false;
	}
	inline friend bool operator < (const MPOLYGONIDPAIR& a, const MPOLYGONIDPAIR& b){
		if(a.a<b.a) return true;
		if(a.a>b.a) return false;
		if(a.b<b.b) return true;
		return false;
	}
};

typedef map<MPOLYGONIDPAIR, bool>	MPOLYGONIDPAIRMAP;
/*
struct MPOLYGONIDPAIR{
	int a, b;
};
struct ltstr
{
	bool operator()(const MPOLYGONIDPAIR& s1, const MPOLYGONIDPAIR& s2) const
	{
		if(s1.a<s2.b) return true;
		return false;
		//return strcmp(s1, s2) < 0;
	}
};
typedef map<MPOLYGONIDPAIR, bool, ltstr>	MPOLYGONIDPAIRMAP;
*/

class MPMPortal{
//protected:
//friend class MPolygonMapModel;
public:
	MPMPoint*	m_pPos[2];	// Portal Position
	int			m_nPortalIndex;
public:
	MPMPortal(MPMPoint* pPos1, MPMPoint* pPos2, int nPortalIndex);
	virtual ~MPMPortal(void);
	void Enlarge(float fMargin);
};


// 정형화되지 않은 맵 모델
class MPolygonMapModel{
protected:
	MPMPolygon*					m_pCurrPolygon;			// Point가 추가되는 현재 폴리곤
	bool						m_bEndPointInPolygon;	// 끝 점이 어떤 폴리곤에 갇혀 있는가?
	int							m_nChangeCount;			// 맵이 변경된 카운트(변경 됐는지 알 수 있다.)
	int*						m_pPolygonPathList;
	int							m_nPolygonPathCount;
	MPOLYGONIDPAIRMAP			m_PolygonSharedMap;

public:
	MPMPoint*					m_pStartPoint;			// 시작 점
	MPMPoint*					m_pEndPoint;			// 끝 점
	CMLinkedList<MPMPoint>		m_PointList;
	CMLinkedList<MPMPolygon>	m_PolygonList;
	CMLinkedList<MPMPortal>		m_PortalList;

protected:
	void Change(void);									// 맵의 변화 지정
	void GetInnerClosedPolygon(CMPtrList<MPMPolygon>* pPolygons, int nPointType, MPMPOLYGONDIR dir);	// nPointType=0 Start, nPointType=1 End Point

	bool TestEndPointInPolygon(void);					// 끝점이 폴리곤 안에 갇혀 있는지 테스트, 미리 테스트함으로써 써치 타임에 성능 항상을 꽤할수 있다.
	bool TestStartPointInPolygon(void);					// 시작점이 폴리곤 안에 갇혀 있는지 테스트, 미리 테스트함으로써 써치 타임에 성능 항상을 꽤할수 있다.

	// 폴리곤 리스트중에 Non-Intersect하는 지점 추천(sx,sy 시작점, ex,ey 끝점)
	bool FindNonIntersectPosition(CMPtrList<MPMPolygon>* pPolygons, float* rx, float* ry, float sx, float sy, float ex, float ey);
	// 폴리곤 리스트중에 Non-Intersect하는 지점 추천(x,y에서 가까운 점)
	bool FindNonIntersectPosition(CMPtrList<MPMPolygon>* pPolygons, float* rx, float* ry, float x, float y);

	bool TestPointInPolygon(float* pNearestPointX, float* pNearestPointY, float x, float y);		// 갇혀있는지 테스트후, 가장 가까운 외부 지점 리턴
	bool RecommendPoint(float* pRecommendedPointX, float* pRecommendedPointY, float x, float y, float fCheckStepDistance, float fExponentialIncrease=1.0f);	// 충돌하지 않는 지점으로 추천
	bool RecommendEndPoint(float* pRecX, float* pRecY);	// 충돌하지 않는 지점으로 추천(return true면 재지정, 따라서.. TestEndPointInPolygon()을 해줘야 한다.

	friend class MPMPoint;
	void ClearVisibledList(void);				// MPMPoint::m_VisibledList Clear

	bool IsVisibleEdgePoint(MPMPoint* p1, MPMPoint* p2, int nBeginRoom, int nEndRoom);					// p1->p2?	( Edge만 검사 )
	int IsBiDirectionVisiblePoint(float* pT1, float* pT2, MPMPoint* p1, MPMPoint* p2);					// 0: 없음, 1: p1->p2, 2:p2->p1, 3:p1<->p2

public:
	MPolygonMapModel(void);
	virtual ~MPolygonMapModel(void);

	void Destroy(void);

	MPMPoint* AddPoint(float x, float y, float z, int nRoomIndex, bool bCheckDup=false);							// 일반점 추가 ( Point가 겹칠 수 있는 경우 중복 체크를 한다. )
	MPMPoint* AddPoint(float x, float y, float z, list<int>& RoomIndexs, bool bCheckDup=false);					// 일반점 추가 ( Point가 겹칠 수 있는 경우 중복 체크를 한다. )
	MPMPolygon* AddPolygon(MPMPoint* pNode, MPMPOLYGONADDTYPE nType=MPMPAT_CONTINUE);	// 현재 폴리곤에 점 추가
	MPMPolygon* AddRect(float x1, float y1, float z1, float x2, float y2, float z2, int pRoomIndexs[4], bool bInverse=false);	// 직 사각형 형태의 폴리곤 추가

	MPMPortal* AddPortal(MPMPoint* p1, MPMPoint* p2);

	MPMPoint* AddStartPoint(float x, float y, float z, int nRoomIndex);				// 시작점 추가
	MPMPoint* AddEndPoint(float x, float y, float z, int nRoomIndex);				// 끝 점 추가
	MPMPoint* GetStartPoint(void);
	MPMPoint* GetEndPoint(void);

	void SetStartPoint(float x, float y, float z, int nRoomIndex);					// 시작점
	void SetEndPoint(float x, float y, float z, int nRoomIndex);					// 끝 점

	// Low-Level Addition
	void AddPoint(MPMPoint* pPoint);												// 포인트 추가
	void AddPolygon(MPMPolygon* pPolygon);											// 폴리곤 추가
	void AddStartPoint(MPMPoint* pPoint);											// 시작점 추가
	void AddEndPoint(MPMPoint* pPoint);												// 끝 점 추가
	void AddPortal(MPMPortal* pPortal);												// 포탈 추가

	void Clear(void);																	// 추가된 Point, Polygon 초기화

	MPMRESULT PreparePathFinding(float* pRecX, float* pRecY);							// Path Finding Prepare ( End Point를 적절히 위치시킨다.)

	void SetPolygonPathList(int* pPolygonList, int nCount);										// 상위레벨의 폴리곤단위의 패스 리스트
	bool IsPolygonPathInclude(int nRoomIndexTest, int nRoomIndexBegin, int nRoomIndexEnd);	// Begin~End 사이에 포함되어 있는가?
	bool IsPolygonPathInclude(list<int>* pRoomIndexTest, list<int>* pRoomIndexBegin, list<int>* pRoomIndexEnd);

	void SetPolygonSharedMap(MPOLYGONIDPAIRMAP* pSharedMap);

	bool IsEndPointInPolygon(void);														// 끝 점이 폴리곤에 갇혀 있는가?
	void MergeBreakPolygon(void);														// 서로 다른 열려있는(MPMPAT_END) 폴리곤들중 시작점이나 끝점이 겹쳐서 연결이 가능한 폴리곤 합치기

	MPMPolygon* IsInnerClosedPolygon(float x, float y, bool bTestClockwise=true);		// 점이 폴리곤 안에 속해 있는가?
	void IsInnerClosedPolygon(CMPtrList<MPMPolygon>* pPolygons, float x, float y, bool bTestClockwise=true);	// 점이 폴리곤 안에 속해 있는가?

	bool IsIntersect(MPMPoint** p1, MPMPoint** p2, float* t, float x1, float y1, float x2, float y2);			// p1->p2?

	MPMPolygon* GetNearestInnerPolygon(CMLinkedList<MPMPolygon>* pPolygons, float x, float y);	// Inner Polygon중 가장 가까이에 있는 폴리곤( 방향에 상관 없음 ) (1, 0, 0) 방향이다.

	bool IsEnable(CMLinkedList<MPMPolygon>* pPolygons, float x, float y);	// Enable Position인가?
	bool IsEnable(float x, float y);									// Enable Position인가?

	void Add(MPolygonMapModel* pNGMM);

	void Enlarge(float fMargin);

	int GetChangeCount(void);
};

#endif
