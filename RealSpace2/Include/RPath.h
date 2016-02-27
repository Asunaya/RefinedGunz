#ifndef _RPATH_H
#define _RPATH_H

#include <list>
#include <vector>

using namespace std;

#include "RTypes.h"
#include "RNameSpace.h"

class MZFileSystem;

_NAMESPACE_REALSPACE2_BEGIN

class RPathNode;

class RPath {
public:
	int		nEdge;				// 인접한 변의 index
	int		nIndex;				// 목표 node
};

typedef vector<rvector*>	RVERTEXLIST;
typedef vector<RPath*>		RPATHLIST;

class RPathNode {
public:
	RPathNode(void);
	~RPathNode(void);

	rplane plane;
	RVERTEXLIST vertices;
	RPATHLIST	m_Neighborhoods;

	int	m_nIndex;				// 리스트 내의 위치

	int	m_nSourceID;			// 원본 폴리곤의 ID
	int	m_nGroupID;				// 노드의 Group ID		// 이동할 경로가 있는 노드들은 같은 ID 를 갖는다.

	void*	m_pUserData;

	void DrawWireFrame(DWORD color);
	void DeletePath(int nIndex);			// nIndex 노드로 가는 길을 제거한다.
};

class RPathList : public vector<RPathNode*> {
public:
	virtual ~RPathList();

	void DeleteAll();

	bool Save(const char *filename,int nSourcePolygon);
	bool Open(const char *filename,int nSourcePolygon,MZFileSystem *pfs=NULL);

	bool ConstructNeighborhood();
	bool ConstructGroupIDs();
	bool EliminateInvalid();

	int GetGroupCount() { return m_nGroup; }

protected:
	int	m_nGroup;

	void MarkGroupID(RPathNode *pNode,int nGroupID);			// GroupID 를 기록한다.
};

_NAMESPACE_REALSPACE2_END

#endif