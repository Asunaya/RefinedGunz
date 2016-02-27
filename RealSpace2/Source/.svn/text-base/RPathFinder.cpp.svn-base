#include "MPolygonMapModel.h"	// MAI Required
#include "MUtil.h"	// MAI Required
#include "RPathFinder.h"
#include "RPath.h"
#include "RBspObject.h"

_NAMESPACE_REALSPACE2_BEGIN


rvector MRPathNode::m_StartPos;
rvector MRPathNode::m_EndPos;
MRPathNode*	MRPathNode::m_pStartNode = NULL;
MRPathNode*	MRPathNode::m_pEndNode = NULL;


MRPathNode::MRPathNode(RPathList* pPathList, RPathNode* pPathNode)
{
	m_pPathList = pPathList;
	m_pPathNode = pPathNode;
	m_pPathNode->m_pUserData = this;
}

MRPathNode::~MRPathNode(void)
{
	m_pPathNode->m_pUserData = NULL;
	m_pPathNode = NULL;
}

bool MRPathNode::GetSuccessorPortal(rvector v[2], MRPathNode* pParent, MRPathNode* pSuccessor)
{
	for(RPATHLIST::iterator ni = pParent->GetRPathNode()->m_Neighborhoods.begin(); ni!=pParent->GetRPathNode()->m_Neighborhoods.end(); ni++){
		RPath* pPath = *ni;
		if(pPath->nIndex==pSuccessor->GetRPathNodeIndex()){
			int nEdget1 = pPath->nEdge;
			int nEdget2 = nEdget1+1;
			if(nEdget2>=(int)pParent->GetRPathNode()->vertices.size()) nEdget2 = 0;
			v[0] = *(pParent->GetRPathNode()->vertices[nEdget1]);
			v[1] = *(pParent->GetRPathNode()->vertices[nEdget2]);
			return true;
		}
	}
	return false;
}

// 이웃 Node들의 Cost 얻기
float MRPathNode::GetSuccessorCost(MNodeModel* pSuccessor)
{
	// GetSuccessorCostFromStart() 펑션이 모든 Cost Function을 포함한다.
	return 0;
}

float MRPathNode::GetSuccessorCostFromStart(MNodeModel* pSuccessor)
{
	MPolygonMapModel PMM;
	MPolygonMapModel* pPMM = &PMM;

	int nPortalCount = 0;
	MNodeModel* pCur = this;
	MNodeModel* pCurSuccesor = pSuccessor;

	// 포탈(혹은 룸)을 노드로 이용하여 코스트를 계산할 방법이 명확하지 않으므로
	// 현재 노드까지 구성된 포탈들을 이용하여, 목적지점부터 시작지점까지의 최단거리를 Cost 값으로 이용한다.
	while(pCurSuccesor!=m_pStartNode){
		rvector v[2];
		bool bResult = GetSuccessorPortal(v, (MRPathNode*)pCur, (MRPathNode*)pCurSuccesor);
		_ASSERT(bResult==true);

		MPMPoint* p1 = pPMM->AddPoint(v[0].x, v[0].y, v[0].z, nPortalCount);
		MPMPoint* p2 = pPMM->AddPoint(v[1].x, v[1].y, v[1].z, nPortalCount);
		pPMM->AddPortal(p1, p2);

		nPortalCount++;

		//if(((pCur->m_pParent!=NULL)?(pCur->m_pParent->m_pParent!=pCur):TRUE)==FALSE) return FLT_MAX;
		_ASSERT((pCur->m_pParent!=NULL)?(pCur->m_pParent->m_pParent!=pCur):TRUE);		// 무한 반복
		pCurSuccesor = pCur;
		pCur = pCur->m_pParent;
		//if(pCur==m_pStartNode) break;
		_ASSERT(nPortalCount<10000);	// 무한 반복일 가능성이 높다.
	}
	// 포탈이 거꾸로 추가되었기 때문에, 검색도 거꾸로 한다.
	MPMPoint* pStartPoint = pPMM->AddStartPoint(m_EndPos.x, m_EndPos.y, m_EndPos.z, 0);
	MPMPoint* pEndPoint = pPMM->AddEndPoint(m_StartPos.x, m_StartPos.y, m_StartPos.z, nPortalCount);

	MAStar VisibilityPathFinder;
	VisibilityPathFinder.FindPath(pStartPoint, pEndPoint, -1, -1, true);


	// 총 거리 계산
	MPMPoint* pPrev = NULL;
	float fDistance = 0;
	for(int h=0; h<VisibilityPathFinder.GetShortestPathCount(); h++){
		MPMPoint* p = (MPMPoint*)VisibilityPathFinder.GetShortestPath(h);
		if(pPrev!=NULL){
			rvector Diff(rvector(pPrev->GetX(), pPrev->GetY(), pPrev->GetZ()) - rvector(p->GetX(), p->GetY(), p->GetZ()));
			fDistance += D3DXVec3Length(&Diff);	// D3DXVec3LengthSq()는 컴포넌트의 합에 대해 비례하지 않으므로 사용하지 않는다.
		}
		pPrev = p;
	}

	return fDistance;
}


// Heuristic Estimation에 의한 pNode까지의 Cost 얻기
float MRPathNode::GetHeuristicCost(MNodeModel* pNode)
{
	// GetSuccessorCostFromStart() 펑션이 모든 Cost Function을 포함한다.
	return 0;
}



RPathFinder::RPathFinder(void)
{
	m_pBSPObject = NULL;
	m_pStartNode = NULL;
	m_pEndNode = NULL;
	m_fEnlarge = 0;
}

RPathFinder::~RPathFinder(void)
{
	Destroy();
}

void RPathFinder::Create(RBspObject* pBSPObject)
{
	Destroy();

	m_pBSPObject = pBSPObject;
	RPathList* pPathList = m_pBSPObject->GetPathList();

	int nCount = 0;
	for(RPathList::iterator i=pPathList->begin(); i!=pPathList->end(); i++){
		RPathNode* pPathNode = *i;
		pPathNode->m_pUserData = new MRPathNode(pPathList, pPathNode);
		nCount++;
	}
}

void RPathFinder::Destroy(void)
{
	if(m_pBSPObject!=NULL){
		RPathList* pPathList = m_pBSPObject->GetPathList();
		for(RPathList::iterator i=pPathList->begin(); i!=pPathList->end(); i++){
			RPathNode* pPathNode = *i;
			MRPathNode* pMRPathNode = (MRPathNode*)pPathNode->m_pUserData;
			delete pMRPathNode;
		}
		m_pBSPObject = NULL;
	}
}

/*
bool RPathFinder::Pick(RSBspNode **ppNode, int* pIndex, rvector* pPickPos, RPathNode** ppPathNode, int x, int y)
{
	int nIndex;
	if(m_pBSPObject->Pick(x, y, ppNode, &nIndex, pPickPos)){
		*ppPathNode = m_pBSPObject->GetPathNode(*ppNode,nIndex);
			//GetPathList()->FindNode(*(rvector*)&(*ppNode)->pVertices[nIndex*3].x,*(rvector*)&(*ppNode)->pVertices[nIndex*3+1].x,*(rvector*)&(*ppNode)->pVertices[nIndex*3+2].x,(*ppNode)->pInfo[nIndex].plane);
		if(*ppPathNode){
			// 픽킹이 제대로 안되므로, 임시로 해놓는다.
			/*
			rvector Pos = rvector(0,0,0);
			for(RVERTEXLIST::iterator j=(*ppPathNode)->vertices.begin(); j!=(*ppPathNode)->vertices.end(); j++){
				Pos.x += (*j)->x;
				Pos.y += (*j)->y;
				Pos.z += (*j)->z;
			}
			Pos.x /= (*ppPathNode)->vertices.size();
			Pos.y /= (*ppPathNode)->vertices.size();
			Pos.z /= (*ppPathNode)->vertices.size();

			*pPickPos = Pos;
			*//*//
			*pIndex = nIndex;
			return true;
		}
	}

	return false;
}
*/

bool RPathFinder::FindVisibilityPath(RVECTORLIST* pVisibilityPathList, rvector& StartPos, rvector& EndPos, CMPtrList<MNodeModel>* pPolygonShortestPath, MPolygonMapModel* pPolygonMapModel)
{
	pVisibilityPathList->clear();

	//if(pPolygonShortestPath->GetCount()<=1){	// 같은 폴리곤 노드안에 있는 경우 곧바로 연결, 나중에 장애물이 들어가면 바뀌어야 할 코드
	if(pPolygonShortestPath->GetCount()==0){
		pVisibilityPathList->insert(pVisibilityPathList->end(), new rvector(StartPos));
		pVisibilityPathList->insert(pVisibilityPathList->end(), new rvector(EndPos));
		return true;
	}

	MPolygonMapModel* pPMM = NULL;
	if(pPolygonMapModel!=NULL) pPMM = pPolygonMapModel;
	else pPMM = new MPolygonMapModel;

	pPMM->Clear();

	MRPathNode* pStartNode = (MRPathNode*)pPolygonShortestPath->Get(0);
	MRPathNode* pEndNode = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1);

	/*
	MPMPolygon* pMergedPolygon = new MPMPolygon;

	// Short Path List
	int* pShortestPathList = new int[pPolygonShortestPath->GetCount()];
	for(int sp=0; sp<pPolygonShortestPath->GetCount(); sp++){
		MRPathNode* pMRNode = (MRPathNode*)pPolygonShortestPath->Get(sp);
		pMRNode->m_nShortestPathID = sp;
		pShortestPathList[sp] = sp;
	}
	pPMM->SetPolygonPathList(pShortestPathList, pPolygonShortestPath->GetCount());
	delete[] pShortestPathList;


	MPOLYGONIDPAIRMAP PolygonSharedMap;
	// 복층 테스트
	for(int a=0; a<pPolygonShortestPath->GetCount(); a++){
		MRPathNode* pMRNode1 = (MRPathNode*)pPolygonShortestPath->Get(a);
		for(int b=a+2; b<pPolygonShortestPath->GetCount(); b++){	// 인접한 폴리곤은 건너 띄고 테스트(인접한 폴리곤이 복층이 될 수 없다.)
			MRPathNode* pMRNode2 = (MRPathNode*)pPolygonShortestPath->Get(b);
			//if(pMRNode1!=pMRNode2 && IsSharedArea(pMRNode1, pMRNode2)==true){
			if(IsSharedArea(pMRNode1, pMRNode2)==true){
				MPOLYGONIDPAIR pair;
				_ASSERT(pMRNode1->m_nShortestPathID==a && pMRNode2->m_nShortestPathID==b);
				pair.a = pMRNode1->m_nShortestPathID;
				pair.b = pMRNode2->m_nShortestPathID;
				PolygonSharedMap.insert(MPOLYGONIDPAIRMAP::value_type(pair, true));
			}
		}
	}

	pPMM->SetPolygonSharedMap(&PolygonSharedMap);

	// Start Polygon Node
	MRPathNode* pMRNode = (MRPathNode*)pPolygonShortestPath->Get(0);
	for(RVERTEXLIST::iterator i=pMRNode->GetRPathNode()->vertices.begin(); i!=pMRNode->GetRPathNode()->vertices.end(); i++){
		MPMPoint* pNewPoint = new MPMPoint((*i)->x, (*i)->y, (*i)->z, pMRNode->m_nShortestPathID);
		pMergedPolygon->Add(pNewPoint);
		pPMM->AddPoint(pNewPoint);
	}
	pMergedPolygon->Enclose(true);

	// Merge Remained Polygon Node
	for(int k=1; k<pPolygonShortestPath->GetCount(); k++){
		MRPathNode* pMRNode = (MRPathNode*)pPolygonShortestPath->Get(k);
		for(RVERTEXLIST::iterator i=pMRNode->GetRPathNode()->vertices.begin(); i!=pMRNode->GetRPathNode()->vertices.end(); i++){
			MPMPoint* pNewPoint = new MPMPoint((*i)->x, (*i)->y, (*i)->z, pMRNode->m_nShortestPathID);
			pMergedPolygon->Merge(pNewPoint);
			pPMM->AddPoint(pNewPoint);
		}
		pMergedPolygon->MergeReset();
	}

	pPMM->AddPolygon(pMergedPolygon);
	MPMPoint* pStartPoint = pPMM->AddStartPoint(StartPos.x, StartPos.y, StartPos.z, pStartNode->m_nShortestPathID);
	MPMPoint* pEndPoint = pPMM->AddEndPoint(EndPos.x, EndPos.y, EndPos.z, pEndNode->m_nShortestPathID);
	float fRecX, fRecY;
	pPMM->PreparePathFinding(&fRecX, &fRecY);
	*/

	// Portal 추가
	for(int k=0; k<pPolygonShortestPath->GetCount()-1; k++){
		MRPathNode* pMRNode1 = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1-k);
		MRPathNode* pMRNode2 = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1-(k+1));
		//_ASSERT(pMRNode->GetRPathNode()->m_Neighborhoods.size()==1);	// 공유되는 엣지(포탈)는 항상 한개여야 한다. (볼록다각형의 공간)
		rvector v[2];
		bool bResult = MRPathNode::GetSuccessorPortal(v, pMRNode1, pMRNode2);
		_ASSERT(bResult==true);

		MPMPoint* p1 = pPMM->AddPoint(v[0].x, v[0].y, v[0].z, k);
		MPMPoint* p2 = pPMM->AddPoint(v[1].x, v[1].y, v[1].z, k);
		pPMM->AddPortal(p1, p2);

		/*
		for(RPATHLIST::iterator ni = pMRNode1->GetRPathNode()->m_Neighborhoods.begin(); ni!=pMRNode1->GetRPathNode()->m_Neighborhoods.end(); ni++){
			RPath* pPath = *ni;
			if(pPath->nIndex==pMRNode2->GetRPathNodeIndex()){
				int nCount = 0;
				for(RVERTEXLIST::iterator i=pMRNode1->GetRPathNode()->vertices.begin(); i!=pMRNode1->GetRPathNode()->vertices.end(); i++){
					if(nCount==pPath->nEdge){
						rvector* v1 = *i++;
						rvector* v2 = NULL;
						if(i!=pMRNode1->GetRPathNode()->vertices.end()) v2 = *i;
						else v2 = *pMRNode1->GetRPathNode()->vertices.begin();

						MPMPoint* p1 = pPMM->AddPoint(v1->x, v1->y, v1->z, k);
						MPMPoint* p2 = pPMM->AddPoint(v2->x, v2->y, v2->z, k);
						pPMM->AddPortal(p1, p2);
						break;
					}
					nCount++;
				}
				break;
			}
		}
		*/

	}

	MPMPoint* pStartPoint = pPMM->AddStartPoint(StartPos.x, StartPos.y, StartPos.z, 0);
	MPMPoint* pEndPoint = pPMM->AddEndPoint(EndPos.x, EndPos.y, EndPos.z, pPolygonShortestPath->GetCount()-1);

	pPMM->Enlarge(m_fEnlarge);


	MAStar VisibilityPathFinder;
	VisibilityPathFinder.FindPath(pStartPoint, pEndPoint, -1, -1, true);

	MPMPoint* pPrev = NULL;
	for(int h=0; h<VisibilityPathFinder.GetShortestPathCount(); h++){
		MPMPoint* p = (MPMPoint*)VisibilityPathFinder.GetShortestPath(h);
		if(pPrev!=NULL){
			// 패스 중간에 교차된 포탈이 있으면 포탈에 패스 노드 추가 ( 바닥을 타고 다니기 위해. 단, 각 Room폴린곤은 평면이여야 한다. )
			int nBeginIndex = pPrev->GetMaxRoomIndex();
			int nEndIndex = p->GetMinRoomIndex();
			_ASSERT(nBeginIndex<=nEndIndex);	// 시작 노드보다 끝 노드의 인덱스가 항상 크거나 같다.
			_ASSERT(nEndIndex<=pPolygonShortestPath->GetCount());
			if(nBeginIndex<nEndIndex){
				for(int i=nBeginIndex; i<nEndIndex; i++){
					MRPathNode* pMRNode1 = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1-i);
					MRPathNode* pMRNode2 = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1-(i+1));

					rvector Portal[2];
					bool bPortal = MRPathNode::GetSuccessorPortal(Portal, pMRNode1, pMRNode2);
					_ASSERT(bPortal==true);	// 항상 포탈이 존재해야 한다.
					float t1, t2;
					bool bIntersect = MFGetIntersectOfSegmentAndSegment(&t1, &t2, pPrev->GetX(), pPrev->GetY(), p->GetX(), p->GetY(),
						Portal[0].x, Portal[0].y, Portal[1].x, Portal[1].y);
					//_ASSERT(bIntersect==true);	// 항상 겹쳐져야 한다.
					if(bIntersect==true){
						// 교차점에 점 추가
						rvector* pNew = new rvector;
						pNew->x = pPrev->GetX() +  (p->GetX()-pPrev->GetX()) * t1;
						pNew->y = pPrev->GetY() +  (p->GetY()-pPrev->GetY()) * t1;
						pNew->z = Portal[0].z +  (Portal[1].z-Portal[0].z) * t2;
						pVisibilityPathList->insert(pVisibilityPathList->end(), pNew);
					}
				}
			}
		}
		pVisibilityPathList->insert(pVisibilityPathList->end(), new rvector(p->GetX(), p->GetY(), p->GetZ()));
		pPrev = p;
	}

	if(pPolygonMapModel==NULL) delete pPMM;

	return true;
}

bool RPathFinder::FindVisibilityPath(RPathResultList* pVisibilityPathList, rvector& StartPos, rvector& EndPos, CMPtrList<MNodeModel>* pPolygonShortestPath, MPolygonMapModel* pPolygonMapModel)
{
	pVisibilityPathList->clear();

	MRPathNode* pStartNode = (MRPathNode*)pPolygonShortestPath->Get(0);
	MRPathNode* pEndNode = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1);

	if(pPolygonShortestPath->GetCount()==0){
		pVisibilityPathList->insert(pVisibilityPathList->end(), new RPATHRESULT(StartPos, pStartNode));
		pVisibilityPathList->insert(pVisibilityPathList->end(), new RPATHRESULT(EndPos, pEndNode));
		return true;
	}

	MPolygonMapModel* pPMM = NULL;
	if(pPolygonMapModel!=NULL) pPMM = pPolygonMapModel;
	else pPMM = new MPolygonMapModel;

	pPMM->Clear();

	/*
	MPMPolygon* pMergedPolygon = new MPMPolygon;

	// Short Path List
	int* pShortestPathList = new int[pPolygonShortestPath->GetCount()];
	for(int sp=0; sp<pPolygonShortestPath->GetCount(); sp++){
	MRPathNode* pMRNode = (MRPathNode*)pPolygonShortestPath->Get(sp);
	pMRNode->m_nShortestPathID = sp;
	pShortestPathList[sp] = sp;
	}
	pPMM->SetPolygonPathList(pShortestPathList, pPolygonShortestPath->GetCount());
	delete[] pShortestPathList;


	MPOLYGONIDPAIRMAP PolygonSharedMap;
	// 복층 테스트
	for(int a=0; a<pPolygonShortestPath->GetCount(); a++){
	MRPathNode* pMRNode1 = (MRPathNode*)pPolygonShortestPath->Get(a);
	for(int b=a+2; b<pPolygonShortestPath->GetCount(); b++){	// 인접한 폴리곤은 건너 띄고 테스트(인접한 폴리곤이 복층이 될 수 없다.)
	MRPathNode* pMRNode2 = (MRPathNode*)pPolygonShortestPath->Get(b);
	//if(pMRNode1!=pMRNode2 && IsSharedArea(pMRNode1, pMRNode2)==true){
	if(IsSharedArea(pMRNode1, pMRNode2)==true){
	MPOLYGONIDPAIR pair;
	_ASSERT(pMRNode1->m_nShortestPathID==a && pMRNode2->m_nShortestPathID==b);
	pair.a = pMRNode1->m_nShortestPathID;
	pair.b = pMRNode2->m_nShortestPathID;
	PolygonSharedMap.insert(MPOLYGONIDPAIRMAP::value_type(pair, true));
	}
	}
	}

	pPMM->SetPolygonSharedMap(&PolygonSharedMap);

	// Start Polygon Node
	MRPathNode* pMRNode = (MRPathNode*)pPolygonShortestPath->Get(0);
	for(RVERTEXLIST::iterator i=pMRNode->GetRPathNode()->vertices.begin(); i!=pMRNode->GetRPathNode()->vertices.end(); i++){
	MPMPoint* pNewPoint = new MPMPoint((*i)->x, (*i)->y, (*i)->z, pMRNode->m_nShortestPathID);
	pMergedPolygon->Add(pNewPoint);
	pPMM->AddPoint(pNewPoint);
	}
	pMergedPolygon->Enclose(true);

	// Merge Remained Polygon Node
	for(int k=1; k<pPolygonShortestPath->GetCount(); k++){
	MRPathNode* pMRNode = (MRPathNode*)pPolygonShortestPath->Get(k);
	for(RVERTEXLIST::iterator i=pMRNode->GetRPathNode()->vertices.begin(); i!=pMRNode->GetRPathNode()->vertices.end(); i++){
	MPMPoint* pNewPoint = new MPMPoint((*i)->x, (*i)->y, (*i)->z, pMRNode->m_nShortestPathID);
	pMergedPolygon->Merge(pNewPoint);
	pPMM->AddPoint(pNewPoint);
	}
	pMergedPolygon->MergeReset();
	}

	pPMM->AddPolygon(pMergedPolygon);
	MPMPoint* pStartPoint = pPMM->AddStartPoint(StartPos.x, StartPos.y, StartPos.z, pStartNode->m_nShortestPathID);
	MPMPoint* pEndPoint = pPMM->AddEndPoint(EndPos.x, EndPos.y, EndPos.z, pEndNode->m_nShortestPathID);
	float fRecX, fRecY;
	pPMM->PreparePathFinding(&fRecX, &fRecY);
	*/

	// Portal 추가
	for(int k=0; k<pPolygonShortestPath->GetCount()-1; k++){
		MRPathNode* pMRNode1 = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1-k);
		MRPathNode* pMRNode2 = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1-(k+1));
		//_ASSERT(pMRNode->GetRPathNode()->m_Neighborhoods.size()==1);	// 공유되는 엣지(포탈)는 항상 한개여야 한다. (볼록다각형의 공간)
		rvector v[2];
		bool bResult = MRPathNode::GetSuccessorPortal(v, pMRNode1, pMRNode2);
		_ASSERT(bResult==true);

		MPMPoint* p1 = pPMM->AddPoint(v[0].x, v[0].y, v[0].z, k);
		MPMPoint* p2 = pPMM->AddPoint(v[1].x, v[1].y, v[1].z, k);
		pPMM->AddPortal(p1, p2);

		/*
		for(RPATHLIST::iterator ni = pMRNode1->GetRPathNode()->m_Neighborhoods.begin(); ni!=pMRNode1->GetRPathNode()->m_Neighborhoods.end(); ni++){
		RPath* pPath = *ni;
		if(pPath->nIndex==pMRNode2->GetRPathNodeIndex()){
		int nCount = 0;
		for(RVERTEXLIST::iterator i=pMRNode1->GetRPathNode()->vertices.begin(); i!=pMRNode1->GetRPathNode()->vertices.end(); i++){
		if(nCount==pPath->nEdge){
		rvector* v1 = *i++;
		rvector* v2 = NULL;
		if(i!=pMRNode1->GetRPathNode()->vertices.end()) v2 = *i;
		else v2 = *pMRNode1->GetRPathNode()->vertices.begin();

		MPMPoint* p1 = pPMM->AddPoint(v1->x, v1->y, v1->z, k);
		MPMPoint* p2 = pPMM->AddPoint(v2->x, v2->y, v2->z, k);
		pPMM->AddPortal(p1, p2);
		break;
		}
		nCount++;
		}
		break;
		}
		}
		*/

	}

	MPMPoint* pStartPoint = pPMM->AddStartPoint(StartPos.x, StartPos.y, StartPos.z, 0);
	MPMPoint* pEndPoint = pPMM->AddEndPoint(EndPos.x, EndPos.y, EndPos.z, pPolygonShortestPath->GetCount()-1);

	pPMM->Enlarge(m_fEnlarge);


	MAStar VisibilityPathFinder;
	VisibilityPathFinder.FindPath(pStartPoint, pEndPoint, -1, -1, true);

	MPMPoint* pPrev = NULL;
	for(int h=0; h<VisibilityPathFinder.GetShortestPathCount(); h++){
		MPMPoint* p = (MPMPoint*)VisibilityPathFinder.GetShortestPath(h);
		if(pPrev!=NULL){
			// 패스 중간에 교차된 포탈이 있으면 포탈에 패스 노드 추가 ( 바닥을 타고 다니기 위해. 단, 각 Room폴린곤은 평면이여야 한다. )
			int nBeginIndex = pPrev->GetMaxRoomIndex();
			int nEndIndex = p->GetMinRoomIndex();
			_ASSERT(nBeginIndex<=nEndIndex);	// 시작 노드보다 끝 노드의 인덱스가 항상 크거나 같다.
			_ASSERT(nEndIndex<=pPolygonShortestPath->GetCount());
			if(nBeginIndex<nEndIndex){
				for(int i=nBeginIndex; i<nEndIndex; i++){
					MRPathNode* pMRNode1 = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1-i);
					MRPathNode* pMRNode2 = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1-(i+1));

					rvector Portal[2];
					bool bPortal = MRPathNode::GetSuccessorPortal(Portal, pMRNode1, pMRNode2);
					_ASSERT(bPortal==true);	// 항상 포탈이 존재해야 한다.
					float t1, t2;
					bool bIntersect = MFGetIntersectOfSegmentAndSegment(&t1, &t2, pPrev->GetX(), pPrev->GetY(), p->GetX(), p->GetY(),
						Portal[0].x, Portal[0].y, Portal[1].x, Portal[1].y);
					//_ASSERT(bIntersect==true);	// 항상 겹쳐져야 한다.
					if(bIntersect==true){
						// 교차점에 점 추가
						/*
						rvector* pNew = new rvector;
						pNew->x = pPrev->GetX() +  (p->GetX()-pPrev->GetX()) * t1;
						pNew->y = pPrev->GetY() +  (p->GetY()-pPrev->GetY()) * t1;
						pNew->z = Portal[0].z +  (Portal[1].z-Portal[0].z) * t2;
						*/
						RPATHRESULT* pNew = new RPATHRESULT;
						pNew->Pos = rvector(pPrev->GetX() +  (p->GetX()-pPrev->GetX()) * t1, pPrev->GetY() +  (p->GetY()-pPrev->GetY()) * t1, Portal[0].z +  (Portal[1].z-Portal[0].z) * t2);
						pNew->pPathNode = pMRNode2;
						pVisibilityPathList->insert(pVisibilityPathList->end(), pNew);
					}
				}
			}
		}
		MRPathNode* pMRNode = (MRPathNode*)pPolygonShortestPath->Get(pPolygonShortestPath->GetCount()-1-p->GetMinRoomIndex());
		pVisibilityPathList->insert(pVisibilityPathList->end(), new RPATHRESULT(rvector(p->GetX(), p->GetY(), p->GetZ()), pMRNode));
		pPrev = p;
	}

	if(pPolygonMapModel==NULL) delete pPMM;

	return true;
}


bool RPathFinder::SetStartPos(int sx, int sy)
{
	if(m_pBSPObject->PickPathNode(sx,sy,&m_pStartNode,&m_StartPos)==false) return false;

	return true;
}

bool RPathFinder::SetStartPos(rvector &position)
{
	if(m_pBSPObject->PickPathNode(position+rvector(0,0,10),position+rvector(0,0,-1),&m_pStartNode,&m_StartPos)==false) return false;

	return true;
}

void RPathFinder::SetStartPos(RPathNode *pStartNode,rvector StartPos)
{
	m_pStartNode=pStartNode;
	m_StartPos=StartPos;
}

bool RPathFinder::SetEndPos(int sx, int sy)
{
	if(m_pBSPObject->PickPathNode(sx,sy,&m_pEndNode,&m_EndPos)==false) return false;

	return true;
}

bool RPathFinder::SetEndPos(rvector &position)
{
	if(m_pBSPObject->PickPathNode(position+rvector(0,0,10),position+rvector(0,0,-1),&m_pEndNode,&m_EndPos)==false) return false;

	return true;
}

bool RPathFinder::FindPath(RVECTORLIST* pVisibilityPathList)
{
	if(m_pStartNode==NULL || m_pEndNode==NULL) return false;
	if(m_pStartNode->m_nGroupID != m_pEndNode->m_nGroupID ) return false;	// 이미 갈수없는 곳이다.

	MRPathNode* pStartNode = (MRPathNode*)m_pStartNode->m_pUserData;
	MRPathNode* pEndNode = (MRPathNode*)m_pEndNode->m_pUserData;
	_ASSERT(pStartNode!=NULL);
	_ASSERT(pEndNode!=NULL);

	/*
	MRPathNode::m_StartPos = rvector(m_StartPos.x, m_StartPos.y, 0);
	MRPathNode::m_EndPos = rvector(m_EndPos.x, m_EndPos.y, 0);
	*/
	MRPathNode::m_StartPos = m_StartPos;
	MRPathNode::m_EndPos = m_EndPos;
	MRPathNode::m_pStartNode = pStartNode;
	MRPathNode::m_pEndNode = pEndNode;

//#define PATHFIND_DEPTH_LIMIT	200
#define PATHFIND_DEPTH_LIMIT	-1

	bool bFindPath = m_PolygonPathFinder.FindPath(pStartNode, pEndNode, PATHFIND_DEPTH_LIMIT, -1, true);
	if(bFindPath==false) return false;
	bFindPath = FindVisibilityPath(pVisibilityPathList, m_StartPos, m_EndPos, m_PolygonPathFinder.GetShortestPath(), &m_PolygonMapModel);

	return bFindPath;
}

bool RPathFinder::FindPath(RPathResultList* pVisibilityPathList)
{
	if(m_pStartNode==NULL || m_pEndNode==NULL) return false;
	if(m_pStartNode->m_nGroupID != m_pEndNode->m_nGroupID ) return false;	// 이미 갈수없는 곳이다.

	MRPathNode* pStartNode = (MRPathNode*)m_pStartNode->m_pUserData;
	MRPathNode* pEndNode = (MRPathNode*)m_pEndNode->m_pUserData;
	_ASSERT(pStartNode!=NULL);
	_ASSERT(pEndNode!=NULL);

	/*
	MRPathNode::m_StartPos = rvector(m_StartPos.x, m_StartPos.y, 0);
	MRPathNode::m_EndPos = rvector(m_EndPos.x, m_EndPos.y, 0);
	*/
	MRPathNode::m_StartPos = m_StartPos;
	MRPathNode::m_EndPos = m_EndPos;
	MRPathNode::m_pStartNode = pStartNode;
	MRPathNode::m_pEndNode = pEndNode;

	//#define PATHFIND_DEPTH_LIMIT	200
#define PATHFIND_DEPTH_LIMIT	-1

	bool bFindPath = m_PolygonPathFinder.FindPath(pStartNode, pEndNode, PATHFIND_DEPTH_LIMIT, -1, true);
	if(bFindPath==false) return false;
	bFindPath = FindVisibilityPath(pVisibilityPathList, m_StartPos, m_EndPos, m_PolygonPathFinder.GetShortestPath(), &m_PolygonMapModel);

	return bFindPath;
}

bool RPathFinder::FindStraightPath(RVECTORLIST* pVisibilityPathList)
{

	return true;
}

void RPathFinder::Enlarge(float fMargin)
{
	m_fEnlarge = fMargin;
}

/*
class MergedPolygon : public list<rvector*>{
public:
	list<rvector*>::iterator FindSameVertex(list<rvector*>::iterator begin, list<rvector*>::iterator end){
		for(list<rvector*>::iterator i=begin; i
	}

	void Merge(list<rvector*>* pPolygon){
		for(iterator i=begin(); i!=end(); i++){
			for(list<vector*>::iterator j=pPolygon->begin(); j!=pPolygon->end(); j++){
				rvector* p1 = *i;
				rvector* p2 = *j;
				if(IS_EQ3(*p1, *p2)==true){
					
					insert(i, p2);
				}
			}
		}
	}
};
*/

enum MWINDINGDIR{
	MWD_NA = 0,
	MWD_CLOCKWISE = 1,
	MWD_COUNTERCLOCKWISE = 2,
};

bool IsIntersectLineAndHalfLine(float *pT1, float *pT2, rvector& Pos, rvector& Dir, rvector& s1, rvector& s2)
{
	float Bx = Pos.x + Dir.x;
	float By = Pos.y + Dir.y;
	float T1 = Bx-Pos.x;
	float T2 = s2.y-s1.y;
	float T3 = By-Pos.y;
	float T4 = s2.x-s1.x;
	float T5 = Pos.y-s1.y;
	float T6 = Pos.x-s1.x;

	float D = T1*T2 - T3*T4;

	if(D==0) return false;

	float t1 = (T5*T4 - T6*T2) / (float) D;
	float t2 = (T5*T1 - T6*T3) / (float) D;

	*pT1 = t1;
	*pT2 = t2;
	if(t1>=0.0f && t2>=0.0f && t2<=1.0f){
		return true;
	}

	return false;
}

MWINDINGDIR IsInnerPoint(float* t, rvector& p, RVERTEXLIST* pList)
{
	for(RVERTEXLIST::iterator i=pList->begin(); i!=pList->end(); i++){
		rvector* p1 = *i;
		if(IS_EQ(p.x, p1->x) && IS_EQ(p.y, p1->y)) return MWD_NA;	// 경계는 제외한다.
	}

	rvector Pos(p.x, p.y, 0);
	rvector Dir(0.2f, 0.33f, 0);	// (1,0,0)은 가로 Collision Line에 걸려서 체크가 안되는 경우가 있다.

	// 반직선에 교차되는 라인의 개수가 홀수이면 안쪽에 속해 있다.
	int nIntersectCount = 0;
	int nClockwiseCount = 0;
	int nCountClockwiseCount = 0;
	float tNear = FLT_MAX;
	for(RVERTEXLIST::iterator i=pList->begin(); i!=pList->end();){
		rvector* p1 = *i++;
		rvector* p2 = NULL;
		if(i!=pList->end()) p2 = *i;
		else p2 = *pList->begin();

		
		float t1, t2;
		if(IsIntersectLineAndHalfLine(&t1, &t2, Pos, Dir, *p1, *p2)==true){
			rvector cp;
			CrossProduct(&cp, rvector(p1->x, p1->y, 0)-rvector(p.x, p.y, 0), rvector(p2->x, p2->y, 0)-rvector(p.x, p.y, 0));
			if(cp.z>0) nClockwiseCount++;
			else nCountClockwiseCount++;
			nIntersectCount++;
			if(t1<tNear) tNear = t1;
		}
	}

	if(t!=NULL) *t = tNear;

	if(((nIntersectCount%2)==1)){
		// 교차된 선분의 방향이 Clockwise, Counter-clockwise 개수가 큰쪽이 그 방향으로 winding된 폴리곤이다.
		if(nClockwiseCount>nCountClockwiseCount) return MWD_CLOCKWISE;
		if(nClockwiseCount<nCountClockwiseCount) return MWD_COUNTERCLOCKWISE;
	}

	return MWD_NA;
}


bool IsSharedArea(MRPathNode* pMRNode1, MRPathNode* pMRNode2)
{
	RVERTEXLIST* pVertices1 = &pMRNode1->GetRPathNode()->vertices;
	RVERTEXLIST* pVertices2 = &pMRNode2->GetRPathNode()->vertices;
	for(RVERTEXLIST::iterator a=pVertices1->begin(); a!=pVertices1->end();){
		rvector* a1 = *a;
		a++;
		rvector* a2 = NULL;
		if(a!=pVertices1->end()) a2 = *a;
		else a2 = *pVertices1->begin();
		for(RVERTEXLIST::iterator b=pVertices2->begin(); b!=pVertices2->end();){
			rvector* b1 = *b;
			b++;
			rvector* b2 = NULL;
			if(b!=pVertices2->end()) b2 = *b;
			else b2 = *pVertices2->begin();

			float t1, t2;
			if(!( (IS_EQ(a1->x, b1->x) && IS_EQ(a1->y, b1->y)) || (IS_EQ(a2->x, b2->x) && IS_EQ(a2->y, b2->y)) ||
			(IS_EQ(a1->x, b2->x) && IS_EQ(a1->y, b2->y)) || (IS_EQ(a2->x, b1->x) && IS_EQ(a2->y, b1->y)))){
				if(MFGetIntersectOfSegmentAndSegment(&t1, &t2, a1->x, a1->y, a2->x, a2->y, b1->x, b1->y, b2->x, b2->y)==true){
					if(t1>0.01 && t1<0.99 && t2>0.01 && t2<0.99)
						return true;
				}
			}
		}
	}

	float t;
	for(RVERTEXLIST::iterator a=pVertices1->begin(); a!=pVertices1->end(); a++){
		rvector* a1 = *a;
		if(IsInnerPoint(&t, *a1, pVertices2)==MWD_COUNTERCLOCKWISE){
			return true;	// 포함되어 있되, 변에 붙어있지 않으면
		}
	}
	for(RVERTEXLIST::iterator b=pVertices2->begin(); b!=pVertices2->end(); b++){
		rvector* b1 = *b;
		if(IsInnerPoint(&t, *b1, pVertices1)==MWD_COUNTERCLOCKWISE){
			return true;
		}
	}

	return false;
}



// 해당 위치가 노드안에 존재하는가?
bool IsPosInNode(rvector& Pos, RPathNode* pNode)
{
	// 라인을 그어 Node의 라인들과 인터섹션되는 카운트가 1인지 확인한다.
	rvector Pos2 = Pos + rvector(1, 3.3f, 0);

	int nIntersectionCount = 0;
	for(RVERTEXLIST::iterator i=pNode->vertices.begin(); i!=pNode->vertices.end();){
		rvector* v1 = *i;
		i++;
		rvector* v2 = NULL;
		if(i!=pNode->vertices.end()) v2 = *i;
		else v2 = *pNode->vertices.begin();

		float t1, t2;
		if(MFGetIntersectOfSegmentAndSegment(&t1, &t2, v1->x, v1->y, v2->x, v2->y, Pos.x, Pos.y, Pos2.x, Pos2.y)==true){
			if(t1>=0 && t1<=1 && t2>=0) nIntersectionCount++;
		}
	}
	if(nIntersectionCount==1) return true;


	return false;
}

// 해당 노드가 연결되어 있는가?
int IsNodeConnection(RPathNode* pNode1, RPathNode* pNode2, RPathList* pPathList)
{
	int nCount = 0;
	for(int i=0; i<(int)pNode1->m_Neighborhoods.size(); i++){
		if(pPathList->at(pNode1->m_Neighborhoods[i]->nIndex)==pNode2) return i;
	}
	return -1;
}

// 패스가 포탈을 지나가는가?
bool IsPathAcrossPortal(rvector& Pos, rvector& NextPos, RPathNode* pNode, RPathNode* pNextNode, int nNodeNeighborhoodID, RPathList* pPathList)
{
	if(nNodeNeighborhoodID==-1){
		nNodeNeighborhoodID = IsNodeConnection(pNode, pNextNode, pPathList);
		if(nNodeNeighborhoodID==-1) return false;
	}

	int nSharedVertex1 = pNode->m_Neighborhoods[nNodeNeighborhoodID]->nEdge;
	int nSharedVertex2 = nSharedVertex1+1;
	if((int)pNode->vertices.size()>=nSharedVertex2) nSharedVertex2 = 0;

	rvector v1 = *(pNode->vertices[nSharedVertex1]);
	rvector v2 = *(pNode->vertices[nSharedVertex2]);

	float t1, t2;
	if(MFGetIntersectOfSegmentAndSegment(&t1, &t2, v1.x, v1.y, v2.x, v2.y, Pos.x, Pos.y, NextPos.x, NextPos.y)==true){
		if(t1<0 || t1>1 || t2<0 || t2>1) return false;
	}

	return true;
}

bool IsPathInStartNode(rvector& Pos, rvector& NextPos, RPathNode* pNode, RPathNode* pNextNode, int nNodeNeighborhoodID, RPathList* pPathList)
{
	if(nNodeNeighborhoodID==-1){
		nNodeNeighborhoodID = IsNodeConnection(pNode, pNextNode, pPathList);
		if(nNodeNeighborhoodID==-1) return false;
	}

	int nCount = 0;
	for(RVERTEXLIST::iterator i=pNode->vertices.begin(); i!=pNode->vertices.end(); nCount++){
		rvector* v1 = *i;
		i++;
		rvector* v2 = NULL;
		if(i!=pNode->vertices.end()) v2 = *i;
		else v2 = *pNode->vertices.begin();

		if(nCount==nNodeNeighborhoodID) continue;

		float t1, t2;
		if(MFGetIntersectOfSegmentAndSegment(&t1, &t2, v1->x, v1->y, v2->x, v2->y, Pos.x, Pos.y, NextPos.x, NextPos.y)==true){
			if(t1>=0 && t1<=1 && t2>=0 && t2<=1) return false;
		}
	}

	return true;
}

// 시작점과 끝점의 연결선이 시작노드와 끝노드안에서 벽과 인터섹션이 발생하지 않는가?
bool IsPathInNode(rvector& Pos, rvector& NextPos, RPathNode* pNode, RPathNode* pNextNode, int nNodeNeighborhoodID1, int nNodeNeighborhoodID2, RPathList* pPathList)
{
	if(IsPathInStartNode(Pos, NextPos, pNode, pNextNode, nNodeNeighborhoodID1, pPathList)==false) return false;
	if(IsPathInStartNode(NextPos, Pos, pNextNode, pNode, nNodeNeighborhoodID2, pPathList)==false) return false;
	return true;
}


_NAMESPACE_REALSPACE2_END
