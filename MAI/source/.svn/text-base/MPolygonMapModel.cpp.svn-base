#include "MPolygonMapModel.h"
#include <math.h>
#include "rutils.h"
#include "MUtil.h"
#include <float.h>
#include <crtdbg.h>

#define ISIN(_sv, _ev, _vv)	((_sv<=_ev)?(_vv>=_sv && _vv<=_ev):(_vv>=_ev && _vv<=_sv))


bool MPMPoint::IsParentVisiblePoint(MPMPoint* p)
{
	// parent가 이미 보인다고 체크했던 점인지 검사.
	if(m_pParent==NULL) return false;

	int nCount = p->m_VisibledList.GetCount();
	if(nCount==0) return false;
	for(int i=0; i<nCount; i++){
		MPMPoint* pp = p->m_VisibledList.Get(i);
		if(pp==m_pParent) return true;
	}

	return ((MPMPoint*)m_pParent)->IsParentVisiblePoint(p);
}

void MPMPoint::GenerateVisiblePoint(void)
{
	if(m_pParentMap->GetChangeCount()==m_nLastChangeCount) return;

	// 시작점이면 모두 Visible Flag를 클리어하고 시작한다.
	if(this==m_pParentMap->m_pStartPoint) m_pParentMap->ClearVisibledList();

	ResetVisibleList();

	// 포탈에선 제외
	//if((GetNextPoint()==NULL && GetPrevPoint()==NULL) && (this!=m_pParentMap->m_pStartPoint && this!=m_pParentMap->m_pEndPoint)) return;

	// 최종 목표점을 먼저 계산하고, 보이는 점이면 다른 점들은 찾지 않는다.
	if(m_pParentMap->IsVisibleEdgePoint(this, m_pParentMap->m_pEndPoint, GetMinRoomIndex(), m_pParentMap->m_pEndPoint->GetMinRoomIndex())==true){
		AddVisiblePoint(m_pParentMap->m_pEndPoint);
		m_nLastChangeCount = m_pParentMap->GetChangeCount();
		return;
	}

	for(int i=0; i<m_pParentMap->m_PointList.GetCount(); i++){
		MPMPoint* p2 = m_pParentMap->m_PointList.Get(i);
		if(p2==this) continue;
		//if(p2->m_bVisibleFlag==true) continue;
		if(IsParentVisiblePoint(p2)==true) continue;
		if(p2==m_pParentMap->m_pStartPoint) continue;	// 시작점은 당연히 제외
		if(p2==m_pParentMap->m_pEndPoint) continue;		// 앞서서 먼저 계산하므로...

		// Portal 이 있으므로 이건 제외
		//if((p2->GetNextPoint()==NULL && p2->GetPrevPoint()==NULL) && (p2!=m_pParentMap->m_pStartPoint && p2!=m_pParentMap->m_pEndPoint)) continue;

		if(m_pParentMap->IsVisibleEdgePoint(this, p2, GetMinRoomIndex(), p2->GetMinRoomIndex())==true) AddVisiblePoint(p2);
	}

	m_nLastChangeCount = m_pParentMap->GetChangeCount();
}

void MPMPoint::ResetVisibleList(void)
{
	m_VisibleList.DeleteAll();
	m_nLastChangeCount = 0;
}

void MPMPoint::AddVisiblePoint(MPMPoint* pPoint)
{
	m_VisibleList.Add(pPoint);
	pPoint->m_VisibledList.Add(this);
}

MPMPoint::MPMPoint(void)
{
	m_pPrevPoint = m_pNextPoint = NULL;
	m_pTemp = NULL;
	m_pParentMap = NULL;
	m_nLastChangeCount = 0;
}

MPMPoint::MPMPoint(float x, float y, float z, int nRoomIndex)
{
	m_fX = x, m_fY = y, m_fZ = z;
	m_pPrevPoint = m_pNextPoint = NULL;
	m_pParentMap = NULL;
	m_pTemp = NULL;
	m_nLastChangeCount = 0;
	m_RoomIndexs.insert(m_RoomIndexs.end(), nRoomIndex);
}

MPMPoint::MPMPoint(float x, float y, float z, list<int>& RoomIndexs)
{
	m_fX = x, m_fY = y, m_fZ = z;
	m_pPrevPoint = m_pNextPoint = NULL;
	m_pParentMap = NULL;
	m_pTemp = NULL;
	m_nLastChangeCount = 0;
	m_RoomIndexs.merge(RoomIndexs);
}

void MPMPoint::ResetConnectPoint(void)
{
	m_pPrevPoint = m_pNextPoint = NULL;
}

int MPMPoint::GetVisiblePointCount(void)
{
	return m_VisibleList.GetCount();
}
MPMPoint* MPMPoint::GetVisiblePoint(int i)
{
	return m_VisibleList.Get(i);
}

bool MPMPoint::IsVisible(MPMPoint* pPN)
{
	for(int i=0; i<GetVisiblePointCount(); i++){
		MPMPoint* pVPN = GetVisiblePoint(i);
		if(pVPN==pPN) return true;
	}
	return false;
}

int MPMPoint::GetSuccessorCount(void)
{
	GenerateVisiblePoint();
	return GetVisiblePointCount();
}

MNodeModel* MPMPoint::GetSuccessor(int i)
{
	GenerateVisiblePoint();
	return GetVisiblePoint(i);
}

float MPMPoint::GetSuccessorCost(MNodeModel* pSuccessor)
{
	return GetHeuristicCost(pSuccessor);
}

float MPMPoint::GetHeuristicCost(MNodeModel* pNode)
{
	MPMPoint* pPointNode = (MPMPoint *)pNode;
	return (float)sqrt(pow(pPointNode->GetX()-m_fX, 2)+pow(pPointNode->GetY()-m_fY, 2));
}

int MPMPoint::GetMinRoomIndex(void)
{
	_ASSERT(m_RoomIndexs.size()!=0);
	if(m_RoomIndexs.size()==0) return 0;

	int nMin = INT_MAX;

	for(list<int>::iterator it1=m_RoomIndexs.begin(); it1!=m_RoomIndexs.end(); it1++){
		int nValue = *it1;
		if(nValue<nMin) nMin = nValue;
	}

	return nMin;
}

int MPMPoint::GetMaxRoomIndex(void)
{
	_ASSERT(m_RoomIndexs.size()!=0);
	if(m_RoomIndexs.size()==0) return 0;

	int nMax = 0;

	for(list<int>::iterator it1=m_RoomIndexs.begin(); it1!=m_RoomIndexs.end(); it1++){
		int nValue = *it1;
		if(nValue>nMax) nMax = nValue;
	}

	return nMax;
}

MPMPOLYGONDIR MPMPolygon::TestPolygonDir(void)
{
	// 닫혀있는 폴리곤만 테스트 할 수 있다.
	if(m_bEnclosed==false) return MPMPD_NA;

	// 가장 오른쪽에 있는 점 찾기
	MPMPoint* pRightestPoint = NULL;
	for(int i=0; i<m_PointList.GetCount(); i++){
		MPMPoint* pPoint = m_PointList.Get(i);
		if(pRightestPoint==NULL || pRightestPoint->GetX()>pPoint->GetX()){
			pRightestPoint = pPoint;
		}
	}

	_ASSERT(pRightestPoint!=NULL);
	MPMPoint* pPrevPoint = pRightestPoint->GetPrevPoint();
	MPMPoint* pNextPoint = pRightestPoint->GetNextPoint();
	rvector cp = CrossProduct(rvector(pPrevPoint->GetX(), pPrevPoint->GetY(), 0)-rvector(pRightestPoint->GetX(), pRightestPoint->GetY(), 0), rvector(pNextPoint->GetX(), pNextPoint->GetY(), 0)-rvector(pRightestPoint->GetX(), pRightestPoint->GetY(), 0));
	if(cp.z>0) return MPMPD_COUNTERCLOCKWISE;
	return MPMPD_CLOCKWISE;
}

void MPMPolygon::Initialize(void)
{
	m_bEnclosed = false;
	m_nType = MPMPT_NORMAL;
	m_bInnerStartPoint = false;
	m_bInnerEndPoint = false;
	m_nPolygonDir = MPMPD_NA;

	m_nLatestFindIndex = -1;
	m_pMergeTestBegin = NULL;
	m_pMergeTestEnd = NULL;
	//m_nMergeBeginIndex = m_nMergeEndIndex = -1;
}

MPMPolygon::MPMPolygon(void)
{
	Initialize();
}

MPMPolygon::MPMPolygon(MPMPoint* lt, MPMPoint* rt, MPMPoint* rb, MPMPoint* lb)
{
	Initialize();

	Add(lt);
	Add(rt);
	Add(rb);
	Add(lb);
	Enclose(true);

	_ASSERT(lt->GetX()==lb->GetX());
	_ASSERT(rt->GetX()==rb->GetX());
	_ASSERT(lt->GetY()==rt->GetY());
	_ASSERT(lb->GetY()==rb->GetY());

	m_nType = MPMPT_RECT;
}

void MPMPolygon::Add(MPMPoint* pPointNode)
{
	_ASSERT(m_nType!=MPMPT_RECT);		// 직사각형 타입의 Polygon이 아니여야 한다.
	m_PointList.Add(pPointNode);
	int nCount = m_PointList.GetCount();
	if(nCount>1){
		MPMPoint* pPrevPoint = m_PointList.Get(nCount-2);
		pPointNode->m_pPrevPoint = pPrevPoint;
		//_ASSERT(pPrevPoint->m_pNextPoint==NULL);
		pPrevPoint->m_pNextPoint = pPointNode;
	}
}

void MPMPolygon::Delete(int i)
{
	_ASSERT(m_nType!=MPMPT_RECT);						// 직사각형 타입의 Polygon이 아니여야 한다.
	_ASSERT(i==0 || i==m_PointList.GetCount()-1);	// 처음 내지 끝 포인트만 지울 수 있다.
	MPMPoint* pPN = m_PointList.Get(i);
	if(pPN->m_pPrevPoint!=NULL){
		_ASSERT(pPN->m_pPrevPoint->m_pNextPoint==pPN);
		pPN->m_pPrevPoint->m_pNextPoint = NULL;
	}
	if(pPN->m_pNextPoint!=NULL){
		_ASSERT(pPN->m_pNextPoint->m_pPrevPoint==pPN);
		pPN->m_pNextPoint->m_pPrevPoint = NULL;
	}
	pPN->ResetConnectPoint();
	m_PointList.Delete(i);
}

void MPMPolygon::Enclose(bool bEnclosed)
{
	int nCount = m_PointList.GetCount();
	if(bEnclosed==true){
		if(nCount>0){
			MPMPoint* pFirstPoint = m_PointList.Get(0);
			MPMPoint* pLastPoint = m_PointList.Get(nCount-1);
			pLastPoint->m_pNextPoint = pFirstPoint;
			pFirstPoint->m_pPrevPoint = pLastPoint;
		}
	}
	else{
		if(nCount>0){
			MPMPoint* pFirstPoint = m_PointList.Get(0);
			MPMPoint* pLastPoint = m_PointList.Get(nCount-1);
			pLastPoint->m_pNextPoint = NULL;
			pFirstPoint->m_pPrevPoint = NULL;
		}
	}
	m_bEnclosed = bEnclosed;

	if(bEnclosed==true)
		m_nPolygonDir = TestPolygonDir();
}

// 하나 이상의 연속된 에지를 공유하는 두 폴리곤을 머지하는 인터페이스, 머지하려는 폴리곤의 포인트를 하나씩 추가해 나가는 방식
void MPMPolygon::Merge(MPMPoint* pPointNode)
{
	_ASSERT(m_nType!=MPMPT_RECT);		// 직사각형 타입의 Polygon이 아니여야 한다.
	_ASSERT(m_bEnclosed==true);			// 닫힌 폴리곤에 머지하는 경우이다.

	int nFindIndex = FindIndex(pPointNode->GetX(), pPointNode->GetY(), pPointNode->GetZ(), m_pMergeTestBegin, m_pMergeTestEnd);
	if(nFindIndex==-1){
		// 이미 Dup에 추가된 포인트가 있으면, 현재 폴리곤에 삽입
		if(m_TempMergedPointDupList.GetCount()>0){
			_ASSERT(m_nLatestFindIndex>=0);
			AddAfter(m_nLatestFindIndex, pPointNode);
			m_pMergeTestBegin = pPointNode->m_pPrevPoint;
			m_pMergeTestEnd = pPointNode->m_pNextPoint;
		}
		else{
			m_TempMergedPointAddList.Add(pPointNode);
		}
	}
	else{
		// 공통 폴리곤을 가진 포인트
		MPMPoint* pFindPoint = m_PointList.Get(nFindIndex);
		pFindPoint->m_RoomIndexs.merge(pPointNode->m_RoomIndexs);

		m_TempMergedPointDupList.Add(pPointNode);
		// 이미 추가된 포인트가 있으면, 현재 폴리곤에 삽입
		if(m_TempMergedPointAddList.GetCount()>0){
			InsertBefore(nFindIndex, &m_TempMergedPointAddList);
			m_pMergeTestBegin = m_TempMergedPointAddList.Get(m_TempMergedPointAddList.GetCount()-1)->m_pPrevPoint;
			m_pMergeTestEnd = m_TempMergedPointAddList.Get(0)->m_pNextPoint;
			m_TempMergedPointAddList.DeleteAll();
		}
		m_nLatestFindIndex = nFindIndex;
	}
}

void MPMPolygon::MergeReset(void)
{
	m_nLatestFindIndex = -1;
	m_TempMergedPointAddList.DeleteAll();
	m_TempMergedPointDupList.DeleteAll();
}

void MPMPolygon::AddAfter(int nIndex, MPMPoint* pPointNode)
{
	MPMPoint* pThis = m_PointList.Get(nIndex);
	MPMPoint* pNext = pThis->m_pNextPoint;
	pThis->m_pNextPoint = pPointNode;
	pPointNode->m_pPrevPoint = pThis;
	pPointNode->m_pNextPoint = pNext;
	if(pNext!=NULL) pNext->m_pPrevPoint = pPointNode;

	m_PointList.MoveRecord(nIndex);
	m_PointList.AddAfter(pPointNode);
}

void MPMPolygon::InsertBefore(int nIndex, MPMPoint* pPointNode)
{
	MPMPoint* pThis = m_PointList.Get(nIndex);
	MPMPoint* pPrev = pThis->m_pPrevPoint;
	pThis->m_pPrevPoint = pPointNode;
	pPointNode->m_pNextPoint = pThis;
	pPointNode->m_pPrevPoint = pPrev;
	if(pPrev!=NULL) pPrev->m_pNextPoint = pPointNode;

	m_PointList.MoveRecord(nIndex);
	m_PointList.InsertBefore(pPointNode);
}

void MPMPolygon::InsertBefore(int nIndex, CMPtrList<MPMPoint>* pList)
{
	for(int i=pList->GetCount()-1; i>=0; i--){
		MPMPoint* p = pList->Get(i);
		InsertBefore(nIndex, p);
	}
}

void MPMPolygon::AddAfter(int nIndex, CMPtrList<MPMPoint>* pList)
{
	for(int i=0; i<pList->GetCount(); i++){
		MPMPoint* p = pList->Get(i);
		AddAfter(nIndex++, p);
	}
}


#ifndef TOLER
#define TOLER 0.001
#define IS_EQ(a,b) ((fabs((double)(a)-(b)) >= (double) TOLER) ? 0 : 1)
#endif

MPMPoint* MPMPolygon::Find(float x, float y, float z, MPMPoint* pTestBegin, MPMPoint* pTestEnd)
{
	if(pTestBegin==NULL || pTestEnd==NULL){
		pTestBegin = m_PointList.Get(0);
		pTestEnd = m_PointList.Get(m_PointList.GetCount()-1);
	}

	MPMPoint* p = pTestBegin;
	do{
		if(IS_EQ(x, p->GetX())==true && IS_EQ(y, p->GetY())==true && IS_EQ(z, p->GetZ())==true) return p;
		p = p->m_pNextPoint;
	}while(p!=pTestEnd->m_pNextPoint || p==NULL);

	return NULL;
}

int MPMPolygon::FindIndex(float x, float y, float z, MPMPoint* pTestBegin, MPMPoint* pTestEnd)
{
	/*
	int nCount = 0;

	if(nBeginIndex==-1 || nEndIndex){
		nBeginIndex = 0;
		nCount = m_PointList.GetCount();
	}
	else if(nEndIndex>nBeginIndex) nCount = nEndIndex - nBeginIndex + 1;
	else nCount = nEndIndex+m_PointList.GetCount() - nBeginIndex + 1;


	for(int i=nBeginIndex; i<nCount; i++){
		MPMPoint* p = m_PointList.Get(i%m_PointList.GetCount());
		if(IS_EQ(x, p->GetX())==true && IS_EQ(y, p->GetY())==true && IS_EQ(z, p->GetZ())==true) return i;
	}
	*/
	/*
	for(int i=0; i<m_PointList.GetCount(); i++){
		MPMPoint* p = m_PointList.Get(i);
		if(IS_EQ(x, p->GetX())==true && IS_EQ(y, p->GetY())==true && IS_EQ(z, p->GetZ())==true) return i;
	}
	return -1;
	*/

	MPMPoint* p = Find(x, y, z, pTestBegin, pTestEnd);
	if(p==NULL) return -1;
	return GetIndex(p);
}

int MPMPolygon::GetIndex(MPMPoint* pTest)
{
	for(int i=0; i<m_PointList.GetCount(); i++){
		MPMPoint* p = m_PointList.Get(i);
		if(p==pTest) return i;
	}
	return -1;
}


bool MPMPolygon::IsInnerPoint(float x, float y, bool bTestClockwise)
{
	MPMPOLYGONDIR d = IsInnerPoint(NULL, x, y);
	if(d==MPMPD_CLOCKWISE && bTestClockwise==true) return true;
	if(d==MPMPD_COUNTERCLOCKWISE && bTestClockwise==false) return true;

	return false;
}

MPMPOLYGONDIR MPMPolygon::IsInnerPoint(float* t, float x, float y)
{
	if(m_bEnclosed==false) return MPMPD_NA;

	// Rect인 경우
	if(m_nType==MPMPT_RECT){
		_ASSERT(m_PointList.GetCount()==4);	// Rect
		_ASSERT(m_PointList.Get(0)->GetX()==m_PointList.Get(3)->GetX());
		_ASSERT(m_PointList.Get(1)->GetX()==m_PointList.Get(2)->GetX());
		_ASSERT(m_PointList.Get(0)->GetY()==m_PointList.Get(1)->GetY());
		_ASSERT(m_PointList.Get(2)->GetY()==m_PointList.Get(3)->GetY());

		float fLeft = min(m_PointList.Get(0)->GetX(), m_PointList.Get(1)->GetX());
		float fRight = max(m_PointList.Get(0)->GetX(), m_PointList.Get(1)->GetX());
		float fTop = min(m_PointList.Get(0)->GetY(), m_PointList.Get(3)->GetY());
		float fBottom = max(m_PointList.Get(0)->GetY(), m_PointList.Get(3)->GetY());
		if(x>=fLeft && x<=fRight && y>=fTop && y<=fBottom) return GetDir();
		return MPMPD_NA;
	}

	int nCount = m_PointList.GetCount();
	if(nCount<=0) return MPMPD_NA;

	rvector Pos(x, y, 0);
	rvector Dir(0.2f, 0.33f, 0);	// (1,0,0)은 가로 Collision Line에 걸려서 체크가 안되는 경우가 있다.

	// 반직선에 교차되는 라인의 개수가 홀수이면 안쪽에 속해 있다.
	int nIntersectCount = 0;
	int nClockwiseCount = 0;
	int nCountClockwiseCount = 0;
	float tNear = FLT_MAX;
	for(int p=0; p<(m_bEnclosed==true?nCount:nCount-1); p++){
		MPMPoint* pp1 = m_PointList.Get(p);
		MPMPoint* pp2 = m_PointList.Get((p+1)%nCount);
		float t1, t2;
		if(MIsIntersectLineAndHalfLine(&t1, &t2, Pos, Dir, rvector(pp1->GetX(), pp1->GetY(), 0), rvector(pp2->GetX(), pp2->GetY(), 0))==true){
			rvector cp = CrossProduct(rvector(pp1->GetX(), pp1->GetY(), 0)-rvector(x, y, 0), rvector(pp2->GetX(), pp2->GetY(), 0)-rvector(x, y, 0));
			if(cp.z>0) nClockwiseCount++;
			else nCountClockwiseCount++;
			nIntersectCount++;
			if(t1<tNear) tNear = t1;
		}
	}

	if(t!=NULL) *t = tNear;

	if(((nIntersectCount%2)==1)){
		// 교차된 선분의 방향이 Clockwise, Counter-clockwise 개수가 큰쪽이 그 방향으로 winding된 폴리곤이다.
		if(nClockwiseCount>nCountClockwiseCount) return MPMPD_CLOCKWISE;
		if(nClockwiseCount<nCountClockwiseCount) return MPMPD_COUNTERCLOCKWISE;
	}

	return MPMPD_NA;
}

MPMPoint* MPMPolygon::GetStartPoint(void)
{
	return m_PointList.Get(0);
}

MPMPoint* MPMPolygon::GetEndPoint(void)
{
	return m_PointList.Get(m_PointList.GetCount()-1);
}

MPMPoint* MPMPolygon::Get(int i)
{
	return m_PointList.Get(i);
}

void MPMPolygon::Add(MPMPolygon* pPO)
{
	for(int i=0; i<pPO->m_PointList.GetCount(); i++){
		Add(pPO->m_PointList.Get(i));
	}
}

void MPMPolygon::GetPointNormal(float* x, float* y, int i)
{
	MPMPoint* pPN = m_PointList.Get(i);
	MPMPoint* pPPN = NULL;
	MPMPoint* pNPN = NULL;
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
		pnx = pPN->GetX() - pPPN->GetX();
		pny = pPN->GetY() - pPPN->GetY();
	}
	if(pNPN!=NULL){
		nnx = pNPN->GetX() - pPN->GetX();
		nny = pNPN->GetY() - pPN->GetY();
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

void MPMPolygon::Enlarge(float fMargin)
{
	if(fMargin==0.0f) return;

	// 포인트 위치에 따라 주위 값이 변경되므로, 값을 다 알아낸 다음
	CMLinkedList<MPMPoint> PL;
	for(int i=0; i<m_PointList.GetCount(); i++){
		MPMPoint* pPN = m_PointList.Get(i);
		if(pPN->m_InPortals.GetCount()>0) continue;	// 포탈에 속한 점들은 따로 계산한다.
		float x, y;
		GetPointNormal(&x, &y, i);
		MPMPoint* p = new MPMPoint(pPN->GetX() - x * fMargin, pPN->GetY() - y * fMargin, 0, pPN->m_RoomIndexs);
		PL.Add(p);
	}

	// 한번에 변경
	for(i=0; i<PL.GetCount(); i++){
		MPMPoint* p = PL.Get(i);
		MPMPoint* pPN = m_PointList.Get(i);
		pPN->m_fX = p->GetX();
		pPN->m_fY = p->GetY();
	}
}

void MPMPolygon::RebuildConnection(void)
{
	for(int p=0; p<m_PointList.GetCount(); p++){
		MPMPoint* pPPN = NULL;
		MPMPoint* pCPN = NULL;
		if(p>0) pPPN = m_PointList.Get(p-1);
		pCPN = m_PointList.Get(p);

		_ASSERT(pCPN->GetPrevPoint()==NULL);
		_ASSERT(pCPN->GetNextPoint()==NULL);
		if(pPPN!=NULL) pPPN->m_pNextPoint = pCPN;
		pCPN->m_pPrevPoint = pPPN;
	}

	if(m_bEnclosed==true){
		MPMPoint* pSPN = GetStartPoint();
		MPMPoint* pEPN = GetEndPoint();
		pSPN->m_pPrevPoint = pEPN;
		pEPN->m_pNextPoint = pSPN;
	}
}

bool MPMPolygon::IsIntersect(MPMPoint** p1, MPMPoint** p2, float* t, float x1, float y1, float x2, float y2)
{
	bool bIntersect = false;
	*t = FLT_MAX;
	float t1, t2;
	if(m_nType==MPMPT_RECT){	// Rect형으로 정규화가 된 경우는 위치에 따라 분류
		float l = m_PointList.Get(0)->GetX();
		float t = m_PointList.Get(0)->GetY();
		float r = m_PointList.Get(2)->GetX();
		float b = m_PointList.Get(2)->GetY();
		float px = x2 - x1;
		float py = y2 - y1;

		// Culling...
		if(px>0 && py<0){			// 1 사분면
			if(r<x1 || t>y1) return false;
			if(l>x2 || b<y2) return false;
		}
		else if(px<0 && py<0){		// 2 사분면
			if(l>x1 || t>y1) return false;
			if(r<x2 || b<y2) return false;
		}
		else if(px<0 && py>0){		// 3 사분면
			if(l>x1 || b<y1) return false;
			if(r<x2 || t>y2) return false;
		}
		else if(px>0 && py>0){		// 4 사분면
			if(r<x1 || b<y1) return false;
			if(l>x2 || t>y2) return false;
		}
		
		if(px>0 && py==0){
			if(r<x1) return false;
			if(t>y1) return false;
			if(b<y1) return false;
		}
		else if(px<0 && py==0){
			if(l>x1) return false;
			if(t>y1) return false;
			if(b<y1) return false;
		}
		else if(px==0 && py>0){
			if(b<y1) return false;
			if(l>x1) return false;
			if(r<x1) return false;
		}
		else if(px==0 && py<0){
			if(t>y1) return false;
			if(l>x1) return false;
			if(r<x1) return false;
		}
	}

	int nCount = m_PointList.GetCount();
	for(int p=0; p<(m_bEnclosed==true?nCount:nCount-1); p++){
		MPMPoint* pp1 = m_PointList.Get(p);
		MPMPoint* pp2 = m_PointList.Get((p+1)%nCount);

		// Cross Product ( 선분 s1s2과 선분 p1p2를 CrossProduct했을 경우 0 보다 크면 이 교점은 벽이다. )
		float cx1 = pp2->GetX() - pp1->GetX();
		float cy1 = pp2->GetY() - pp1->GetY();
		float cx2 = x2 - x1;
		float cy2 = y2 - y1;
		float z = cx1*cy2 - cy1*cx2;
		if(z<0) continue;

		// p1에서 p2로 선을 그엇을때 Segment와 겹치는가?
		if(MFGetIntersectOfSegmentAndSegment(&t1, &t2, x1, y1, x2, y2,
			pp1->GetX(), pp1->GetY(), pp2->GetX(), pp2->GetY())==true){
			if(t1<*t){
				*t = t1;
				*p1 = pp1;
				*p2 = pp2;
			}
			bIntersect = true;
		}
	}

	return bIntersect;
}

float MPMPolygon::GetNearestContactPoint(float* px, float* py, float x, float y, bool bInverse)
{
	float fNearestLength = FLT_MAX;
	rvector NearestPoint(0, 0, 0);
	float t;

	int nCount = m_PointList.GetCount();
	for(int p=0; p<(m_bEnclosed==true?nCount:nCount-1); p++){
		MPMPoint* p1 = NULL;
		MPMPoint* p2 = NULL;
		if(bInverse==false){
			p1 = m_PointList.Get(p);
			p2 = m_PointList.Get((p+1)%nCount);
		}
		else{
			p1 = m_PointList.Get((p+1)%nCount);
			p2 = m_PointList.Get(p);
		}

		rvector s(p1->GetX(), p1->GetY(), 0);
		rvector e(p2->GetX(), p2->GetY(), 0);
		rvector cp;
		MGetPointFromPointToLine(&cp, &t, rvector(x, y, 0), s, e);
		if(t>=0 && t<=1){
			rvector Diff = cp - rvector(x, y, 0);
			float fDiff = Diff.GetSafeMagnitude();
			if(fDiff<fNearestLength){
				NearestPoint = cp;
				fNearestLength = fDiff;
			}
		}
		else{
			rvector Diff = rvector(x, y, 0) - s;
			float fDiff = Diff.GetSafeMagnitude();
			if(fDiff<fNearestLength){
				NearestPoint = s;
				fNearestLength = fDiff;
			}
			Diff = rvector(x, y, 0) - e;
			fDiff = Diff.GetSafeMagnitude();
			if(fDiff<fNearestLength){
				NearestPoint = e;
				fNearestLength = fDiff;
			}
		}
	}

	*px = NearestPoint.x;
	*py = NearestPoint.y;

	return fNearestLength;
}


MPMPOLYGONDIR MPMPolygon::GetDir(void)
{
	_ASSERT(m_nPolygonDir==MPMPD_NA || m_nPolygonDir==MPMPD_CLOCKWISE || m_nPolygonDir==MPMPD_COUNTERCLOCKWISE);
	return m_nPolygonDir;
}



MPMPortal::MPMPortal(MPMPoint* pPos1, MPMPoint* pPos2, int nPortalIndex)
{
	m_pPos[0] = pPos1;
	m_pPos[1] = pPos2;
	m_nPortalIndex = nPortalIndex;
}

MPMPortal::~MPMPortal(void)
{
}

void MPMPortal::Enlarge(float fMargin)
{
	rvector d(m_pPos[1]->GetX()-m_pPos[0]->GetX(), m_pPos[1]->GetY()-m_pPos[0]->GetY(), m_pPos[1]->GetZ()-m_pPos[0]->GetZ());
	d = Normalize(d);
	m_pPos[0]->m_fX += (d.x*fMargin);
	m_pPos[0]->m_fY += (d.y*fMargin);
	m_pPos[0]->m_fZ += (d.z*fMargin);
	d = -d;
	m_pPos[1]->m_fX += (d.x*fMargin);
	m_pPos[1]->m_fY += (d.y*fMargin);
	m_pPos[1]->m_fZ += (d.z*fMargin);
}


void MPolygonMapModel::GetInnerClosedPolygon(CMPtrList<MPMPolygon>* pPolygons, int nPointType, MPMPOLYGONDIR dir)
{
	_ASSERT(dir!=MPMPD_NA);
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPMPolygon* pPolygon = m_PolygonList.Get(i);
		if(pPolygon->GetDir()!=dir) continue;
		if((nPointType==0 && pPolygon->m_bInnerStartPoint==true) || (nPointType==1 && pPolygon->m_bInnerEndPoint==true)){	// Start Point or End Point
			pPolygons->Add(pPolygon);
		}
	}
}

bool MPolygonMapModel::IsVisibleEdgePoint(MPMPoint* p1, MPMPoint* p2, int nBeginRoom, int nEndRoom)
{
	_ASSERT(nBeginRoom>=0 && nEndRoom<=m_PortalList.GetCount());

	bool bP1ToP2 = true;
	bool bP2ToP1 = true;

	// 맵 폴리곤에 대해서 체크 ( 포탈만 체크해보면 Visible인지 알 수 있다. )
	for(int i=nBeginRoom; i<nEndRoom; i++){
		MPMPortal* pPortal = m_PortalList.Get(i);

		/*
		// Cross Product ( 선분 s1s2과 선분 p1p2를 CrossProduct했을 경우 0 보다 크면 이 교점은 벽이다. )
		float x1 = pp2->GetX() - pp1->GetX();
		float y1 = pp2->GetY() - pp1->GetY();
		float x2 = p2->GetX() - p1->GetX();
		float y2 = p2->GetY() - p1->GetY();
		float z = x1*y2 - y1*x2;
		if(z<0) continue;
		*/

		if(p1==pPortal->m_pPos[0] || p1==pPortal->m_pPos[1]) continue;	// 시작 포인트가 공유를 통해 포탈에 속해있는 포인트이라면 visible로 보고 스킵
		if(p2==pPortal->m_pPos[0] || p2==pPortal->m_pPos[1]) continue;	// 목표 포인트가 공유를 통해 포탈에 속해있는 포인트이라면 visible

		// p1에서 p2로 선을 그엇을때 Segment와 겹치는가?
		float t1, t2;
		bool bIntersect = MFGetIntersectOfSegmentAndSegment(&t1, &t2, p1->GetX(), p1->GetY(), p2->GetX(), p2->GetY(),
			pPortal->m_pPos[0]->GetX(), pPortal->m_pPos[0]->GetY(), pPortal->m_pPos[1]->GetX(), pPortal->m_pPos[1]->GetY());
		/*
		bool bIntersect = MFGetIntersectOfSegmentAndSegment(&t1, &t2, p1->GetX(), p1->GetY(), p2->GetX(), p2->GetY(),
			pPortal->m_pPos[1]->GetX(), pPortal->m_pPos[1]->GetY(), pPortal->m_pPos[0]->GetX(), pPortal->m_pPos[0]->GetY());
		*/
		if(bIntersect==false){	// 포탈에 겹쳐져야 서로 보인다.
			return false;
		}
	}


	// 우선은 포탈만 테스트
	return true;


	// 폴리곤에 소속된 포인트이면, 안쪽에 점이 위치해 있는지 체크해 봐야 한다.
	if((p1->GetPrevPoint()!=NULL && p1->GetNextPoint()!=NULL) && (p2!=p1->GetPrevPoint() && p2!=p1->GetNextPoint())){
		bP1ToP2 = ::MFIsVisiblePoint(p1->GetPrevPoint()->GetX(), p1->GetPrevPoint()->GetY(),
			p1->GetX(), p1->GetY(), p1->GetNextPoint()->GetX(), p1->GetNextPoint()->GetY(),
			p2->GetX(), p2->GetY());

		if(bP1ToP2==false) return false;
	}
	if((p2->GetPrevPoint()!=NULL && p2->GetNextPoint()!=NULL) && (p1!=p2->GetPrevPoint() && p1!=p2->GetNextPoint())){
		// Edge가 아니므로, 고려할 필요가 없는 점이다.
		bool bEdge = ::MFIsEdgePoint(p2->GetPrevPoint()->GetX(), p2->GetPrevPoint()->GetY(),
			p2->GetX(), p2->GetY(), p2->GetNextPoint()->GetX(), p2->GetNextPoint()->GetY(),
			p1->GetX(), p1->GetY());
		if(bEdge==false) return false;

		bP2ToP1 = ::MFIsVisiblePoint(p2->GetPrevPoint()->GetX(), p2->GetPrevPoint()->GetY(),
			p2->GetX(), p2->GetY(), p2->GetNextPoint()->GetX(), p2->GetNextPoint()->GetY(),
			p1->GetX(), p1->GetY());
		if(bP2ToP1==false) return false;
	}

	/*
	// 갈 수 없는 맵으로 지정된 경우
	// ( 2D 프로젝션후 공유하는 면(복층)이 있으면 갈수 없는 곳이다. )
	if(p1->m_RoomIndexs.size()==1 && p2->m_RoomIndexs.size()==1){	// 한 폴리곤만을 가리킬때만 테스트(폴리곤 여러개를 공유하는 경우는 제외된다.)
		for(list<int>::iterator it1=p1->m_RoomIndexs.begin(); it1!=p1->m_RoomIndexs.end(); it1++){
			for(list<int>::iterator it2=p2->m_RoomIndexs.begin(); it2!=p2->m_RoomIndexs.end(); it2++){
				MPOLYGONIDPAIR pair;
				pair.a = *it1;
				pair.b = *it2;
				MPOLYGONIDPAIRMAP::iterator f = m_PolygonSharedMap.find(pair);
				//MPOLYGONIDPAIRMAP::iterator f = m_PolygonSharedMap.findif(m_PolygonSharedMap.begin(), m_PolygonSharedMap.end(), value_equals<MPOLYGONIDPAIR, boo>pair);
				if(f!=m_PolygonSharedMap.end()) return false;
			}
		}
	}
	*/

	float t1, t2;
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPMPolygon* pPolygon = m_PolygonList.Get(i);
		//if(pPolygon->m_bInnerEndPoint==true && pPolygon->GetDir()==MPMPD_CLOCKWISE) continue;			// 이 폴리곤안에 목표점이 있으면 충돌 체크에 없는 것으로 우선 간주
		//if(pPolygon->m_bInnerStartPoint==false && pPolygon->GetDir()==MPMPD_COUNTERCLOCKWISE) continue;	// Counter-Clockwise 폴리곤 밖에 시작점이 있으면.. 이 폴리곤 검색 제외
		if(pPolygon->m_nType==MPMPT_RECT){	// Rect형으로 정규화가 된 경우는 위치에 따라 분류
			float l = pPolygon->m_PointList.Get(0)->GetX();
			float t = pPolygon->m_PointList.Get(0)->GetY();
			float r = pPolygon->m_PointList.Get(2)->GetX();
			float b = pPolygon->m_PointList.Get(2)->GetY();
			float px = p2->GetX() - p1->GetX();
			float py = p2->GetY() - p1->GetY();

			// Culling...
			if(px>0 && py<0){			// 1 사분면
				if(r<p1->GetX() || t>p1->GetY()) continue;
				if(l>p2->GetX() || b<p2->GetY()) continue;
			}
			else if(px<0 && py<0){		// 2 사분면
				if(l>p1->GetX() || t>p1->GetY()) continue;
				if(r<p2->GetX() || b<p2->GetY()) continue;
			}
			else if(px<0 && py>0){		// 3 사분면
				if(l>p1->GetX() || b<p1->GetY()) continue;
				if(r<p2->GetX() || t>p2->GetY()) continue;
			}
			else if(px>0 && py>0){		// 4 사분면
				if(r<p1->GetX() || b<p1->GetY()) continue;
				if(l>p2->GetX() || t>p2->GetY()) continue;
			}
			
			if(px>0 && py==0){
				if(r<p1->GetX()) continue;
				if(t>p1->GetY()) continue;
				if(b<p1->GetY()) continue;
			}
			else if(px<0 && py==0){
				if(l>p1->GetX()) continue;
				if(t>p1->GetY()) continue;
				if(b<p1->GetY()) continue;
			}
			else if(px==0 && py>0){
				if(b<p1->GetY()) continue;
				if(l>p1->GetX()) continue;
				if(r<p1->GetX()) continue;
			}
			else if(px==0 && py<0){
				if(t>p1->GetY()) continue;
				if(l>p1->GetX()) continue;
				if(r<p1->GetX()) continue;
			}
		}
		int nCount = pPolygon->m_PointList.GetCount();
		for(int p=0; p<(pPolygon->m_bEnclosed==true?nCount:nCount-1); p++){
			MPMPoint* pp1 = pPolygon->m_PointList.Get(p);
			MPMPoint* pp2 = pPolygon->m_PointList.Get((p+1)%nCount);

			if(p1==pp1 || p1==pp2 || p2==pp1 || p2==pp2) continue;

			// RoomIndex에 의한 걸러내기
			// 경로에 포함되어 있지 않은 폴리곤에 속한 라인이라면 테스트에서 걸러낸다.
			//if(IsPolygonPathInclude(&pp1->m_RoomIndexs, &p1->m_RoomIndexs, &p2->m_RoomIndexs)==false) continue;
			//if(IsPolygonPathInclude(&pp2->m_RoomIndexs, &p1->m_RoomIndexs, &p2->m_RoomIndexs)==false) continue;
			/*
			if(p1->m_pRoomIndex!=NULL && p2->m_pRoomIndex!=NULL &&
				p1->m_pRoomIndex!=pp1->m_pRoomIndex && p1->m_pRoomIndex!=pp2->m_pRoomIndex && p2->m_pRoomIndex!=pp1->m_pRoomIndex && p2->m_pRoomIndex!=pp2->m_pRoomIndex){
				if(IsPolygonPathInclude(pp1->m_pRoomIndex, p1->m_pRoomIndex, p2->m_pRoomIndex)==false) continue;
				if(pp1->m_pRoomIndex!=pp2->m_pRoomIndex){
					if(IsPolygonPathInclude(pp2->m_pRoomIndex, p1->m_pRoomIndex, p2->m_pRoomIndex)==false) continue;
				}
			}
			*/

			// Cross Product ( 선분 s1s2과 선분 p1p2를 CrossProduct했을 경우 0 보다 크면 이 교점은 벽이다. )
			float x1 = pp2->GetX() - pp1->GetX();
			float y1 = pp2->GetY() - pp1->GetY();
			float x2 = p2->GetX() - p1->GetX();
			float y2 = p2->GetY() - p1->GetY();
			float z = x1*y2 - y1*x2;
			if(z<0) continue;

			// p1에서 p2로 선을 그엇을때 Segment와 겹치는가?
			bool bIntersect = MFGetIntersectOfSegmentAndSegment(&t1, &t2, p1->GetX(), p1->GetY(), p2->GetX(), p2->GetY(),
				pp1->GetX(), pp1->GetY(), pp2->GetX(), pp2->GetY());
			if(bIntersect==true){
				return false;
			}
		}
	}

	return true;
}

int MPolygonMapModel::IsBiDirectionVisiblePoint(float* pT1, float* pT2, MPMPoint* p1, MPMPoint* p2)	// 0: 없음, 1: p1->p2, 2:p2->p1, 3:p1<->p2
{
	//if(p1->m_pPrevPoint==p2 || p1->m_pNextPoint==p2) return 3;	// 양쪽 다 보임

	bool bP1ToP2 = true;
	bool bP2ToP1 = true;

	// 폴리곤에 소속된 포인트이면, 안쪽에 점이 위치해 있는지 체크해 봐야 한다.
	if((p1->GetPrevPoint()!=NULL && p1->GetNextPoint()!=NULL) && (p2!=p1->GetPrevPoint() && p2!=p1->GetNextPoint())){
		bP1ToP2 = ::MFIsVisiblePoint(p1->GetPrevPoint()->GetX(), p1->GetPrevPoint()->GetY(),
			p1->GetX(), p1->GetY(), p1->GetNextPoint()->GetX(), p1->GetNextPoint()->GetY(),
			p2->GetX(), p2->GetY());
	}
	if((p2->GetPrevPoint()!=NULL && p2->GetNextPoint()!=NULL) && (p1!=p2->GetPrevPoint() && p1!=p2->GetNextPoint())){
		// Edge가 아니므로, 고려할 필요가 없는 점이다.
		/*
		bool bEdge = ::MFIsEdgePoint(p2->GetPrevPoint()->GetX(), p2->GetPrevPoint()->GetY(),
			p2->GetX(), p2->GetY(), p2->GetNextPoint()->GetX(), p2->GetNextPoint()->GetY(),
			p1->GetX(), p1->GetY());
		if(bEdge==false) return 0;
		*/

		bP2ToP1 = ::MFIsVisiblePoint(p2->GetPrevPoint()->GetX(), p2->GetPrevPoint()->GetY(),
			p2->GetX(), p2->GetY(), p2->GetNextPoint()->GetX(), p2->GetNextPoint()->GetY(),
			p1->GetX(), p1->GetY());
	}
	if(bP1ToP2==false && bP2ToP1==false) return 0;

	float t1, t2;
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPMPolygon* pPolygon = m_PolygonList.Get(i);
		if(pPolygon->m_bInnerEndPoint==true && pPolygon->GetDir()==MPMPD_CLOCKWISE) continue;			// 이 폴리곤안에 목표점이 있으면 충돌 체크에 없는 것으로 우선 간주
		if(pPolygon->m_bInnerStartPoint==false && pPolygon->GetDir()==MPMPD_COUNTERCLOCKWISE) continue;	// Counter-Clockwise 폴리곤 밖에 시작점이 있으면.. 이 폴리곤 검색 제외
		if(pPolygon->m_nType==MPMPT_RECT){	// Rect형으로 정규화가 된 경우는 위치에 따라 분류
			float l = pPolygon->m_PointList.Get(0)->GetX();
			float t = pPolygon->m_PointList.Get(0)->GetY();
			float r = pPolygon->m_PointList.Get(2)->GetX();
			float b = pPolygon->m_PointList.Get(2)->GetY();
			float px = p2->GetX() - p1->GetX();
			float py = p2->GetY() - p1->GetY();

			// Culling...
			if(px>0 && py<0){			// 1 사분면
				if(r<p1->GetX() || t>p1->GetY()) continue;
				if(l>p2->GetX() || b<p2->GetY()) continue;
			}
			else if(px<0 && py<0){		// 2 사분면
				if(l>p1->GetX() || t>p1->GetY()) continue;
				if(r<p2->GetX() || b<p2->GetY()) continue;
			}
			else if(px<0 && py>0){		// 3 사분면
				if(l>p1->GetX() || b<p1->GetY()) continue;
				if(r<p2->GetX() || t>p2->GetY()) continue;
			}
			else if(px>0 && py>0){		// 4 사분면
				if(r<p1->GetX() || b<p1->GetY()) continue;
				if(l>p2->GetX() || t>p2->GetY()) continue;
			}
			
			if(px>0 && py==0){
				if(r<p1->GetX()) continue;
				if(t>p1->GetY()) continue;
				if(b<p1->GetY()) continue;
			}
			else if(px<0 && py==0){
				if(l>p1->GetX()) continue;
				if(t>p1->GetY()) continue;
				if(b<p1->GetY()) continue;
			}
			else if(px==0 && py>0){
				if(b<p1->GetY()) continue;
				if(l>p1->GetX()) continue;
				if(r<p1->GetX()) continue;
			}
			else if(px==0 && py<0){
				if(t>p1->GetY()) continue;
				if(l>p1->GetX()) continue;
				if(r<p1->GetX()) continue;
			}
		}
		int nCount = pPolygon->m_PointList.GetCount();
		for(int p=0; p<(pPolygon->m_bEnclosed==true?nCount:nCount-1); p++){
			MPMPoint* pp1 = pPolygon->m_PointList.Get(p);
			MPMPoint* pp2 = pPolygon->m_PointList.Get((p+1)%nCount);

			if(p1==pp1 || p1==pp2 || p2==pp1 || p2==pp2) continue;

			// p1에서 p2로 선을 그엇을때 Segment와 겹치는가?
			bool bIntersect = MFGetIntersectOfSegmentAndSegment(&t1, &t2, p1->GetX(), p1->GetY(), p2->GetX(), p2->GetY(),
				pp1->GetX(), pp1->GetY(), pp2->GetX(), pp2->GetY());
			if(bIntersect==true){
				/*
				int cx = p1->GetX() + t1*(p2->GetX()-p1->GetX());
				int cy = p1->GetY() + t1*(p2->GetY()-p1->GetY());
				*/
				// Cross Product ( 선분 s1s2과 선분 p1p2를 CrossProduct했을 경우 0 보다 크면 이 교점은 벽이다. )
				float x1 = pp2->GetX() - pp1->GetX();
				float y1 = pp2->GetY() - pp1->GetY();
				float x2 = p2->GetX() - p1->GetX();
				float y2 = p2->GetY() - p1->GetY();
				float z = x1*y2 - y1*x2;
				if(z>0) bP1ToP2 = false;
				else if(z<0) bP2ToP1 = false;
				*pT1 = t1;
				*pT2 = t2;
				if(bP1ToP2==false && bP2ToP1==false) return 0;
			}
		}
	}

	if(bP1ToP2==true && bP2ToP1==true) return 3;
	else if(bP1ToP2==true) return 1;
	else if(bP2ToP1==true) return 2;
	return 0;
}

bool MPolygonMapModel::IsIntersect(MPMPoint** p1, MPMPoint** p2, float* t, float x1, float y1, float x2, float y2)
{
	bool bP1ToP2 = true;

	bool bIntersect = false;
	*t = FLT_MAX;
	float tt;
	MPMPoint* pp1;
	MPMPoint* pp2;
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPMPolygon* pPolygon = m_PolygonList.Get(i);
		if(pPolygon->IsIntersect(&pp1, &pp2, &tt, x1, y1, x2, y2)==true){
			if(tt<*t){
				*t = tt;
				*p1 = pp1;
				*p2 = pp2;
				bIntersect = true;
			}
		}
	}

	return bIntersect;
}

MPolygonMapModel::MPolygonMapModel(void)
{
	m_pCurrPolygon = NULL;
	m_pStartPoint = NULL;
	m_pEndPoint = NULL;
	m_bEndPointInPolygon = false;
	m_nChangeCount++;
	m_pPolygonPathList = NULL;
}

MPolygonMapModel::~MPolygonMapModel(void)
{
	Destroy();
}

void MPolygonMapModel::Destroy(void)
{
	if(m_pPolygonPathList!=NULL){
		delete[] m_pPolygonPathList;
		m_pPolygonPathList = NULL;
	}
	m_pCurrPolygon = NULL;
	m_bEndPointInPolygon = false;
	Clear();
}

MPMPoint* MPolygonMapModel::AddPoint(float x, float y, float z, int nRoomIndex, bool bCheckDup)
{
	if(bCheckDup==true){
		for(int i=0; i<m_PointList.GetCount(); i++){
			MPMPoint* pPN = m_PointList.Get(i);
			if(pPN->GetX()==x && pPN->GetY()==y) return pPN;
		}
	}
	MPMPoint* pNode = new MPMPoint(x, y, z, nRoomIndex);
	pNode->m_pParentMap = this;
	m_PointList.Add(pNode);

	Change();
	return pNode;
}

MPMPoint* MPolygonMapModel::AddPoint(float x, float y, float z, list<int>& RoomIndexs, bool bCheckDup)
{
	if(bCheckDup==true){
		for(int i=0; i<m_PointList.GetCount(); i++){
			MPMPoint* pPN = m_PointList.Get(i);
			if(pPN->GetX()==x && pPN->GetY()==y) return pPN;
		}
	}
	MPMPoint* pNode = new MPMPoint(x, y, z, RoomIndexs);
	pNode->m_pParentMap = this;
	m_PointList.Add(pNode);

	Change();
	return pNode;
}

MPMPolygon* MPolygonMapModel::AddPolygon(MPMPoint* pNode, MPMPOLYGONADDTYPE nType)
{
	if(m_pCurrPolygon==NULL){
		m_pCurrPolygon = new MPMPolygon;
		m_PolygonList.Add(m_pCurrPolygon);
	}

	MPMPolygon* pReturn = m_pCurrPolygon;
	
	m_pCurrPolygon->Add(pNode);
	
	if(nType==MPMPAT_END){
		m_pCurrPolygon = NULL;
	}
	else if(nType==MPMPAT_ENCLOSE){
		m_pCurrPolygon->Enclose(true);
		m_pCurrPolygon = NULL;
	}

	Change();
	return pReturn;
}

MPMPolygon* MPolygonMapModel::AddRect(float x1, float y1, float z1, float x2, float y2, float z2, int pRoomIndexs[4], bool bInverse)
{
	MPMPoint* pLT = AddPoint(x1, y1, z1, pRoomIndexs[0]);
	MPMPoint* pRT = AddPoint(x2, y1, (z1+z2)/2, pRoomIndexs[1]);
	MPMPoint* pRB = AddPoint(x2, y2, z2, pRoomIndexs[2]);
	MPMPoint* pLB = AddPoint(x1, y2, (z1+z2)/2, pRoomIndexs[3]);
	MPMPolygon* pRectObstacle;
	if(bInverse==false) pRectObstacle = new MPMPolygon(pLT, pRT, pRB, pLB);
	else pRectObstacle = new MPMPolygon(pRT, pLT, pLB, pRB);
	m_PolygonList.Add(pRectObstacle);

	Change();
	return pRectObstacle;
}

void MPolygonMapModel::AddPoint(MPMPoint* pPoint)
{
	m_PointList.Add(pPoint);
	pPoint->m_pParentMap = this;
	Change();
}

void MPolygonMapModel::AddPolygon(MPMPolygon* pPolygon)
{
	m_PolygonList.Add(pPolygon);
	Change();
}

void MPolygonMapModel::AddStartPoint(MPMPoint* pPoint)
{
	AddPoint(pPoint);
	m_pStartPoint = pPoint;
	Change();
}

void MPolygonMapModel::AddEndPoint(MPMPoint* pPoint)
{
	AddPoint(pPoint);
	m_pEndPoint = pPoint;
	Change();
}

MPMPortal* MPolygonMapModel::AddPortal(MPMPoint* p1, MPMPoint* p2)
{
	MPMPortal* pPortal = new MPMPortal(p1, p2, m_PortalList.GetCount());
	m_PortalList.Add(pPortal);
	p1->m_InPortals.Add(pPortal);
	p2->m_InPortals.Add(pPortal);

	Change();

	return pPortal;
}

MPMPoint* MPolygonMapModel::AddStartPoint(float x, float y, float z, int nRoomIndex)
{
	m_pStartPoint = AddPoint(x, y, z, nRoomIndex);

	Change();
	return m_pStartPoint;
}

MPMPoint* MPolygonMapModel::AddEndPoint(float x, float y, float z, int nRoomIndex)
{
	m_pEndPoint = AddPoint(x, y, z, nRoomIndex);

	Change();
	return m_pEndPoint;
}

MPMPoint* MPolygonMapModel::GetStartPoint(void)
{
	return m_pStartPoint;
}

MPMPoint* MPolygonMapModel::GetEndPoint(void)
{
	return m_pEndPoint;
}

void MPolygonMapModel::SetStartPoint(float x, float y, float z, int nRoomIndex)
{
	if(m_pStartPoint==NULL) AddStartPoint(x, y, z, nRoomIndex);
	else{
		m_pStartPoint->m_fX = x;
		m_pStartPoint->m_fY = y;
		m_pStartPoint->m_fZ = z;
	}
	Change();
}

void MPolygonMapModel::SetEndPoint(float x, float y, float z, int nRoomIndex)
{
	if(m_pEndPoint==NULL) AddEndPoint(x, y, z, nRoomIndex);
	else{
		m_pEndPoint->m_fX = x;
		m_pEndPoint->m_fY = y;
		m_pEndPoint->m_fZ = z;
	}
	Change();
}

void MPolygonMapModel::Clear(void)
{
	m_PointList.DeleteAll();
	m_PolygonList.DeleteAll();
	m_PortalList.DeleteAll();

	m_pStartPoint = NULL;
	m_pEndPoint = NULL;

	Change();
}

/*
void MPolygonMapModel::ClearVisiblePoint(void)
{
	for(int i=0; i<m_PointList.GetCount(); i++){
		MPMPoint* p1 = m_PointList.Get(i);
		p1->ResetVisibleList();
	}
}

void MPolygonMapModel::GenerateVisiblePoint(void)
{
	ClearVisiblePoint();

	m_bEndPointInPolygon = false;
	if(m_pEndPoint!=NULL){
		for(int i=0; i<m_PolygonList.GetCount(); i++){
			MPMPolygon* pPolygon = m_PolygonList.Get(i);
			if(pPolygon->IsInnerPoint(m_pEndPoint->GetX(), m_pEndPoint->GetY())==true){
				pPolygon->m_bInnerEndPoint = true;
				m_bEndPointInPolygon = true;
			}
			else
				pPolygon->m_bInnerEndPoint = false;
		}
	}

	for(int i=0; i<m_PointList.GetCount(); i++){
		MPMPoint* p1 = m_PointList.Get(i);

		if((p1->GetNextPoint()==NULL && p1->GetPrevPoint()==NULL) && (p1!=m_pStartPoint && p1!=m_pEndPoint)) continue;

		for(int t=i+1; t<m_PointList.GetCount(); t++){
			MPMPoint* p2 = m_PointList.Get(t);
			if((p2->GetNextPoint()==NULL && p2->GetPrevPoint()==NULL) && (p2!=m_pStartPoint && p2!=m_pEndPoint)) continue;

			int nResult = IsBiDirectionVisiblePoint(p1, p2);
			if(nResult==1) p1->AddVisiblePoint(p2);
			else if(nResult==2) p2->AddVisiblePoint(p1);
			else if(nResult==3){
				p1->AddVisiblePoint(p2);
				p2->AddVisiblePoint(p1);
			}
		}
	}
}
*/

bool MPolygonMapModel::TestEndPointInPolygon(void)
{
	m_bEndPointInPolygon = false;
	if(m_pEndPoint!=NULL){
		for(int i=0; i<m_PolygonList.GetCount(); i++){
			MPMPolygon* pPolygon = m_PolygonList.Get(i);
			MPMPOLYGONDIR d = pPolygon->GetDir();
			if(pPolygon->IsInnerPoint(m_pEndPoint->GetX(), m_pEndPoint->GetY(), (d==MPMPD_CLOCKWISE)?true:false)==true){
				pPolygon->m_bInnerEndPoint = true;
				m_bEndPointInPolygon = true;
			}
			else{
				pPolygon->m_bInnerEndPoint = false;
			}
		}
	}

	return m_bEndPointInPolygon;
}

bool MPolygonMapModel::TestStartPointInPolygon(void)
{
	if(m_pStartPoint!=NULL){
		for(int i=0; i<m_PolygonList.GetCount(); i++){
			MPMPolygon* pPolygon = m_PolygonList.Get(i);
			MPMPOLYGONDIR d = pPolygon->GetDir();
			if(pPolygon->IsInnerPoint(m_pStartPoint->GetX(), m_pStartPoint->GetY(), (d==MPMPD_CLOCKWISE)?true:false)==true){
				pPolygon->m_bInnerStartPoint = true;
			}
			else{
				pPolygon->m_bInnerStartPoint = false;
			}
		}
	}

	return false;
}

bool MPolygonMapModel::TestPointInPolygon(float* pNearestPointX, float* pNearestPointY, float x, float y)
{
	float nx, ny;
	float nNearestLength = 0;
	bool bPointInPolygon = false;

	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPMPolygon* pPolygon = m_PolygonList.Get(i);
		if(pPolygon->IsInnerPoint(x, y, true)==true){
			float nLength = pPolygon->GetNearestContactPoint(&nx, &ny, x, y, true);
			if(bPointInPolygon==false || nLength<nNearestLength){
				*pNearestPointX = nx;
				*pNearestPointY = ny;
				nNearestLength = nLength;
			}
			bPointInPolygon = true;
		}
	}

	return bPointInPolygon;
}

bool MPolygonMapModel::RecommendPoint(float* pRecommendedPointX, float* pRecommendedPointY, float x, float y, float fCheckStepDistance, float fExponentialIncrease)
{
#define CHECK_ANGLE_COUNT	10
	*pRecommendedPointX = x;
	*pRecommendedPointY = y;
	if(IsInnerClosedPolygon(x, y)==NULL){
		return true;
	}
	int nCheckCount = 20;
	for(int i=0; i<nCheckCount; i++){
		for(float p=0; p<pi*2; p+=(2*pi/(float)CHECK_ANGLE_COUNT)){
			float fDistance = (float)pow((float)i, fExponentialIncrease);
			float rx = x + fCheckStepDistance * fDistance * (float)cos(p);
			float ry = y + fCheckStepDistance * fDistance * (float)sin(p);
			if(IsInnerClosedPolygon(rx, ry)==false){
				*pRecommendedPointX = rx;
				*pRecommendedPointY = ry;
				return true;
			}
		}
	}

	return false;
}

/*
bool MPolygonMapModel::RecommendEndPointByStartPointRange(float* pRecommendedPointX, float* pRecommendedPointY)
{
#define CHECK_ANGLE_COUNT	10
	*pRecommendedPointX = x;
	*pRecommendedPointY = y;
	if(IsInnerClosedPolygon(x, y)==NULL){
		return true;
	}
	for(int i=0; i<StartCCWPolygons.GetCount(); i++){
		MPMPolygon* pPolygon = StartCCWPolygons.Get(i);
		float rx, ry;
		if(pPolygon->m_bInnerStartPoint==true) continue;
		pPolygon->GetNearestContactPoint(&rx, &ry, x, y, true);
	}
	for(int i=0; i<)
	int nCheckCount = 20;
	for(int i=0; i<nCheckCount; i++){
		for(float p=0; p<pi*2; p+=(2*pi/(float)CHECK_ANGLE_COUNT)){
			float fDistance = pow(i, fExponentialIncrease);
			float rx = x + fCheckStepDistance * fDistance * (float)cos(p);
			float ry = y + fCheckStepDistance * fDistance * (float)sin(p);
			if(IsInnerClosedPolygon(rx, ry)==false){
				*pRecommendedPointX = rx;
				*pRecommendedPointY = ry;
				return true;
			}
		}
	}

	return false;
}
*/
// p1에 폴리곤들이 p2에 포함되어 있는가?
// p1 << p2
bool IsPolygonsIncluded(CMPtrList<MPMPolygon>* p1, CMPtrList<MPMPolygon>* p2)
{
	for(int i=0; i<p1->GetCount(); i++){
		MPMPolygon* pPolygon1 = p1->Get(i);
		bool bSamePolygon = false;
		for(int j=0; j<p2->GetCount(); j++){
			MPMPolygon* pPolygon2 = p2->Get(j);
			if(pPolygon1==pPolygon2){
				bSamePolygon = true;
				break;
			}
		}
		if(bSamePolygon==false) return false;
	}
	return true;
}

// p1에서 p2와 공통되는 부분 삭제
void RemovePolygonsIncluded(CMPtrList<MPMPolygon>* p1, CMPtrList<MPMPolygon>* p2)
{
	for(int i=0; i<p1->GetCount(); i++){
		MPMPolygon* pPolygon1 = p1->Get(i);
		for(int j=0; j<p2->GetCount(); j++){
			MPMPolygon* pPolygon2 = p2->Get(j);
			if(pPolygon1==pPolygon2){
				p1->Delete(i);
				i--;
				break;
			}
		}
	}
}

/*
bool MPolygonMapModel::RecommendEndPoint(float fCheckStepDistance, float fExponentialIncrease)
{
	// 시작점이 CounterClockwise방향의 폴리곤에 갇혀 있는 경우
	// 리스트로 체크하는 이유는 여러개의 CounterClockwise로 감싸져 있는 경우가 있기 때문에 가장 안쪽의 폴리곤을 알아내기 위해서이다.
	CMPtrList<MPMPolygon> StartCCWPolygons;
	IsInnerClosedPolygon(&StartCCWPolygons, m_pStartPoint->GetX(), m_pStartPoint->GetY(), false);
	if(StartCCWPolygons.GetCount()>0){
		CMPtrList<MPMPolygon> EndCCWPolygons;
		IsInnerClosedPolygon(&EndCCWPolygons, m_pEndPoint->GetX(), m_pEndPoint->GetY(), false);
		if(IsPolygonsIncluded(&StartCCWPolygons, &EndCCWPolygons)==true) return false;	// 같은 폴리곤안에 속해 있으면 그냥 서치하면 된다.
		else{
			// 폴리곤 밖에 있는경우 안쪽에 적당한 지점을 추천한다.
			// 시작점과 끝점을 그어서 가장 먼저 교차하는 지점은 0.9f 포지션 ( 가장 먼저 교차하는 라인이 항상 폴리곤 안에 있는 지점이다. )
			// 가장 안쪽 폴리곤의 점을 찾아 EndPoint로 지정
			for(int i=0; i<StartCCWPolygons.GetCount(); i++){
				MPMPolygon* pPolygon = StartCCWPolygons.Get(i);
				MPMPoint* p1;
				MPMPoint* p2;
				float t;
				if(pPolygon->IsIntersect(&p1, &p2, &t, m_pStartPoint->GetX(), m_pStartPoint->GetY(), m_pEndPoint->GetX(), m_pEndPoint->GetY())==true){
					float dx = m_pEndPoint->GetX() - m_pStartPoint->GetX();
					float dy = m_pEndPoint->GetY() - m_pStartPoint->GetY();
					dx*=(t*0.99f);
					dy*=(t*0.99f);

					float x, y;
					x = m_pStartPoint->GetX()+dx;
					y = m_pStartPoint->GetY()+dy;
					SetEndPoint(x, y);
					return true;
				}
			}
		}
	}
	else{
		// 목적지점이 Clockwise폴리곤 안에 갇혀 있는 경우, 적당한 외부점으로 빼주기
		float rx, ry;
		if(RecommendPoint(&rx, &ry, m_pEndPoint->GetX(), m_pEndPoint->GetY(), fCheckStepDistance, fExponentialIncrease)==true){
			SetEndPoint(rx, ry);
			return true;
		}
	}

	return false;
}
*/

bool MPolygonMapModel::FindNonIntersectPosition(CMPtrList<MPMPolygon>* pPolygons, float* rx, float* ry, float sx, float sy, float ex, float ey)
{
	bool bChanged = false;
	for(int i=0; i<pPolygons->GetCount(); i++){
		MPMPolygon* pPolygon = pPolygons->Get(i);
		MPMPoint* p1;
		MPMPoint* p2;
		float t;
#define NONINTERSECTIONESTIMATIONCONSTANT	0.99f
		if(pPolygon->IsIntersect(&p1, &p2, &t, sx, sy, ex, ey)==true){
			float dx = ex - sx;
			float dy = ey - sy;
			dx*=(t*NONINTERSECTIONESTIMATIONCONSTANT);
			dy*=(t*NONINTERSECTIONESTIMATIONCONSTANT);

			ex = sx+dx;
			ey = sy+dy;
			bChanged = true;
		}
	}

	*rx = ex;
	*ry = ey;
	return bChanged;
}

bool MPolygonMapModel::FindNonIntersectPosition(CMPtrList<MPMPolygon>* pPolygons, float* rx, float* ry, float x, float y)
{
	float nx = x;
	float ny = y;
	for(int i=0; i<pPolygons->GetCount(); i++){
		MPMPolygon* pPolygon = pPolygons->Get(i);
		bool bTestOK = pPolygon->GetDir()==MPMPD_CLOCKWISE?true:false;	// Clockwise는 폴리곤 바깥쪽으로 빼줘야 하며, CounterClockwise는 폴리곤 안쪽으로 빼줘야 한다.
		if(pPolygon->IsInnerPoint(nx, ny, pPolygon->GetDir()==MPMPD_CLOCKWISE?true:false)==bTestOK){
			pPolygon->GetNearestContactPoint(&nx, &ny, nx, ny, pPolygon->GetDir()==MPMPD_COUNTERCLOCKWISE?true:false);
			float dx = nx-x;
			float dy = ny-y;
			dx*=1.1f;
			dy*=1.1f;
			nx = x+dx;
			ny = y+dy;
		}
	}

	if(nx==x && ny==y) return false;
	*rx = nx;
	*ry = ny;
	return true;
}


bool MPolygonMapModel::RecommendEndPoint(float* pRecX, float* pRecY)
{
	// 시작점이 CounterClockwise방향의 폴리곤에 갇혀 있는 경우
	// 리스트로 체크하는 이유는 여러개의 CounterClockwise로 감싸져 있는 경우가 있기 때문에 가장 안쪽의 폴리곤을 알아내기 위해서이다.
	CMPtrList<MPMPolygon> StartCCWPolygons;
	GetInnerClosedPolygon(&StartCCWPolygons, 0, MPMPD_COUNTERCLOCKWISE);
	if(StartCCWPolygons.GetCount()>0){
		CMPtrList<MPMPolygon> EndCCWPolygons;
		GetInnerClosedPolygon(&EndCCWPolygons, 1, MPMPD_COUNTERCLOCKWISE);
		/*
		char temp[256];
		sprintf(temp, "EndCCWPolygons Count = %d\n", EndCCWPolygons.GetCount());
		OutputDebugString(temp);
		*/
		if(IsPolygonsIncluded(&StartCCWPolygons, &EndCCWPolygons)==true);// return false;	// 같은 폴리곤안에 속해 있으면 그냥 서치하면 된다.
		else{
			//OutputDebugString("RemovePolygonsIncluded()\n");
			RemovePolygonsIncluded(&StartCCWPolygons, &EndCCWPolygons);

			// 폴리곤 밖에 있는경우 안쪽에 적당한 지점을 추천한다.
			// 시작점과 끝점을 그어서 가장 먼저 교차하는 지점은 0.9f 포지션 ( 가장 먼저 교차하는 라인이 항상 폴리곤 안에 있는 지점이다. )
			// 가장 안쪽 폴리곤의 점을 찾아 EndPoint로 지정
			float x, y;
			//if(FindNonIntersectPosition(&StartCCWPolygons, &x, &y, m_pStartPoint->GetX(), m_pStartPoint->GetY(), m_pEndPoint->GetX(), m_pEndPoint->GetY())==true){
			if(FindNonIntersectPosition(&StartCCWPolygons, &x, &y, m_pEndPoint->GetX(), m_pEndPoint->GetY())==true){
				*pRecX = x, *pRecY = y;
				return true;
			}
		}
	}

	// Clockwise 방향의 폴리곤 체크
	CMPtrList<MPMPolygon> StartCWPolygons;
	GetInnerClosedPolygon(&StartCWPolygons, 0, MPMPD_CLOCKWISE);
	CMPtrList<MPMPolygon> EndCWPolygons;
	GetInnerClosedPolygon(&EndCWPolygons, 1, MPMPD_CLOCKWISE);
	if(IsPolygonsIncluded(&EndCWPolygons, &StartCWPolygons)==true) return false;	// 같은 폴리곤안에 속해 있으면 그냥 서치하면 된다.
	else{
		// 폴리곤 밖에 있는경우 안쪽에 적당한 지점을 추천한다.
		// 시작점과 끝점을 그어서 가장 먼저 교차하는 지점은 0.9f 포지션 ( 가장 먼저 교차하는 라인이 항상 폴리곤 안에 있는 지점이다. )
		// 가장 안쪽 폴리곤의 점을 찾아 EndPoint로 지정
		RemovePolygonsIncluded(&EndCWPolygons, &StartCWPolygons);
		float x, y;
		//if(FindNonIntersectPosition(&EndCWPolygons, &x, &y, m_pStartPoint->GetX(), m_pStartPoint->GetY(), m_pEndPoint->GetX(), m_pEndPoint->GetY())==true){
		if(FindNonIntersectPosition(&EndCWPolygons, &x, &y, m_pEndPoint->GetX(), m_pEndPoint->GetY())==true){
			*pRecX = x, *pRecY = y;
			return true;
		}
	}

	return false;
}

MPMRESULT MPolygonMapModel::PreparePathFinding(float* pRecX, float* pRecY)
{
	float sx = m_pStartPoint->GetX();
	float sy = m_pStartPoint->GetY();

	// 시작점이 닫혀있는 폴리곤 안에 속해 있는 경우, 밖으로 빼준다.
	MPMPolygon* pInnerPolygon = GetNearestInnerPolygon(&m_PolygonList, sx, sy);
	if(pInnerPolygon!=NULL){
		if(pInnerPolygon->GetDir()==MPMPD_CLOCKWISE){
			float nx, ny;
			pInnerPolygon->GetNearestContactPoint(&nx, &ny, sx, sy, false);
#define NEW_POSITION_CONSTANT	1.01f	// 약간 벗어난 지점을 지정해 준다.
			SetStartPoint(sx+(nx-sx)*NEW_POSITION_CONSTANT, sy+(ny-sy)*NEW_POSITION_CONSTANT, 0, NULL);
			_ASSERT(FALSE);	// MPolygonMapModel은 현재까지 추천기능을 지원하지 않는다.
		}
	}

	// Test하는 이유는 서치 타임을 줄일 수 있게 하기 위해서이다.
	// 미리 폴리곤안에 점이 속해 있는지 테스트, 플래그 하고,
	TestStartPointInPolygon();
	TestEndPointInPolygon();

	// 적절한 End Point를 추천, 다시 폴리곤 안에 점이 속해 있는지 테스트, 플래그 달기
	if(RecommendEndPoint(pRecX, pRecY)==true){
		SetEndPoint(*pRecX, *pRecY, 0, NULL);
		_ASSERT(FALSE);	// MPolygonMapModel은 현재까지 추천기능을 지원하지 않는다.
		TestEndPointInPolygon();	// 다시 테스트하지 않고, RecommendEndPoint()안에서 처리할 수 있다.
		return MPMR_ENDPOINTCHANGED;
	}

	return MPMR_OK;
}

void MPolygonMapModel::SetPolygonPathList(int* pPolygonList, int nCount)
{
	if(nCount==0) return;
	if(m_pPolygonPathList!=NULL) delete[] m_pPolygonPathList;
	m_pPolygonPathList = new int[nCount];
	memcpy(m_pPolygonPathList, pPolygonList, sizeof(int)*nCount);
	m_nPolygonPathCount = nCount;
}

bool MPolygonMapModel::IsPolygonPathInclude(int nRoomIndexTest, int nRoomIndexBegin, int nRoomIndexEnd)
{
	for(int i=0; i<m_nPolygonPathCount; i++){
		if(m_pPolygonPathList[i]==nRoomIndexBegin){
			for(int j=i; j<m_nPolygonPathCount; j++){
				if(m_pPolygonPathList[j]==nRoomIndexTest) return true;
				if(m_pPolygonPathList[j]==nRoomIndexEnd) return false;
			}
			break;
		}
	}
	return false;
}

bool IsPolygonShared(list<int>* pl1, list<int>* pl2)
{
	for(list<int>::iterator i=pl1->begin(); i!=pl1->end(); i++){
		for(list<int>::iterator j=pl2->begin(); j!=pl2->end(); j++){
			if(*i==*j) return true;
		}
	}
	return false;
}

bool IsPolygonShared(int p, list<int>* pl)
{
	for(list<int>::iterator i=pl->begin(); i!=pl->end(); i++){
		if(*i==p) return true;
	}
	return false;
}

bool MPolygonMapModel::IsPolygonPathInclude(list<int>* pRoomIndexTest, list<int>* pRoomIndexBegin, list<int>* pRoomIndexEnd)
{
	// Find Range
	int nMin = INT_MAX;
	int nMax = 0;
	for(list<int>::iterator i=pRoomIndexBegin->begin(); i!=pRoomIndexBegin->end(); i++){
		int nValue = *i;
		if(nValue<nMin) nMin = nValue;
		if(nValue>nMax) nMax = nValue;
	}
	for(i=pRoomIndexEnd->begin(); i!=pRoomIndexEnd->end(); i++){
		int nValue = *i;
		if(nValue<nMin) nMin = nValue;
		if(nValue>nMax) nMax = nValue;
	}

	for(i=pRoomIndexTest->begin(); i!=pRoomIndexTest->end(); i++){
		int nValue = *i;
		if(nValue<nMin || nValue>nMax) return false;
	}

	return true;
}

void MPolygonMapModel::SetPolygonSharedMap(MPOLYGONIDPAIRMAP* pSharedMap)
{
	m_PolygonSharedMap.clear();
	m_PolygonSharedMap.insert(pSharedMap->begin(), pSharedMap->end());
}


/*
bool MPolygonMapModel::RecommendEndPoint(void)
{
	// 미리 Start, End Point가 폴리곤에 속해 있는지 테스트 해본다.
	TestStartPointInPolygon();
	TestEndPointInPolygon();

	CMPtrList<MPMPolygon> StartCCWPolygons;
	GetInnerClosedPolygon(&StartCCWPolygons, 0, MPMPD_COUNTERCLOCKWISE);
	if(StartCCWPolygons.GetCount()>0){
		CMPtrList<MPMPolygon> EndCCWPolygons;
		GetInnerClosedPolygon(&StartCCWPolygons, 1, MPMPD_COUNTERCLOCKWISE);
		if(IsPolygonsIncluded(&StartCCWPolygons, &EndCCWPolygons)==true) return false;	// 같은 폴리곤안에 속해 있으면 그냥 서치하면 된다.
		else{
			// 폴리곤 밖에 있는경우 안쪽에 적당한 지점을 추천한다.
			// 시작점과 끝점을 그어서 가장 먼저 교차하는 지점은 0.9f 포지션 ( 가장 먼저 교차하는 라인이 항상 폴리곤 안에 있는 지점이다. )
			// 가장 안쪽 폴리곤의 점을 찾아 EndPoint로 지정
			for(int i=0; i<StartCCWPolygons.GetCount(); i++){
				MPMPolygon* pPolygon = StartCCWPolygons.Get(i);
				MPMPoint* p1;
				MPMPoint* p2;
				float t;
				if(pPolygon->IsIntersect(&p1, &p2, &t, m_pStartPoint->GetX(), m_pStartPoint->GetY(), m_pEndPoint->GetX(), m_pEndPoint->GetY())==true){
					float dx = m_pEndPoint->GetX() - m_pStartPoint->GetX();
					float dy = m_pEndPoint->GetY() - m_pStartPoint->GetY();
#define INNER_POSITION_CONSTANT	0.95f
					dx*=(t*INNER_POSITION_CONSTANT);
					dy*=(t*INNER_POSITION_CONSTANT);

					float x, y;
					x = m_pStartPoint->GetX()+dx;
					y = m_pStartPoint->GetY()+dy;
					SetEndPoint(x, y);
					return true;
				}
			}
		}
	}
	else{
		// 목적지점이 Clockwise폴리곤 안에 갇혀 있는 경우, 적당한 외부점으로 빼주기
		float rx, ry;
		if(RecommendPoint(&rx, &ry, m_pEndPoint->GetX(), m_pEndPoint->GetY(), fCheckStepDistance, fExponentialIncrease)==true){
			SetEndPoint(rx, ry);
			return true;
		}
	}

	return false;
}
*/
void MPolygonMapModel::ClearVisibledList(void)
{
	for(int i=0; i<m_PointList.GetCount(); i++){
		MPMPoint* p = m_PointList.Get(i);
		p->m_VisibledList.DeleteAll();
	}
}

MPMPolygon* MPolygonMapModel::IsInnerClosedPolygon(float x, float y, bool bTestClockwise)
{
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPMPolygon* pPolygon = m_PolygonList.Get(i);
		if(pPolygon->IsInnerPoint(x, y, bTestClockwise)==true) return pPolygon;
	}
	return NULL;
}

void MPolygonMapModel::IsInnerClosedPolygon(CMPtrList<MPMPolygon>* pPolygons, float x, float y, bool bTestClockwise)
{
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPMPolygon* pPolygon = m_PolygonList.Get(i);
		if(pPolygon->IsInnerPoint(x, y, bTestClockwise)==true){
			pPolygons->Add(pPolygon);
		}
	}
}

bool MPolygonMapModel::IsEndPointInPolygon(void)
{
	return m_bEndPointInPolygon;
}

MPMPolygon* MPolygonMapModel::GetNearestInnerPolygon(CMLinkedList<MPMPolygon>* pPolygons, float x, float y)
{
	float tNear = FLT_MAX;
	MPMPolygon* pNearestPolygon = NULL;
	for(int i=0; i<pPolygons->GetCount(); i++){
		MPMPolygon* pPolygon = pPolygons->Get(i);

		float t;
		MPMPOLYGONDIR d = pPolygon->IsInnerPoint(&t, x, y);

		if(d!=MPMPD_NA){
			if(t<tNear){
				tNear = t;
				pNearestPolygon = pPolygon;
			}
		}
	}

	return pNearestPolygon;
}

bool MPolygonMapModel::IsEnable(CMLinkedList<MPMPolygon>* pPolygons, float x, float y)
{
	MPMPolygon* pPolygon = GetNearestInnerPolygon(pPolygons, x, y);
	if(pPolygon==NULL) return true;
	if(pPolygon->GetDir()==MPMPD_CLOCKWISE) return false;

	return true;
}

bool MPolygonMapModel::IsEnable(float x, float y)
{
	return IsEnable(&m_PolygonList, x, y);
}

void MPolygonMapModel::Add(MPolygonMapModel* pNGMM)
{
	// 폴리곤과 포인트 추가
	for(int i=0; i<pNGMM->m_PolygonList.GetCount(); i++){
		MPMPolygon* pPO = pNGMM->m_PolygonList.Get(i);
		int nPointCount = pPO->m_PointList.GetCount();
		for(int p=0; p<nPointCount; p++){
			MPMPoint* pPN = pPO->m_PointList.Get(p);
			MPMPoint* pThisPN = AddPoint(pPN->GetX(), pPN->GetY(), pPN->GetZ(), pPN->m_RoomIndexs);
			pThisPN->m_pTemp = pPN;	// 임시로 원래 포인터를 대입한다.
			pPN->m_pTemp = pThisPN;	// 임시로 추가되는 포인터를 대입한다.
			if(p==nPointCount-1){
				if(pPO->m_bEnclosed==true)
					AddPolygon(pThisPN, MPMPAT_ENCLOSE);
				else
					AddPolygon(pThisPN, MPMPAT_END);
			}
			else
				AddPolygon(pThisPN);
		}
	}

	// Visible
	/*
	for(int p=0; p<m_PointList.GetCount(); p++){
		MPMPoint* pPN = m_PointList.Get(p);
		MPMPoint* pOrigPN = (MPMPoint *)pPN->m_pTemp;
		for(int v=0; v<pOrigPN->GetVisiblePointCount(); v++){
			MPMPoint* pVP = pOrigPN->GetVisiblePoint(v);
			pVP->AddVisiblePoint((MPMPoint *)pVP->m_pTemp);
		}
	}
	*/

	Change();
}

void MPolygonMapModel::MergeBreakPolygon(void)
{
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPMPolygon* pPO = m_PolygonList.Get(i);
		MPMPoint* pPN = pPO->GetEndPoint();

		if(pPO->m_bEnclosed==true) continue;

		for(int j=0; j<m_PolygonList.GetCount(); j++){
			MPMPolygon* pDPO = m_PolygonList.Get(j);
			if(pPO==pDPO) continue;

			if(pDPO->m_bEnclosed==false){
				MPMPoint* pDPN = pDPO->GetStartPoint();
				if(pPN->GetX()==pDPN->GetX() && pPN->GetY()==pDPN->GetY()){	// pPO의 끝점과 pDPO의 시작점 같은 경우 합친다.
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
		MPMPolygon* pPO = m_PolygonList.Get(i);
		if(pPO->m_bEnclosed==true) continue;

		MPMPoint* pSPN = pPO->GetStartPoint();
		MPMPoint* pEPN = pPO->GetEndPoint();
		if(pSPN->GetX()==pEPN->GetX() && pSPN->GetY()==pEPN->GetY()){
			pPO->m_PointList.Delete(pPO->m_PointList.GetCount()-1);
			pPO->m_bEnclosed = true;
		}	
	}

	// Rebuild Connection
	for(i=0; i<m_PointList.GetCount(); i++){
		MPMPoint* p1 = m_PointList.Get(i);
		p1->ResetConnectPoint();
	}
	for(i=0; i<m_PolygonList.GetCount(); i++){
		MPMPolygon* pPO = m_PolygonList.Get(i);
		pPO->RebuildConnection();
	}

	Change();
}

void MPolygonMapModel::Enlarge(float fMargin)
{
	for(int i=0; i<m_PortalList.GetCount(); i++){
		MPMPortal* pPO = m_PortalList.Get(i);
		pPO->Enlarge(fMargin);
	}
	for(int i=0; i<m_PolygonList.GetCount(); i++){
		MPMPolygon* pPO = m_PolygonList.Get(i);
		pPO->Enlarge(fMargin);
	}

	Change();
}


void MPolygonMapModel::Change(void)
{
	m_nChangeCount++;
}

int MPolygonMapModel::GetChangeCount(void)
{
	return m_nChangeCount;
}
