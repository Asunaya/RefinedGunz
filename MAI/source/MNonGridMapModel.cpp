#include "MNonGridMapModel.h"
#include <math.h>

MPointNode::MPointNode(void)
{
	m_pPreviousPoint = m_pNextPoint = NULL;
	m_pTemp = NULL;
	m_bPreCalcVisiblePoint = false;
}

MPointNode::MPointNode(float x, float y)
{
	m_fX = x, m_fY = y;
	m_pPreviousPoint = m_pNextPoint = NULL;
	m_pTemp = NULL;
	m_bPreCalcVisiblePoint = false;
}

void MPointNode::ResetVisibleList(void)
{
	m_VisibleList.DeleteAll();
}

void MPointNode::ResetConnectPoint(void)
{
	m_pPreviousPoint = m_pNextPoint = NULL;
}

bool MPointNode::IsVisible(MPointNode* pPN)
{
	for(int i=0; i<m_VisibleList.GetCount(); i++){
		MPointNode* pVPN = m_VisibleList.Get(i);
		if(pVPN==pPN) return true;
	}
	return false;
}

int MPointNode::GetSuccessorCount(void)
{
	return m_VisibleList.GetCount();
}

MNodeModel* MPointNode::GetSuccessor(int i)
{
	return m_VisibleList.Get(i);
}

float MPointNode::GetSuccessorCost(MNodeModel* pSuccessor)
{
	return GetHeuristicCost(pSuccessor);
}

float MPointNode::GetHeuristicCost(MNodeModel* pNode)
{
	MPointNode* pPointNode = (MPointNode *)pNode;
	return (float)sqrt(pow(pPointNode->m_fX-m_fX, 2)+pow(pPointNode->m_fY-m_fY, 2));
}

MPolygonObstacle::MPolygonObstacle(void)
{
	m_bEnclosed = false;
	m_nType = MPT_NORMAL;
	m_bInnerGoal = false;
}

MPolygonObstacle::MPolygonObstacle(MPointNode* lt, MPointNode* rt, MPointNode* rb, MPointNode* lb)
{
	Add(lt);
	Add(rt);
	Add(rb);
	Add(lb);
	Enclose(true);

	_ASSERT(lt->m_fX==lb->m_fX);
	_ASSERT(rt->m_fX==rb->m_fX);
	_ASSERT(lt->m_fY==rt->m_fY);
	_ASSERT(lb->m_fY==rb->m_fY);

	m_nType = MPT_RECT;
	m_bInnerGoal = false;
}

void MPolygonObstacle::Add(MPointNode* pPointNode)
{
	_ASSERT(m_nType!=MPT_RECT);		// 직사각형 타입의 Polygon이 아니여야 한다.
	m_PointList.Add(pPointNode);
	int nCount = m_PointList.GetCount();
	if(nCount>1){
		MPointNode* pPrevPoint = m_PointList.Get(nCount-2);
		pPointNode->m_pPreviousPoint = pPrevPoint;
		//_ASSERT(pPrevPoint->m_pNextPoint==NULL);
		pPrevPoint->m_pNextPoint = pPointNode;
	}
}

void MPolygonObstacle::Delete(int i)
{
	_ASSERT(m_nType!=MPT_RECT);						// 직사각형 타입의 Polygon이 아니여야 한다.
	_ASSERT(i==0 || i==m_PointList.GetCount()-1);	// 처음 내지 끝 포인트만 지울 수 있다.
	MPointNode* pPN = m_PointList.Get(i);
	if(pPN->m_pPreviousPoint!=NULL){
		_ASSERT(pPN->m_pPreviousPoint->m_pNextPoint==pPN);
		pPN->m_pPreviousPoint->m_pNextPoint = NULL;
	}
	if(pPN->m_pNextPoint!=NULL){
		_ASSERT(pPN->m_pNextPoint->m_pPreviousPoint==pPN);
		pPN->m_pNextPoint->m_pPreviousPoint = NULL;
	}
	pPN->ResetConnectPoint();
	m_PointList.Delete(i);
}

void MPolygonObstacle::Enclose(bool bEnclosed)
{
	int nCount = m_PointList.GetCount();
	if(bEnclosed==true){
		if(nCount>0){
			MPointNode* pFirstPoint = m_PointList.Get(0);
			MPointNode* pLastPoint = m_PointList.Get(nCount-1);
			pLastPoint->m_pNextPoint = pFirstPoint;
			pFirstPoint->m_pPreviousPoint = pLastPoint;
		}
	}
	else{
		if(nCount>0){
			MPointNode* pFirstPoint = m_PointList.Get(0);
			MPointNode* pLastPoint = m_PointList.Get(nCount-1);
			pLastPoint->m_pNextPoint = NULL;
			pFirstPoint->m_pPreviousPoint = NULL;
		}
	}
	m_bEnclosed = bEnclosed;
}

bool IsPointInSegment(float sx1, float sy1, float sx2, float sy2, float x, float y)
{
	float x1 = sx1 - x;
	float y1 = sy1 - y;
	float x2 = sx2 - x;
	float y2 = sy2 - y;
	float z1 = x1*y2 - y1*x2;
	if(z1>0) return true;
	return false;
}
bool MPolygonObstacle::IsInnerPoint(float x, float y)
{
	if(m_bEnclosed==false) return false;

	int nCount = m_PointList.GetCount();
	if(nCount<=0) return false;

	MPointNode* p1 = m_PointList.Get(0);
	for(int i=0; i<nCount; i++){
		MPointNode* p2 = m_PointList.Get((i+1)%nCount);
		if(IsPointInSegment(p1->m_fX, p1->m_fY, p2->m_fX, p2->m_fY, x, y)==false) return false;
		p1 = p2;
	}

	return true;
}

MPointNode* MPolygonObstacle::GetStartPoint(void)
{
	return m_PointList.Get(0);
}

MPointNode* MPolygonObstacle::GetEndPoint(void)
{
	return m_PointList.Get(m_PointList.GetCount()-1);
}

MPointNode* MPolygonObstacle::Get(int i)
{
	return m_PointList.Get(i);
}

void MPolygonObstacle::Add(MPolygonObstacle* pPO)
{
	for(int i=0; i<pPO->m_PointList.GetCount(); i++){
		Add(pPO->m_PointList.Get(i));
	}
}

void MPolygonObstacle::GetPointNormal(float* x, float* y, int i)
{
	MPointNode* pPN = m_PointList.Get(i);
	MPointNode* pPPN = NULL;
	MPointNode* pNPN = NULL;
	int nCount = m_PointList.GetCount();

	if(m_bEnclosed==true){
		pPPN = m_PointList.Get((i+nCount-1)%nCount);
		pNPN = m_PointList.Get((i+1)%nCount);
	}
	else{
		if(i-1>=0) pPPN = m_PointList.Get(i+1);
		if(i+1<nCount) pNPN = m_PointList.Get(i+1);
	}

	float pnx = 0;
	float pny = 0;
	float nnx = 0;
	float nny = 0;
	if(pPPN!=NULL){
		pnx = pPN->m_fX - pPPN->m_fX;
		pny = pPN->m_fY - pPPN->m_fY;
	}
	if(pNPN!=NULL){
		nnx = pNPN->m_fX - pPN->m_fX;
		nny = pNPN->m_fY - pPN->m_fY;
	}

	// 90도 회전
	float temp;
	temp = pnx;
	pnx = -pny;
	pny = temp;
	temp = nnx;
	nnx = -nny;
	nny = temp;

	// Normalize;
	if(pnx!=0 || pny!=0){
		float fLen = (float)sqrt(pow(pnx, 2)+pow(pny, 2));
		pnx /= fLen;
		pny /= fLen;
	}
	if(nnx!=0 || nny!=0){
		float fLen = (float)sqrt(pow(nnx, 2)+pow(nny, 2));
		nnx /= fLen;
		nny /= fLen;
	}

	*x = pnx + nnx;
	*y = pny + nny;
	float fLen = (float)sqrt(pow(*x, 2)+pow(*y, 2));
	*x /= fLen;
	*y /= fLen;
}

void MPolygonObstacle::Shrink(float fMargin)
{
	// 포인트 위치에 따라 주위 값이 변경되므로, 값을 다 알아낸 다음
	CMLinkedList<MPointNode> PL;
	for(int i=0; i<m_PointList.GetCount(); i++){
		MPointNode* pPN = m_PointList.Get(i);
		float x, y;
		GetPointNormal(&x, &y, i);
		MPointNode* p = new MPointNode(pPN->m_fX + x * fMargin, pPN->m_fY + y * fMargin);
		PL.Add(p);
	}

	// 한번에 변경
	for(i=0; i<PL.GetCount(); i++){
		MPointNode* p = PL.Get(i);
		MPointNode* pPN = m_PointList.Get(i);
		pPN->m_fX = p->m_fX;
		pPN->m_fY = p->m_fY;
	}
}



/*
MRectObstacle::MRectObstacle(MPointNode* lt, MPointNode* rt, MPointNode* rb, MPointNode* lb)
{
	Add(lt);
	Add(rt);
	Add(rb);
	Add(lb);
	Enclose(true);

	_ASSERT(lt->m_fX==lb->m_fX);
	_ASSERT(rt->m_fX==rb->m_fX);
	_ASSERT(lt->m_fY==rt->m_fY);
	_ASSERT(lb->m_fY==rb->m_fY);
}
*/
/*
            (Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy)
        r = -----------------------------  (eqn 1)
            (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
            (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
        s = -----------------------------  (eqn 2)
            (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
*/
bool GetIntersectOfSegmentAndSegment(float *pT1, float *pT2, float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Dx, float Dy)
{
	float T1 = Bx-Ax;
	float T2 = Dy-Cy;
	float T3 = By-Ay;
	float T4 = Dx-Cx;
	float T5 = Ay-Cy;
	float T6 = Ax-Cx;

	float D = T1*T2 - T3*T4;

	if(D==0) return false;

	float t1 = (T5*T4 - T6*T2) / (float) D;
	float t2 = (T5*T1 - T6*T3) / (float) D;
	//if(t1>=0.0f && t1<=1.0f && t2>=0.0f && t2<=1.0f){
	//if(t1>0.0f && t1<1.0f && t2>0.0f && t2<1.0f){
	if(t1>=0.0f && t1<=1.0f && t2>0.0f && t2<1.0f){		// 타겟 선분의 양 끝은 포함시키지 않음.
		*pT1 = t1;
		*pT2 = t2;
		return true;
	}

	return false;
}
/*
bool MNonGridMapModel::IsVisiblePoint(MPointNode* p1, MPointNode* p2)
{
	if(p1->m_pPreviousPoint==p2 || p1->m_pNextPoint==p2) return true;
	// 폴리곤에 소속된 포인트이면, 안쪽에 점이 위치해 있는지 체크해 봐야 한다.
	if(p1->m_pPreviousPoint!=NULL && p1->m_pNextPoint!=NULL){
		int x1 = p1->m_pPreviousPoint->m_fX - p1->m_fX;
		int y1 = p1->m_pPreviousPoint->m_fY - p1->m_fY;
		int x2 = p2->m_fX - p1->m_fX;
		int y2 = p2->m_fY - p1->m_fY;
		int z1 = x1*y2 - y1*x2;
		int x3 = p2->m_fX - p1->m_fX;
		int y3 = p2->m_fY - p1->m_fY;
		int x4 = p1->m_pNextPoint->m_fX - p1->m_fX;
		int y4 = p1->m_pNextPoint->m_fY - p1->m_fY;
		int z2 = x3*y4 - y3*x4;

		int x5 = p1->m_pPreviousPoint->m_fX - p1->m_fX;
		int y5 = p1->m_pPreviousPoint->m_fY - p1->m_fY;
		int x6 = p1->m_pNextPoint->m_fX - p1->m_fX;
		int y6 = p1->m_pNextPoint->m_fY - p1->m_fY;
		if(x5*y6-y5*x6<=0){
			if(z1<=0 && z2<=0) return false;	// 안쪽이면 보이지 않는다.
		}
		else{
			if(z1<=0 || z2<=0) return false;	// 안쪽이면 보이지 않는다.
		}
	}
	float t1, t2;
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPolygonObstacle* pPolygon = m_PolygonList.Get(i);
		int nCount = pPolygon->m_PointList.GetCount();
		for(int p=0; p<(pPolygon->m_bEnclosed==true?nCount:nCount-1); p++){
			MPointNode* pp1 = pPolygon->m_PointList.Get(p);
			MPointNode* pp2 = pPolygon->m_PointList.Get((p+1)%nCount);

			if(p1==pp1 || p1==pp2 || p2==pp1 || p2==pp2) continue;

			// p1에서 p2로 선을 그엇을때 Segment와 겹치는가?
			bool bIntersect = GetIntersectOfSegmentAndSegment(&t1, &t2, p1->m_fX, p1->m_fY, p2->m_fX, p2->m_fY,
				pp1->m_fX, pp1->m_fY, pp2->m_fX, pp2->m_fY);
			if(bIntersect==true){
				// Cross Product ( 선분 p1s1과 선분 p1s2를 CrossProduct했을 경우 0 보다 크면 이 교점은 벽이다. )
				int x1 = pp1->m_fX - p1->m_fX;
				int y1 = pp1->m_fY - p1->m_fY;
				int x2 = pp2->m_fX - p1->m_fX;
				int y2 = pp2->m_fY - p1->m_fY;
				int z = x1*y2 - y1*x2;
				if(z<=0) return false;
			}
		}
	}

	return true;
}
*/
bool IsVisiblePoint(float x1, float y1, float x2, float y2, float x3, float y3, float px, float py)
{
	float sx1 = x1 - x2;
	float sy1 = y1 - y2;
	float sx2 = px - x2;
	float sy2 = py - y2;
	float z1 = sx1*sy2 - sy1*sx2;
	float sx3 = px - x2;
	float sy3 = py - y2;
	float sx4 = x3 - x2;
	float sy4 = y3 - y2;
	float z2 = sx3*sy4 - sy3*sx4;

	float sx5 = x1 - x2;
	float sy5 = y1 - y2;
	float sx6 = x3 - x2;
	float sy6 = y3 - y2;
	if(sx5*sy6-sy5*sx6<=0){
		if(z1<=0 && z2<=0) return false;	// 안쪽이면 보이지 않는다.
	}
	else{
		if(z1<=0 || z2<=0) return false;	// 안쪽이면 보이지 않는다.
	}
	return true;
}

int MNonGridMapModel::IsBiDirectionVisiblePoint(MPointNode* p1, MPointNode* p2)	// 0: 없음, 1: p1->p2, 2:p2->p1, 3:p1<->p2
{
	//if(p1->m_pPreviousPoint==p2 || p1->m_pNextPoint==p2) return 3;	// 양쪽 다 보임

	bool bP1ToP2 = true;
	bool bP2ToP1 = true;
	// 폴리곤에 소속된 포인트이면, 안쪽에 점이 위치해 있는지 체크해 봐야 한다.
	if((p1->m_pPreviousPoint!=NULL && p1->m_pNextPoint!=NULL) && (p2!=p1->m_pPreviousPoint && p2!=p1->m_pNextPoint)){
		bP1ToP2 = ::IsVisiblePoint(p1->m_pPreviousPoint->m_fX, p1->m_pPreviousPoint->m_fY,
			p1->m_fX, p1->m_fY, p1->m_pNextPoint->m_fX, p1->m_pNextPoint->m_fY,
			p2->m_fX, p2->m_fY);
	}
	if((p2->m_pPreviousPoint!=NULL && p2->m_pNextPoint!=NULL) && (p1!=p2->m_pPreviousPoint && p1!=p2->m_pNextPoint)){
		bP2ToP1 = ::IsVisiblePoint(p2->m_pPreviousPoint->m_fX, p2->m_pPreviousPoint->m_fY,
			p2->m_fX, p2->m_fY, p2->m_pNextPoint->m_fX, p2->m_pNextPoint->m_fY,
			p1->m_fX, p1->m_fY);
	}
	if(bP1ToP2==false && bP2ToP1==false) return 0;

	float t1, t2;
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPolygonObstacle* pPolygon = m_PolygonList.Get(i);
		//if(pPolygon->m_PointList.Get(0)->m_bPreCalcVisiblePoint==true);
		if(pPolygon->m_bInnerGoal==true) continue;	// 이 폴리곤안에 목표점이 있으면 충돌 체크에 없는 것으로 우선 간주
		if(pPolygon->m_nType==MPT_RECT){	// Rect형으로 정규화가 된 경우는 위치에 따라 분류
			float l = pPolygon->m_PointList.Get(0)->m_fX;
			float t = pPolygon->m_PointList.Get(0)->m_fY;
			float r = pPolygon->m_PointList.Get(2)->m_fX;
			float b = pPolygon->m_PointList.Get(2)->m_fY;
			float px = p2->m_fX - p1->m_fX;
			float py = p2->m_fY - p1->m_fY;

			// Culling...
			if(px>0 && py<0){			// 1 사분면
				if(r<p1->m_fX || t>p1->m_fY) continue;
				if(l>p2->m_fX || b<p2->m_fY) continue;
			}
			else if(px<0 && py<0){		// 2 사분면
				if(l>p1->m_fX || t>p1->m_fY) continue;
				if(r<p2->m_fX || b<p2->m_fY) continue;
			}
			else if(px<0 && py>0){		// 3 사분면
				if(l>p1->m_fX || b<p1->m_fY) continue;
				if(r<p2->m_fX || t>p2->m_fY) continue;
			}
			else if(px>0 && py>0){		// 4 사분면
				if(r<p1->m_fX || b<p1->m_fY) continue;
				if(l>p2->m_fX || t>p2->m_fY) continue;
			}
			
			if(px>0 && py==0){
				if(r<p1->m_fX) continue;
				if(t>p1->m_fY) continue;
				if(b<p1->m_fY) continue;
			}
			else if(px<0 && py==0){
				if(l>p1->m_fX) continue;
				if(t>p1->m_fY) continue;
				if(b<p1->m_fY) continue;
			}
			else if(px==0 && py>0){
				if(b<p1->m_fY) continue;
				if(l>p1->m_fX) continue;
				if(r<p1->m_fX) continue;
			}
			else if(px==0 && py<0){
				if(t>p1->m_fY) continue;
				if(l>p1->m_fX) continue;
				if(r<p1->m_fX) continue;
			}
		}
		int nCount = pPolygon->m_PointList.GetCount();
		for(int p=0; p<(pPolygon->m_bEnclosed==true?nCount:nCount-1); p++){
			MPointNode* pp1 = pPolygon->m_PointList.Get(p);
			MPointNode* pp2 = pPolygon->m_PointList.Get((p+1)%nCount);

			if(p1==pp1 || p1==pp2 || p2==pp1 || p2==pp2) continue;

			// p1에서 p2로 선을 그엇을때 Segment와 겹치는가?
			bool bIntersect = GetIntersectOfSegmentAndSegment(&t1, &t2, p1->m_fX, p1->m_fY, p2->m_fX, p2->m_fY,
				pp1->m_fX, pp1->m_fY, pp2->m_fX, pp2->m_fY);
			if(bIntersect==true){
				/*
				int cx = p1->m_fX + t1*(p2->m_fX-p1->m_fX);
				int cy = p1->m_fY + t1*(p2->m_fY-p1->m_fY);
				*/
				// Cross Product ( 선분 s1s2과 선분 p1p2를 CrossProduct했을 경우 0 보다 크면 이 교점은 벽이다. )
				float x1 = pp2->m_fX - pp1->m_fX;
				float y1 = pp2->m_fY - pp1->m_fY;
				float x2 = p2->m_fX - p1->m_fX;
				float y2 = p2->m_fY - p1->m_fY;
				float z = x1*y2 - y1*x2;
				if(z>0) bP1ToP2 = false;
				else if(z<0) bP2ToP1 = false;
				if(bP1ToP2==false && bP2ToP1==false) return 0;
			}
		}
	}

	if(bP1ToP2==true && bP2ToP1==true) return 3;
	else if(bP1ToP2==true) return 1;
	else if(bP2ToP1==true) return 2;
	return 0;
}

MNonGridMapModel::MNonGridMapModel(void)
{
	m_pCurrPolygon = NULL;
	m_pStartPoint = NULL;
	m_pEndPoint = NULL;
	m_bEndPointInPolygon = false;
}

void MNonGridMapModel::Destroy(void)
{
	m_pCurrPolygon = NULL;
	m_pStartPoint = NULL;
	m_pEndPoint = NULL;
	m_bEndPointInPolygon = false;
	m_PointList.DeleteAll();
	m_PolygonList.DeleteAll();
}

MPointNode* MNonGridMapModel::AddPoint(float x, float y, bool bCheckDup)
{
	if(bCheckDup==true){
		for(int i=0; i<m_PointList.GetCount(); i++){
			MPointNode* pPN = m_PointList.Get(i);
			if(pPN->m_fX==x && pPN->m_fY==y) return pPN;
		}
	}
	MPointNode* pNode = new MPointNode;
	pNode->m_fX = x;
	pNode->m_fY = y;
	m_PointList.Add(pNode);
	return pNode;
}

MPointNode* MNonGridMapModel::AddStartPoint(float x, float y)
{
	m_pStartPoint = AddPoint(x, y);
	return m_pStartPoint;
}

MPointNode* MNonGridMapModel::AddEndPoint(float x, float y)
{
	m_pEndPoint = AddPoint(x, y);
	return m_pEndPoint;
}

MPolygonObstacle* MNonGridMapModel::AddPolygon(MPointNode* pNode, MPolygonObstacleADDTYPE nType)
{
	if(m_pCurrPolygon==NULL){
		m_pCurrPolygon = new MPolygonObstacle;
		m_PolygonList.Add(m_pCurrPolygon);
	}

	MPolygonObstacle* pReturn = m_pCurrPolygon;
	
	m_pCurrPolygon->Add(pNode);
	
	if(nType==MPAT_END){
		m_pCurrPolygon = NULL;
	}
	else if(nType==MPAT_ENCLOSE){
		m_pCurrPolygon->Enclose(true);
		m_pCurrPolygon = NULL;
	}

	return pReturn;
}

MPolygonObstacle* MNonGridMapModel::AddRect(float x1, float y1, float x2, float y2, bool bInverse)
{
	MPointNode* pLT = AddPoint(x1, y1);
	MPointNode* pRT = AddPoint(x2, y1);
	MPointNode* pRB = AddPoint(x2, y2);
	MPointNode* pLB = AddPoint(x1, y2);
	MPolygonObstacle* pRectObstacle;
	if(bInverse==false) pRectObstacle = new MPolygonObstacle(pLT, pRT, pRB, pLB);
	else pRectObstacle = new MPolygonObstacle(pRT, pLT, pLB, pRB);
	m_PolygonList.Add(pRectObstacle);
	return pRectObstacle;
}

void MNonGridMapModel::GenerateVisiblePoint(void)
{
	for(int i=0; i<m_PointList.GetCount(); i++){
		MPointNode* p1 = m_PointList.Get(i);
		p1->ResetVisibleList();
	}

	m_bEndPointInPolygon = false;
	if(m_pEndPoint!=NULL){
		for(i=0; i<m_PolygonList.GetCount(); i++){
			MPolygonObstacle* pPolygon = m_PolygonList.Get(i);
			if(pPolygon->IsInnerPoint(m_pEndPoint->m_fX, m_pEndPoint->m_fY)==true){
				pPolygon->m_bInnerGoal = true;
				m_bEndPointInPolygon = true;
			}
			else
				pPolygon->m_bInnerGoal = false;
		}
	}

	for(i=0; i<m_PointList.GetCount(); i++){
		MPointNode* p1 = m_PointList.Get(i);

		if((p1->m_pNextPoint==NULL && p1->m_pPreviousPoint==NULL) && (p1!=m_pStartPoint && p1!=m_pEndPoint)) continue;

		for(int t=i+1; t<m_PointList.GetCount(); t++){
			MPointNode* p2 = m_PointList.Get(t);
			if((p2->m_pNextPoint==NULL && p2->m_pPreviousPoint==NULL) && (p2!=m_pStartPoint && p2!=m_pEndPoint)) continue;

			/*
			if(p1->m_bPreCalcVisiblePoint==true && p2->m_bPreCalcVisiblePoint==true){
				if(p1->IsVisible(p2)==false) continue;	// 이미 안보이는 점이므로..
			}*/

			int nResult = IsBiDirectionVisiblePoint(p1, p2);
			if(nResult==1) p1->m_VisibleList.Add(p2);
			else if(nResult==2) p2->m_VisibleList.Add(p1);
			else if(nResult==3){
				p1->m_VisibleList.Add(p2);
				p2->m_VisibleList.Add(p1);
			}
		}
	}
}

void MNonGridMapModel::Reset(void)
{
	m_PointList.DeleteAll();
	m_PolygonList.DeleteAll();
}

bool MNonGridMapModel::IsEndPointInPolygon(void)
{
	return m_bEndPointInPolygon;
}

void MNonGridMapModel::Add(MNonGridMapModel* pNGMM, bool bVisibleTest)
{
	// 폴리곤과 포인트 추가
	for(int i=0; i<pNGMM->m_PolygonList.GetCount(); i++){
		MPolygonObstacle* pPO = pNGMM->m_PolygonList.Get(i);
		int nPointCount = pPO->m_PointList.GetCount();
		for(int p=0; p<nPointCount; p++){
			MPointNode* pPN = pPO->m_PointList.Get(p);
			MPointNode* pThisPN = AddPoint(pPN->m_fX, pPN->m_fY);
			pThisPN->m_pTemp = pPN;	// 임시로 원래 포인터를 대입한다.
			pPN->m_pTemp = pThisPN;	// 임시로 추가되는 포인터를 대입한다.
			if(p==nPointCount-1){
				if(pPO->m_bEnclosed==true)
					AddPolygon(pThisPN, MPAT_ENCLOSE);
				else
					AddPolygon(pThisPN, MPAT_END);
			}
			else
				AddPolygon(pThisPN);
		}
	}

	// Visible
	for(int p=0; p<m_PointList.GetCount(); p++){
		MPointNode* pPN = m_PointList.Get(p);
		pPN->m_bPreCalcVisiblePoint = true;
		MPointNode* pOrigPN = (MPointNode *)pPN->m_pTemp;
		for(int v=0; v<pOrigPN->m_VisibleList.GetCount(); v++){
			MPointNode* pVP = pOrigPN->m_VisibleList.Get(v);
			pVP->m_VisibleList.Add((MPointNode *)pVP->m_pTemp);
		}
	}
}

void MNonGridMapModel::MergeBreakPolygon(void)
{
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPolygonObstacle* pPO = m_PolygonList.Get(i);
		MPointNode* pPN = pPO->GetEndPoint();

		if(pPO->m_bEnclosed==true) continue;

		for(int j=0; j<m_PolygonList.GetCount(); j++){
			MPolygonObstacle* pDPO = m_PolygonList.Get(j);
			if(pPO==pDPO) continue;

			if(pDPO->m_bEnclosed==false){
				MPointNode* pDPN = pDPO->GetStartPoint();
				if(pPN->m_fX==pDPN->m_fX && pPN->m_fY==pDPN->m_fY){	// pPO의 끝점과 pDPO의 시작점 같은 경우 합친다.
					//m_PointList
					pDPO->Delete(0);				// 첫번째 포인트를 지우고 연결
					pPO->Add(pDPO);
					m_PolygonList.Delete(j);
					//j--;
					pPN = pPO->GetEndPoint();		// 길이가 길어졌으므로 End Point 다시 계산
					j = 0;							// 다시 처음부터 시작
				}
			}
		}
	}

	// Enclose Test
	for(i=0; i<m_PolygonList.GetCount(); i++){
		MPolygonObstacle* pPO = m_PolygonList.Get(i);
		if(pPO->m_bEnclosed==true) continue;

		MPointNode* pSPN = pPO->GetStartPoint();
		MPointNode* pEPN = pPO->GetEndPoint();
		if(pSPN->m_fX==pEPN->m_fX && pSPN->m_fY==pEPN->m_fY){
			pPO->m_PointList.Delete(pPO->m_PointList.GetCount()-1);
			pPO->m_bEnclosed = true;
		}	
	}

	// Rebuild Connection
	for(i=0; i<m_PointList.GetCount(); i++){
		MPointNode* p1 = m_PointList.Get(i);
		p1->ResetConnectPoint();
	}
	for(i=0; i<m_PolygonList.GetCount(); i++){
		MPolygonObstacle* pPO = m_PolygonList.Get(i);

		for(int p=0; p<pPO->m_PointList.GetCount(); p++){
			MPointNode* pPPN = NULL;
			MPointNode* pCPN = NULL;
			//MPointNode* pNPN = NULL;
			if(p>0) pPPN = pPO->m_PointList.Get(p-1);
			pCPN = pPO->m_PointList.Get(p);
			//if(p<pPO->m_PointList.GetCount()-1) pNPN = pPO->m_PointList.Get(p+1);

			_ASSERT(pCPN->m_pPreviousPoint==NULL);
			_ASSERT(pCPN->m_pNextPoint==NULL);
			if(pPPN!=NULL) pPPN->m_pNextPoint = pCPN;
			//if(pNPN!=NULL) pNPN->m_pPreviousPoint = pCPN;
			pCPN->m_pPreviousPoint = pPPN;
			//pCPN->m_pPreviousPoint = pCPN;
		}

		if(pPO->m_bEnclosed==true){
			MPointNode* pSPN = pPO->GetStartPoint();
			MPointNode* pEPN = pPO->GetEndPoint();
			pSPN->m_pPreviousPoint = pEPN;
			pEPN->m_pNextPoint = pSPN;
		}
	}
}

void MNonGridMapModel::Shrink(float fMargin)
{
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPolygonObstacle* pPO = m_PolygonList.Get(i);
		pPO->Shrink(fMargin);
	}
}
