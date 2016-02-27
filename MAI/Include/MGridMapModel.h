/*

  Grid형태의 최단 거리 검색 맵 모델
  ---------------------------------

  1999/10/8, lee jang ho
  All copyright (c) 1998, MAIET entertainment software, inc

*/
#ifndef MGRIDMAPMODEL_H
#define MGRIDMAPMODEL_H

#include "MAStar.h"
#include <math.h>

class MGridMapNode : public MNodeModel{
public:
	int		m_nX;
	int		m_nY;
public:
	MGridMapNode(void);
	MGridMapNode(int x, int y);
	virtual int GetSuccessorCount(void);
	virtual MNodeModel* GetSuccessor(int i);

	// 이웃 Node들의 Cost 얻기
	virtual float GetSuccessorCost(MNodeModel* pSuccessor);
	// Heuristic Estimation에 의한 pNode까지의 Cost 얻기
	virtual float GetHeuristicCost(MNodeModel* pNode);
};

/*
// 현재 방향을 고려한 휴리스틱을 가진 Grid Map Node
class MDirectionalGridMapNode : public MGridMapNode{
protected:
	static int		m_nStartDirX;
	static int		m_nStartDirY;
	static float	m_fRotation;		// 1만큼 움직이는데 가능한 Rotation Angle
protected:
	void GetDirection(int* px, int* py);
public:
	virtual float GetSuccessorCost(MNodeModel* pSuccessor);
};
*/
enum MGridMapNodeType{
	MGMNT_NORMAL = 0,
	//MGMNT_DIRECTIONSUPPORT,
};

class MGridMapModel{
private:
	MGridMapNode*	m_pNodes;
	int				m_nWidth;
	int				m_nHeight;
	int				m_nCheckNodeSize;
public:
	static MGridMapModel*	m_pInstance;
public:
	MGridMapModel(void);
	~MGridMapModel(void);

	bool Create(int x, int y, MGridMapNodeType nt=MGMNT_NORMAL);
	void Destroy(void);

	virtual bool IsCloseCell(int x, int y, int sx, int sy) = 0;

	//void OpenCell(int x, int y);
	//void CloseCell(int x, int y);

	int GetWidth(void);
	int GetHeight(void);

	int GetNodeSize(void);
	void SetNodeSize(int nSize);
	MGridMapNode* GetNode(int x, int y, int sx, int sy, bool bForced=false);
	MGridMapNode* GetNode(int x, int y, bool bForced=false);
	virtual float GetNeighborCost(int x1, int y1, int x2, int y2);
	virtual float GetHeuristicCost(int x1, int y1, int x2, int y2);
};

#endif
