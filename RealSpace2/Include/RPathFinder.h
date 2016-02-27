// PathFinder for RealSpace2
// MAI Required.

#ifndef RPATHFINDER_H
#define RPATHFINDER_H

#include <list>
using namespace std;

#include "RNameSpace.h"
#include "RTypes.h"
#include "CMPtrList.h"
#include "MPolygonMapModel.h"
#include "RPath.h"

_NAMESPACE_REALSPACE2_BEGIN

class RBspObject;
class RSBspNode;

class RVECTORLIST : public list<rvector*>{
public:
	virtual ~RVECTORLIST(void){
		Clear();
	}

	void Clear() {
		while(empty()==false){
			delete *begin();
			erase(begin());
		}
	}
};

// 폴리곤단위 노드
class MRPathNode : public MNodeModel{
	RPathList*	m_pPathList;
	RPathNode*	m_pPathNode;
public:
	int			m_nShortestPathID;		/// Temporal Used

	static rvector		m_StartPos;		/// 검색할 시작 지점
	static rvector		m_EndPos;		/// 검색할 도착 지점
	static MRPathNode*	m_pStartNode;	/// 검색할 시작 노드
	static MRPathNode*	m_pEndNode;		/// 검색할 도착 노드
public:
	MRPathNode(RPathList* pPathList, RPathNode* pPathNode);
	virtual ~MRPathNode(void);

	RPathNode* GetRPathNode(void){ return m_pPathNode; }
	int GetRPathNodeIndex(void){ return m_pPathNode->m_nIndex; }

	/// 이웃 Node 얻기
	virtual int GetSuccessorCount(void){
		return (int)m_pPathNode->m_Neighborhoods.size();
	}
	virtual MNodeModel* GetSuccessor(int i){
		_ASSERT(i>=0 && i<(int)m_pPathNode->m_Neighborhoods.size());
		return (MNodeModel*)((*m_pPathList)[m_pPathNode->m_Neighborhoods[i]->nIndex])->m_pUserData;
	}

	/// 두 노드 사이의 포탈 구하기
	static bool GetSuccessorPortal(rvector v[2], MRPathNode* pParent, MRPathNode* pSuccessor);

	/// 이웃 Node들의 Cost 얻기
	virtual float GetSuccessorCost(MNodeModel* pSuccessor);
	/// 시작점부터 이웃 Node들까지의 Cost 얻기
	virtual float GetSuccessorCostFromStart(MNodeModel* pSuccessor);
	/// Heuristic Estimation에 의한 pNode까지의 Cost 얻기
	virtual float GetHeuristicCost(MNodeModel* pNode);
};

/// Path 결과. 위치와 해당 노드
struct RPATHRESULT{
	rvector		Pos;
	MRPathNode*	pPathNode;

	RPATHRESULT(void){}
	RPATHRESULT(rvector& Pos, MRPathNode* pPathNode){
		RPATHRESULT::Pos = Pos;
		RPATHRESULT::pPathNode = pPathNode;
	}
};

class RPathResultList : public list<RPATHRESULT*>{
public:
	virtual ~RPathResultList(void){
		Clear();
	}

	void Clear() {
		while(empty()==false){
			delete *begin();
			erase(begin());
		}
	}
};

/// PathFinder for RealSpace2
class RPathFinder{
	RBspObject*	m_pBSPObject;	// 소스 BSP

	RPathNode*	m_pStartNode;	// 시작 Node
	RPathNode*	m_pEndNode;		// 목적 Node
	rvector		m_StartPos;		// 시작 지점
	rvector		m_EndPos;		// 목적 지점
	MAStar		m_PolygonPathFinder;		// 폴리곤 노드를 위한 패스파인더
	MPolygonMapModel	m_PolygonMapModel;	// VisibilityPath를 위한 맵 모델
	float		m_fEnlarge;
protected:
//	bool Pick(RSBspNode** ppNode, int* pIndex, rvector* pPickPos, RPathNode** ppPathNode, int x, int y);
	bool FindVisibilityPath(RVECTORLIST* pVisibilityPathList, rvector& StartPos, rvector& EndPos, CMPtrList<MNodeModel>* pPolygonShortestPath, MPolygonMapModel* pPolygonMapModel=NULL);
	bool FindVisibilityPath(RPathResultList* pVisibilityPathList, rvector& StartPos, rvector& EndPos, CMPtrList<MNodeModel>* pPolygonShortestPath, MPolygonMapModel* pPolygonMapModel=NULL);
public:
	RPathFinder(void);
	virtual ~RPathFinder(void);

	void Create(RBspObject* pBSPObject);
	void Destroy(void);

	/// 시작점 설정
	/// @param	sx		화면 좌표계 x
	/// @param	sy		화면 좌표계 y
	bool SetStartPos(int sx, int sy);
	bool SetStartPos(rvector &position);

	/// 시작점 직접 설정
	void SetStartPos(RPathNode *pStartNode,rvector StartPos);

	/// 끝점 설정
	/// @param	sx		화면 좌표계 x
	/// @param	sy		화면 좌표계 y
	bool SetEndPos(int sx, int sy);
	bool SetEndPos(rvector &position);

	/// 길찾기. SetStartPos(), SetEndPos()이 미리 설정되어야 한다.
	/// @param	pVisibilityPathList		검색된 경로. 시작점과 도착지점이 포함된다.
	/// @return							검색 성공 여부
	bool FindPath(RVECTORLIST* pVisibilityPathList);
	bool FindPath(RPathResultList* pVisibilityPathList);

	/// 길찾기. 일직선으로 갈 수 있는 경로까지를 찾아준다.
	/// SetStartPos(), SetEndPos()이 미리 설정되어야 한다.
	bool FindStraightPath(RVECTORLIST* pVisibilityPathList);

	/// 검색할 유닛의 크기에 따라 포탈의 크기를 조절해 준다.
	void Enlarge(float fMargin);

	RPathNode* GetStartNode(void){ return m_pStartNode; }
	RPathNode* GetEndNode(void){ return m_pEndNode; }
	MAStar* GetPolygonPathFinder(void){ return &m_PolygonPathFinder; }
	MPolygonMapModel* GetPolygonMapModel(void){ return &m_PolygonMapModel; }
};


// 검증 코드

// 해당 위치가 노드안에 존재하는가?
bool IsPosInNode(rvector& Pos, RPathNode* pNode);
// 해당 노드가 연결되어 있는가?
int IsNodeConnection(RPathNode* pNode1, RPathNode* pNode2, RPathList* pPathList);
// 패스가 포탈을 지나가는가?
bool IsPathAcrossPortal(rvector& Pos, rvector& NextPos, RPathNode* pNode, RPathNode* pNextNode, int nNodeNeighborhoodID, RPathList* pPathList);
// 시작점과 끝점의 연결선이 시작노드와 끝노드안에서 벽과 인터섹션이 발생하지 않는가?
bool IsPathInNode(rvector& Pos, rvector& NextPos, RPathNode* pNode, RPathNode* pNextNode, int nNodeNeighborhoodID1, int nNodeNeighborhoodID2, RPathList* pPathList);


_NAMESPACE_REALSPACE2_END

#endif