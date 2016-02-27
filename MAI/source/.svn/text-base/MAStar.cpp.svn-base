#include "MAStar.h"

MNodeModel::MNodeModel(void)
{
	m_fCostFromStart = 0.0f;
	m_fCostToGoal = 0.0f;
	m_pParent = NULL;
	m_nVisitID = 0;
}

MNodeModel::~MNodeModel(void)
{
}

MAStar::MAStar(void)
{
	static int _InstanceVisitID = 0;
	m_nVisitID = _InstanceVisitID;
	_InstanceVisitID++;
}

void MAStar::PushOpen(MNodeModel* pNode)
{
	m_OpenStack.AddSorted(pNode);
}

MNodeModel* MAStar::PopOpen(void)
{
	int nStackCount = m_OpenStack.GetCount();
	if(nStackCount==0) return NULL;

	MNodeModel* pNode = m_OpenStack.Get(nStackCount-1);
	m_OpenStack.Delete(nStackCount-1);

	return pNode;
}

MNodeModel* MAStar::PopLowestCostOpen(void)
{
	MNodeModel* pLowestCostNode = NULL;
	if(m_OpenStack.GetCount()>0){
		pLowestCostNode = m_OpenStack.Get(0);	// Sorted List라서 맨 앞에 있는것이 최소값
		m_OpenStack.Delete(0);
	}
	return pLowestCostNode;
}

bool MAStar::IsOpenEmpty(void)
{
	if(m_OpenStack.GetCount()==0) return true;
	return false;
}

int MAStar::IsOpenNode(MNodeModel* pNode)
{
	for(int i=0; i<m_OpenStack.GetCount(); i++){
		MNodeModel* pCurNode = m_OpenStack.Get(i);
		if(pCurNode==pNode) return i;
	}
	return -1;
}
/*
void MAStar::PushClose(MNodeModel* pNode)
{
	m_CloseStack.Add(pNode);
}

MNodeModel* MAStar::PopClose(void)
{
	int nStackCount = m_CloseStack.GetCount();
	if(nStackCount==0) return NULL;

	MNodeModel* pNode = m_CloseStack.Get(nStackCount-1);
	m_CloseStack.Delete(nStackCount-1);

	return m_CloseStack.Get(nStackCount-1);
}

int MAStar::IsCloseNode(MNodeModel* pNode)
{
	for(int i=0; i<m_CloseStack.GetCount(); i++){
		MNodeModel* pCurNode = m_CloseStack.Get(i);
		if(pCurNode==pNode) return i;
	}
	return -1;
}

void MAStar::RemoveClose(int i)
{
	m_CloseStack.Delete(i);
}
*/

bool MAStar::IsForcedClose(MNodeModel* pNode, MNodeModel* pFromNode)
{
	for(int i=0; i<m_ForcedCloseNodes.GetCount(); i++){
		MFORCEDNODE* pForcedNode = m_ForcedCloseNodes.Get(i);
		if(pForcedNode->pFromNode==pFromNode && pForcedNode->pNode==pNode) return true;
	}
	return false;
}

void MAStar::AddForcedClose(MNodeModel* pNode, MNodeModel* pFromNode)
{
	MFORCEDNODE* pNew = new MFORCEDNODE;
	pNew->pNode = pNode;
	pNew->pFromNode = pFromNode;
	m_ForcedCloseNodes.Add(pNew);
}

void MAStar::RemoveForcedClose(void)
{
	m_ForcedCloseNodes.DeleteAll();
}

/*
Listing 3: A* search algorithm. 
priorityqueue	Open
list		Closed

AStarSearch
	s.g = 0		// s is the start node
	s.h = GoalDistEstimate( s )
	s.f = s.g + s.h
	s.parent = null
	push s on Open
	while Open is not empty
		pop node n from Open  // n has the lowest f
		if n is a goal node 
			construct path 
			return success
		for each successor n' of n
			newg = n.g + cost(n,n')
			if n' is in Open or Closed,
			 and n'.g <= newg
				skip
			n'.parent = n
			n'.g = newg
			n'.h = GoalDistEstimate( n' )
			n'.f = n'.g + n'.h
			if n' is in Closed
				remove it from Closed
			if n' is not yet in Open
				push n' on Open
		push n onto Closed
	return failure  // if no path found


--------------------------
f(n) = g(n) + h(n)

where:
     f(n)    is the score assigned to node n 
     g(n)    is the actual cheapest cost of 
             arriving at n from the start 
     h(n)    is the heuristic estimate of the 
             cost to the goal from n 
*/

bool MAStar::FindPath(MNodeModel* pStartNode, MNodeModel* pGoalNode, int nLimit, float fMinHeuristicValue, bool bIncludeStartNode)
{
	m_nNodeTraverseCount = 0;
	m_nVisitID++;
	m_OpenStack.DeleteAll();
	//m_CloseStack.DeleteAll();

	_ASSERT(pStartNode!=NULL);
	_ASSERT(pGoalNode!=NULL);
	if(pStartNode==NULL) return false;
	if(pGoalNode==NULL) return false;
	pStartNode->m_fCostFromStart = 0;
	pStartNode->m_fCostToGoal = pStartNode->GetHeuristicCost(pGoalNode);
	pStartNode->m_pParent = NULL;

	PushOpen(pStartNode);
	MNodeModel* pLowestCostNode = NULL;
	MNodeModel* pLowestCostToGoalNode = NULL;
	while(IsOpenEmpty()==false){
		MNodeModel* pNode = PopLowestCostOpen();
		if(pNode==pGoalNode || (nLimit>0 && m_nNodeTraverseCount>nLimit) || pNode->m_fCostToGoal<fMinHeuristicValue){
			// Construction Shortest Path;
			m_ShortestPath.DeleteAll();
			MNodeModel* pShortestNode = pNode;
			if(nLimit>0 && m_nNodeTraverseCount>nLimit) pShortestNode = pLowestCostToGoalNode;	// 리미트에 걸린 경우 현재까지 가장 짧은 노드를 리턴
			while(1){
				if(bIncludeStartNode==false && pShortestNode==pStartNode) break;	// StartNode 제외
				m_ShortestPath.Add(pShortestNode);
				if(bIncludeStartNode==true && pShortestNode==pStartNode) break;		// StartNode 포함
				pShortestNode = pShortestNode->m_pParent;
			};

			return true;
		}

		//if(pLowestCostNode==NULL|| pNode->m_fCostToGoal<pLowestCostNode->m_fCostToGoal)
		if(pLowestCostNode==NULL|| pNode->GetTotalCost()<pLowestCostNode->GetTotalCost())
			pLowestCostNode = pNode;
		if(pLowestCostToGoalNode==NULL|| pNode->m_fCostToGoal<pLowestCostToGoalNode->m_fCostToGoal)
			pLowestCostToGoalNode = pNode;

		for(int i=0; i<pNode->GetSuccessorCount(); i++){
			MNodeModel* pSuccessor = pNode->GetSuccessor(i);
			if(pSuccessor==NULL) continue;	// NULL이면 없는 노드로 간주한다.
			if(IsForcedClose(pSuccessor, pNode)==true) continue;	// 사용자에 의해 강제로 Close이면 갈 수 없는 노드로 간주한다.
			if(pSuccessor==pStartNode) continue;					// 시작 노드로 다시 돌아올 필요가 없으므로 제외한다.
			if(pNode->m_pParent==pSuccessor) continue;				// 왔던 길을 다시 돌아갈 필요가 없으므로 부모 노드는 제외

			//float fNewCostFromStart = pNode->m_fCostFromStart + pNode->GetSuccessorCost(pSuccessor);
			float fNewCostFromStart = pNode->GetSuccessorCostFromStart(pSuccessor);

			if(pSuccessor->m_nVisitID==m_nVisitID){
				if( pSuccessor->m_fCostFromStart <= fNewCostFromStart ) continue;	// Skip
			}
			/*
			int nOpenIndex = IsOpenNode(pSuccessor);
			int nCloseIndex = IsCloseNode(pSuccessor);
			if(nOpenIndex>=0 || nCloseIndex>=0){
				if( pSuccessor->m_fCostFromStart <= fNewCostFromStart ) continue;	// Skip
			}
			*/

			/*
			_ASSERT(pSuccessor!=pStartNode);	/// 시작노드로 돌아갈 이유가 없다.
			_ASSERT(pNode->m_fCostFromStart<=fNewCostFromStart);	// 현재 pNode를 거쳐가는 pSuccessor의 코스트는 현재 pNode의 코스트보다 작을 순 없다.
			if(!(pNode->m_fCostFromStart<=fNewCostFromStart)){
				if(pNode->m_pParent!=NULL){
					float fCostVerification = pNode->m_pParent->GetSuccessorCostFromStart(pNode);
					_ASSERT(fCostVerification==pNode->m_fCostFromStart);
					fCostVerification = pNode->GetSuccessorCostFromStart(pSuccessor);
					_ASSERT(fCostVerification==fNewCostFromStart);
				}
				_ASSERT(FALSE);
			}
			*/
			pSuccessor->m_pParent = pNode;
			pSuccessor->m_fCostFromStart = fNewCostFromStart;
			pSuccessor->m_fCostToGoal = pSuccessor->GetHeuristicCost(pGoalNode);

			if(pSuccessor->m_nVisitID!=m_nVisitID){
				pSuccessor->m_nVisitID = m_nVisitID;
				PushOpen(pSuccessor);
			}
			/*
			if(nCloseIndex>=0)
				RemoveClose(nCloseIndex);
			if(nOpenIndex==-1)
				PushOpen(pSuccessor);
			*/
			m_nNodeTraverseCount++;
		}

		//PushClose(pNode);
	}

	// 길을 찾지 못한 경우 가장 가까운 패스를 제시한다.
	// Construction Shortest Path;
	m_ShortestPath.DeleteAll();
	if(pLowestCostNode!=NULL){
		while(pLowestCostNode!=pStartNode){	// StartNode를 제외한 모든 GoalNode까지의 경로
			m_ShortestPath.Add(pLowestCostNode);
			pLowestCostNode = pLowestCostNode->m_pParent;
		};
	}

	return false;
}

int MAStar::GetShortestPathCount(void)
{
	return m_ShortestPath.GetCount();
}

MNodeModel* MAStar::GetShortestPath(int i)
{
	return m_ShortestPath.Get(m_ShortestPath.GetCount()-i-1);
}

void MAStar::RemoveShortestPath(int i)
{
	m_ShortestPath.Delete(m_ShortestPath.GetCount()-i-1);
}

CMPtrList<MNodeModel>* MAStar::GetShortestPath(void)
{
	return &m_ShortestPath;
}

void MAStar::Reset(void)
{
	m_ShortestPath.DeleteAll();
}
