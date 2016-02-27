#include "MGridMapModel.h"
//#include <math.h>
//#include "RSutils.h"

MGridMapNode::MGridMapNode(void)
{
}

MGridMapNode::MGridMapNode(int x, int y)
{
	m_nX = x;
	m_nY = y;
}

int MGridMapNode::GetSuccessorCount(void)
{
	return 8;
}

MNodeModel* MGridMapNode::GetSuccessor(int i)
{
	/*
	  3	| 2 | 1
	 ---+---+---
	  4	|   | 0
	 ---+---+---
	  5 | 6 | 7
	*/
	switch(i){
	case 0:
		return MGridMapModel::m_pInstance->GetNode(m_nX+1, m_nY, m_nX, m_nY);
	case 1:
		if(GetSuccessor(0)==NULL && GetSuccessor(2)==NULL) return NULL;
		return MGridMapModel::m_pInstance->GetNode(m_nX+1, m_nY-1, m_nX, m_nY);
	case 2:
		return MGridMapModel::m_pInstance->GetNode(m_nX, m_nY-1, m_nX, m_nY);
	case 3:
		if(GetSuccessor(2)==NULL && GetSuccessor(4)==NULL) return NULL;
		return MGridMapModel::m_pInstance->GetNode(m_nX-1, m_nY-1, m_nX, m_nY);
	case 4:
		return MGridMapModel::m_pInstance->GetNode(m_nX-1, m_nY, m_nX, m_nY);
	case 5:
		if(GetSuccessor(4)==NULL && GetSuccessor(6)==NULL) return NULL;
		return MGridMapModel::m_pInstance->GetNode(m_nX-1, m_nY+1, m_nX, m_nY);
	case 6:
		return MGridMapModel::m_pInstance->GetNode(m_nX, m_nY+1, m_nX, m_nY);
	case 7:
		if(GetSuccessor(6)==NULL && GetSuccessor(0)==NULL) return NULL;
		return MGridMapModel::m_pInstance->GetNode(m_nX+1, m_nY+1, m_nX, m_nY);
	}
	return NULL;
}

// 이웃 Node들의 Cost 얻기
float MGridMapNode::GetSuccessorCost(MNodeModel* pSuccessor)
{
	/*
	_ASSERT(pSuccessor!=NULL);
	double fCostPow = pow((MGrdiMapNode *)pSuccessor->m_nX-m_nX, 2) + pow((MGrdiMapNode *)pSuccessor->m_nY-m_nY, 2);
	if(fCostPow==0.0f) return 0.0f;
	return (float)sqrt(fCostPow);
	*/
	_ASSERT(pSuccessor!=NULL);
	int x2 = ((MGridMapNode *)pSuccessor)->m_nX;
	int y2 = ((MGridMapNode *)pSuccessor)->m_nY;
	return MGridMapModel::m_pInstance->GetNeighborCost(m_nX, m_nY, x2, y2);
}

// Heuristic Estimation에 의한 pNode까지의 Cost 얻기
float MGridMapNode::GetHeuristicCost(MNodeModel* pNode)
{
	if(pNode==NULL) return 0.0f;
	int x2 = ((MGridMapNode *)pNode)->m_nX;
	int y2 = ((MGridMapNode *)pNode)->m_nY;
	return MGridMapModel::m_pInstance->GetHeuristicCost(m_nX, m_nY, x2, y2);
}

/*
int MDirectionalGridMapNode::m_nStartDirX = 0;
int MDirectionalGridMapNode::m_nStartDirY = 1;
float MDirectionalGridMapNode::m_fRotation = 0.1f;

void MDirectionalGridMapNode::GetDirection(int* px, int* py)
{
	MDirectionalGridMapNode* pParent = (MDirectionalGridMapNode *)m_pParent;
	if(m_pParent!=NULL){
		*px = m_nX - pParent->m_nX;
		*py = m_nY - pParent->m_nY;
	}
	else{
		*px = m_nStartDirX;
		*py = m_nStartDirY;
	}
}

float MDirectionalGridMapNode::GetSuccessorCost(MNodeModel* pSuccessor)
{
	MDirectionalGridMapNode* pScr = (MDirectionalGridMapNode *)pSuccessor;
	int nToScsrDirX = pScr->m_nX - m_nX;
	int nToScsrDirY = pScr->m_nY - m_nY;
	int nCurrDirX, nCurrDirY;
	GetDirection(&nCurrDirX, &nCurrDirY);
	float fAngle = GetAngleOfVectors(rvector((float)nToScsrDirX, (float)nToScsrDirY, 0.0f), rvector((float)nCurrDirX, (float)nCurrDirY, 0.0f));
	float fAngleCost = (float)fabs(fAngle / m_fRotation);
	float fDistanceCost = GetHeuristicCost(pSuccessor);
	if(fAngleCost>fDistanceCost) return fAngleCost;
	else return fDistanceCost;
}
*/

MGridMapModel* MGridMapModel::m_pInstance = NULL;

MGridMapModel::MGridMapModel(void)
{
	m_pNodes = NULL;
	m_nCheckNodeSize = 1;

	_ASSERT(m_pInstance==NULL);	// 오직 하나의 Map Model Instance만이 존재해야 한다.
	m_pInstance = this;
}

MGridMapModel::~MGridMapModel(void)
{
	Destroy();
}

bool MGridMapModel::Create(int x, int y, MGridMapNodeType nt)
{
	if(nt==MGMNT_NORMAL) m_pNodes = new MGridMapNode[x*y];
	//else if(nt==MGMNT_DIRECTIONSUPPORT) m_pNodes = new MDirectionalGridMapNode[x*y];
	else{
		_ASSERT(false);	// Invalid Type
		return false;
	}

	m_nWidth = x;
	m_nHeight = y;
	if(m_pNodes==NULL) return false;
	return true;
}

void MGridMapModel::Destroy(void)
{
	if(m_pNodes!=NULL){
		delete[] m_pNodes;
		m_pNodes = NULL;
	}
}

/*
void MGridMapModel::OpenCell(int x, int y)
{
	SetCell(x, y, false);
}

void MGridMapModel::CloseCell(int x, int y)
{
	SetCell(x, y, true);
}
*/
int MGridMapModel::GetWidth(void)
{
	return m_nWidth;
}

int MGridMapModel::GetHeight(void)
{
	return m_nHeight;
}

int MGridMapModel::GetNodeSize(void)
{
	return m_nCheckNodeSize;
}

void MGridMapModel::SetNodeSize(int nSize)
{
	_ASSERT(nSize>0);
	m_nCheckNodeSize = nSize;
}

MGridMapNode* MGridMapModel::GetNode(int x, int y, int sx, int sy, bool bForced)
{
	if(x<0 || x>GetWidth()-m_nCheckNodeSize) return NULL;
	if(y<0 || y>GetHeight()-m_nCheckNodeSize) return NULL;

	if(bForced==false){
		for(int i=0; i<m_nCheckNodeSize; i++){
			for(int j=0; j<m_nCheckNodeSize; j++){
				if(IsCloseCell(x+i, y+j, sx+i, sy+i)==true) return NULL;
			}
		}
	}
	
	MGridMapNode* pNode = &(m_pNodes[x+y*GetWidth()]);
	pNode->m_nX = x;
	pNode->m_nY = y;
	return pNode;
}

MGridMapNode* MGridMapModel::GetNode(int x, int y, bool bForced)
{
	return GetNode(x, y, x, y, bForced);
}

float MGridMapModel::GetNeighborCost(int x1, int y1, int x2, int y2)
{
	return GetHeuristicCost(x1, y1, x2, y2);
}

float MGridMapModel::GetHeuristicCost(int x1, int y1, int x2, int y2)
{
	double fCostPow = pow(x1-x2, 2) + pow(y1-y2, 2);
	if(fCostPow==0.0f) return 0.0f;
	return (float)sqrt(fCostPow);
}
