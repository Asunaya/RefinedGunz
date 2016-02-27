/*

  A Star Search
  -------------

  1999/10/8, lee jang ho
  All copyright (c) 1998, MAIET entertainment software, inc

*/
#ifndef MASTAR_H
#define MASTAR_H

#include "CMPtrList.h"
#include "CMList.h"

class MNodeModel{
public:
	float		m_fCostFromStart;
	float		m_fCostToGoal;
	MNodeModel*	m_pParent;
	int			m_nVisitID;
public:
	MNodeModel(void);
	virtual ~MNodeModel(void);
	// 이웃 Node 얻기
	virtual int GetSuccessorCount(void) = 0;
	virtual MNodeModel* GetSuccessor(int i) = 0;

	// 이웃 Node들의 Cost 얻기
	float GetSuccessorCost(int i){
		return GetSuccessorCost(GetSuccessor(i));
	}
	virtual float GetSuccessorCost(MNodeModel* pSuccessor) = 0;

	// 시작점부터 이웃 Node들까지의 Cost 얻기
	virtual float GetSuccessorCostFromStart(MNodeModel* pSuccessor){
		return m_fCostFromStart + GetSuccessorCost(pSuccessor);
	}

	// Heuristic Estimation에 의한 pNode까지의 Cost 얻기
	virtual float GetHeuristicCost(MNodeModel* pNode) = 0;

	float GetTotalCost(void){
		return m_fCostFromStart+m_fCostToGoal;
	}
	virtual float GetTotalCostPriority(void){	// 우선순위를 결정하기 위한 Total Cost
		return GetTotalCost();	// 대부분 GetTotalCost()와 같다.
	}
};

struct MFORCEDNODE{
	MNodeModel* pNode;
	MNodeModel* pFromNode;
};

class MAStar{
	CMPtrList<MNodeModel>	m_ShortestPath;
	class CMSortedList : public CMPtrList<MNodeModel>{
	public:
		int Compare(MNodeModel *lpRecord1, MNodeModel *lpRecord2){
			float fCompare = lpRecord1->GetTotalCostPriority()-lpRecord2->GetTotalCostPriority();
			if(fCompare>0.0f) return 1;
			else if(fCompare==0.0f) return 0;
			else return -1;
		}
	} m_OpenStack;

	CMLinkedList<MFORCEDNODE>	m_ForcedCloseNodes;
	int		m_nVisitID;	/// 노드 검색 ID
	
public:
	int		m_nNodeTraverseCount;		// 노드 탐색 횟수

	//CMPtrList<MNodeModel>	m_CloseStack;
protected:
	void PushOpen(MNodeModel* pNode);
	MNodeModel* PopOpen(void);
	MNodeModel* PopLowestCostOpen(void);
	bool IsOpenEmpty(void);
	int IsOpenNode(MNodeModel* pNode);
	/*
	void PushClose(MNodeModel* pNode);
	MNodeModel* PopClose(void);
	int IsCloseNode(MNodeModel* pNode);
	void RemoveClose(int i);
	*/
	bool IsForcedClose(MNodeModel* pNode, MNodeModel* pFromNode);

public:
	MAStar(void);

	void AddForcedClose(MNodeModel* pNode, MNodeModel* pFromNode);	// 강제로 Close 정의
	void RemoveForcedClose(void);

	bool FindPath(MNodeModel* pStartNode, MNodeModel* pGoalNode, int nLimit=-1, float fMinHeuristicValue=-1, bool bIncludeStartNode=false);

	int GetShortestPathCount(void);
	MNodeModel* GetShortestPath(int i);
	void RemoveShortestPath(int i);

	CMPtrList<MNodeModel>* GetShortestPath(void);

	void Reset(void);
};

#endif
