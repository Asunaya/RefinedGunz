#include "MRayCastingDriver.h"
#include "MUtil.h"

float g_fOutPosConstant = 1.1f;

void SetOutPosConstant(float fConstant)
{
	g_fOutPosConstant = fConstant;
}

MObstacle::MObstacle(rvector& Pos)
{
	m_Pos = Pos;
	m_nTempUsed = 0;
}

MObstacle::~MObstacle(void)
{
}

bool MObstacle::IsCollision(rvector* pNewPos, rvector& Pos)
{
	if(IsCollision(Pos)==true){
		*pNewPos = GetOutPos(Pos);
		return true;
	}
	return false;
}

bool MObstacle::IsOverlap(MObstacle* pObstacle)
{
	float fDiff = GetDistance(pObstacle);
	if(fDiff<=0) return true;
	return false;
}

rvector MObstacle::GetCenter(void)
{
	return m_Pos;
}

/*
void MObstacle::GetOverlappedEdge(rvector* pLeft, rvector* pRight, rvector& Pos)
{
	rvector Left, Right;
	GetEdge(&Left, &Right, Pos);
	rvector ToLeft = Normalize(Left - Pos);
	rvector ToRight = Normalize(Right - Pos);
	MObstacle* pLeftEdge = this;
	MObstacle* pRightEdge = this;
	for(int i=0; i<m_OverlappedObstacles.GetCount(); i++){
		MObstacle* pObstacle = m_OverlappedObstacles.Get(i);
		rvector OLeft, ORight;
		pObstacle->GetEdge(&OLeft, &ORight, Pos);
		rvector OToLeft = Normalize(OLeft - Pos);
		rvector OToRight = Normalize(ORight - Pos);

		rvector L = CrossProduct(ToLeft, OToLeft);
		if(L.z<0){
			Left = OLeft;
			ToLeft = OToLeft;
			pLeftEdge = pObstacle;
		}

		rvector R = CrossProduct(ToRight, OToRight);
		if(R.z>0){
			Right = ORight;
			ToRight = OToRight;
			pRightEdge = pObstacle;
		}
	}


	if(pLeftEdge!=this){
		pLeftEdge->GetOverlappedLeftEdge(&Left, Pos);
	}

	if(pRightEdge!=this){
		pRightEdge->GetOverlappedLeftEdge(&Right, Pos);
	}

	*pLeft = Left;
	*pRight = Right;

	return true;
}
*/

void MObstacle::CheckOverlappedObstacleTemp(int nTemp)
{
	if(m_nTempUsed==nTemp) return;

	m_nTempUsed = nTemp;
	for(int i=0; i<m_OverlappedObstacles.GetCount(); i++){
		MObstacle* pObs = m_OverlappedObstacles.Get(i);
		pObs->CheckOverlappedObstacleTemp(nTemp);
	}
}

void MObstacle::GetOverlappedEdge(rvector* pLeft, rvector* pRight, rvector& Pos, CMPtrList<MObstacle>* pLeftObsList, CMPtrList<MObstacle>* pRightObsList)
{
#define MN_OVERLAPPEDEDGE		10
	CheckOverlappedObstacleTemp(0);	// 무한 반복을 막기 위해
	GetOverlappedLeftEdge(pLeft, Pos, this, pLeftObsList);
	GetOverlappedRightEdge(pRight, Pos, this, pRightObsList);
}

MObstacle* MObstacle::GetOverlappedLeftEdge(rvector* pLeft, rvector& Pos, MObstacle* pAncestor, CMPtrList<MObstacle>* pObsList)
{
	rvector Left, Right;
	GetEdge(&Left, &Right, Pos);
	rvector ToLeft = Normalize(Left - Pos);
	MObstacle* pLeftEdge = this;

	for(int i=0; i<m_OverlappedObstacles.GetCount(); i++){
		MObstacle* pObstacle = m_OverlappedObstacles.Get(i);
		if(pObstacle==pAncestor) continue;	// 이미 검색한것이므로..
		if(pObstacle->m_nTempUsed==MN_OVERLAPPEDEDGE) continue;
		rvector OLeft, ORight;
		pObstacle->GetEdge(&OLeft, &ORight, Pos);
		rvector OToLeft = Normalize(OLeft - Pos);

		rvector L = CrossProduct(ToLeft, OToLeft);
		if(L.z<0){
			Left = OLeft;
			ToLeft = OToLeft;
			pLeftEdge = pObstacle;
		}
	}

	if(pObsList!=NULL) pObsList->Add(pLeftEdge);

	if(pLeftEdge!=this){
		pLeftEdge->m_nTempUsed = MN_OVERLAPPEDEDGE;
		pLeftEdge = pLeftEdge->GetOverlappedLeftEdge(&Left, Pos, this, pObsList);
	}

	*pLeft = Left;
	return pLeftEdge;
}

MObstacle* MObstacle::GetOverlappedRightEdge(rvector* pRight, rvector& Pos, MObstacle* pAncestor, CMPtrList<MObstacle>* pObsList)
{
	rvector Left, Right;
	GetEdge(&Left, &Right, Pos);
	rvector ToRight = Normalize(Right - Pos);
	MObstacle* pRightEdge = this;

	for(int i=0; i<m_OverlappedObstacles.GetCount(); i++){
		MObstacle* pObstacle = m_OverlappedObstacles.Get(i);
		if(pObstacle==pAncestor) continue;	// 이미 검색한것이므로..
		if(pObstacle->m_nTempUsed==MN_OVERLAPPEDEDGE) continue;
		rvector OLeft, ORight;
		pObstacle->GetEdge(&OLeft, &ORight, Pos);
		rvector OToRight = Normalize(ORight - Pos);

		rvector R = CrossProduct(ToRight, OToRight);
		if(R.z>0){
			Right = ORight;
			ToRight = OToRight;
			pRightEdge = pObstacle;
		}
	}

	if(pObsList!=NULL) pObsList->Add(pRightEdge);

	if(pRightEdge!=this){
		pRightEdge->m_nTempUsed = MN_OVERLAPPEDEDGE;
		pRightEdge = pRightEdge->GetOverlappedRightEdge(&Right, Pos, this, pObsList);
	}

	*pRight = Right;
	return pRightEdge;
}



// 2D Circle

MCircleObstacle::MCircleObstacle(rvector& Pos, float fRadius)
: MObstacle(Pos)
{
	//m_Pos = Pos;
	m_fRadius = fRadius;
}

rvector MCircleObstacle::GetOutPos(rvector& Pos)
{
	rvector Diff = Pos - m_Pos;
	Diff.z = 0;	// 2D
	float fDiff = Diff.GetMagnitude();
	if(fDiff==0.0f) return (m_Pos + rvector(1, 0, 0) * m_fRadius * g_fOutPosConstant );
	else return (Normalize(Diff) + Pos);
}

bool MCircleObstacle::IsCollision(rvector& Pos)
{
	rvector Diff = Pos - m_Pos;
	Diff.z = 0;	// 2D
	float fDiff = Diff.GetMagnitude();
	if(fDiff<m_fRadius) return true;
	return false;
}

bool MCircleObstacle::IsCollision(rvector* pNewPos, rvector& Pos)
{
	rvector Diff = Pos - m_Pos;
	Diff.z = 0;	// 2D
	float fDiff = Diff.GetMagnitude();
	if(fDiff<m_fRadius){
		if(pNewPos!=NULL){
			if(fDiff==0.0f) *pNewPos = m_Pos + rvector(1, 0, 0) * m_fRadius * g_fOutPosConstant;
			else *pNewPos = m_Pos + Diff*(1.0f/fDiff) * m_fRadius * g_fOutPosConstant;
		}
		return true;
	}
	return false;
}

bool MCircleObstacle::IsIntersect(float* t, rvector& Pos, rvector& Dir, float fMagnitude)
{
	return MIsIntersectLineAndCircle(t, Pos, Dir*fMagnitude, m_Pos, m_fRadius);
}

void MCircleObstacle::GetEdge(rvector* pLeft, rvector* pRight, rvector& Pos)
{
	rvector ContactPoint[2];
	MGetContactPoint(ContactPoint, Pos, m_Pos, m_fRadius);
	*pLeft = ContactPoint[0];
	*pRight = ContactPoint[1];
}

bool MCircleObstacle::GetDiffFromSurface(rvector* pDiff, rvector& p)
{
	rvector Pos(m_Pos.x, m_Pos.y, p.z);
	rvector Diff = p - Pos;
	rvector Dir = Normalize(Diff);
	rvector ToSurface = Dir * m_fRadius;
	*pDiff = Diff - ToSurface;
	if(Diff.GetMagnitude()>m_fRadius) return true;		// p가 외부에 있으면 true 리턴
	else return false;
}

float MGetMagnitude(rvector& v)
{
	if(v.x==0.0f && v.y==0.0f && v.z==0.0f) return 0.0f;
	return v.GetMagnitude();
}

float MCircleObstacle::GetDistance(MObstacle* pObstacle)
{
	rvector Diff;
	bool bOut = pObstacle->GetDiffFromSurface(&Diff, m_Pos);
	Diff.z = 0;
	float fDiff = MGetMagnitude(Diff) * ((bOut==true)?1:-1) - m_fRadius;
	return fDiff;
}

float MCircleObstacle::GetArea(void)
{
#define square(_r)	(_r*_r)
	return 2*pi*square(m_fRadius);
}

void MCircleObstacle::EnLarge(float fAdd)
{
	m_fRadius += fAdd;
}


// Rectangle Obstacle

int MRectangleObstacle::GetDivision(rvector& Pos)
{
	float x = Pos.x - m_Pos.x;
	float y = Pos.y - m_Pos.y;
	float w2 = m_fWidth / 2.0f;
	float h2 = m_fHeight / 2.0f;

	if(y<=-h2){
		if(x>=w2) return 1;
		else if(x>-w2) return 2;
		else return 3;
	}
	else if(y<h2){
		if(x>=0) return 8;
		else return 4;
	}
	else{
		if(x<=-w2) return 5;
		else if(x<w2) return 6;
		else return 7;
	}
}

MRectangleObstacle::MRectangleObstacle(rvector& Pos, float w, float h)
: MObstacle(Pos)
{
	m_fWidth = w;
	m_fHeight = h;
}

MRectangleObstacle::MRectangleObstacle(float x1, float y1, float x2, float y2)
: MObstacle(rvector((x1+x2)/2.0f, (y1+y2)/2.0f, 0.0f))
{
	m_fWidth = x2 - x1;
	m_fHeight = y2 - y1;
}

rvector MRectangleObstacle::GetOutPos(rvector& Pos)
{
	float w2 = (m_fWidth / 2.0f)  * g_fOutPosConstant;
	float h2 = (m_fHeight / 2.0f) * g_fOutPosConstant;
	float x = Pos.x - m_Pos.x;
	float y = Pos.y - m_Pos.y;
	float dx = w2 - (float)fabs(x);
	float dy = h2 - (float)fabs(y);

	if(dx<dy){
		if(x<0) return rvector(m_Pos.x - w2, Pos.y, 0);
		else return rvector(m_Pos.x + w2, Pos.y, 0);
	}
	else{
		if(y<0) return rvector(Pos.x, m_Pos.y - h2, 0);
		else return rvector(Pos.x, m_Pos.y + h2, 0);
	}
}

bool MRectangleObstacle::IsCollision(rvector& Pos)
{
	float w2 = m_fWidth / 2.0f;
	float h2 = m_fHeight / 2.0f;

	if(Pos.x>m_Pos.x-w2 && Pos.x<m_Pos.x+w2){
		if(Pos.y>m_Pos.y-h2 && Pos.y<m_Pos.y+h2){
			return true;
		}
	}
	return false;
}

bool MRectangleObstacle::IsIntersect(float* t, rvector& Pos, rvector& Dir, float fMagnitude)
{
	float w2 = m_fWidth / 2.0f;
	float h2 = m_fHeight / 2.0f;

	return (MIsIntersectLineAndRectangle(t, Pos, Dir*fMagnitude, m_Pos.x-w2, m_Pos.y-h2, m_Pos.x+w2, m_Pos.y+h2)>=0);
}

void MRectangleObstacle::GetEdge(rvector* pLeft, rvector* pRight, rvector& Pos)
{
	float w2 = m_fWidth / 2.0f;
	float h2 = m_fHeight / 2.0f;

	/*
	    |     |
	  3 |  2  | 1
	----+-----+----
	    |     | 8           +-- x(+)
	  4 |     |             |
	----+-----+----         y(+)
	  5 |  6  | 7
	    |     |
	*/

	pLeft->z = 0;
	pRight->z = 0;

	switch(GetDivision(Pos)){
	case 1:
		pLeft->x = m_Pos.x + w2;
		pLeft->y = m_Pos.y + h2;
		pRight->x = m_Pos.x - w2;
		pRight->y = m_Pos.y - h2;
		break;
	case 2:
		pLeft->x = m_Pos.x + w2;
		pLeft->y = m_Pos.y - h2;
		pRight->x = m_Pos.x - w2;
		pRight->y = m_Pos.y - h2;
		break;
	case 3:
		pLeft->x = m_Pos.x + w2;
		pLeft->y = m_Pos.y - h2;
		pRight->x = m_Pos.x - w2;
		pRight->y = m_Pos.y + h2;
		break;
	case 4:
		pLeft->x = m_Pos.x - w2;
		pLeft->y = m_Pos.y - h2;
		pRight->x = m_Pos.x - w2;
		pRight->y = m_Pos.y + h2;
		break;
	case 5:
		pLeft->x = m_Pos.x - w2;
		pLeft->y = m_Pos.y - h2;
		pRight->x = m_Pos.x + w2;
		pRight->y = m_Pos.y + h2;
		break;
	case 6:
		pLeft->x = m_Pos.x - w2;
		pLeft->y = m_Pos.y + h2;
		pRight->x = m_Pos.x + w2;
		pRight->y = m_Pos.y + h2;
		break;
	case 7:
		pLeft->x = m_Pos.x - w2;
		pLeft->y = m_Pos.y + h2;
		pRight->x = m_Pos.x + w2;
		pRight->y = m_Pos.y - h2;
		break;
	case 8:
		pLeft->x = m_Pos.x + w2;
		pLeft->y = m_Pos.y + h2;
		pRight->x = m_Pos.x + w2;
		pRight->y = m_Pos.y - h2;
		break;
	}
}

bool MRectangleObstacle::GetDiffFromSurface(rvector* pDiff, rvector& p)
{
	float w2 = m_fWidth / 2.0f;
	float h2 = m_fHeight / 2.0f;
	float fRaidus = (float)sqrt(w2*w2 + h2*h2);

	if(IsCollision(p)==true){	// 내부에 있는 경우
		float x = p.x - m_Pos.x;
		float y = p.y - m_Pos.y;
		if(w2-fabs(x)<h2-fabs(y)){	// 가까운 면에서부터의 Diff Vector를 찾는다.
			if(x>=0){
				pDiff->x = p.x - (m_Pos.x + w2);
				pDiff->y = 0;
			}
			else{
				pDiff->x = p.x - (m_Pos.x - w2);
				pDiff->y = 0;
			}
		}
		else{
			if(y>=0){
				pDiff->x = 0;
				pDiff->y = p.y - (m_Pos.y + h2);
			}
			else{
				pDiff->x = 0;
				pDiff->y = p.y - (m_Pos.y - h2);
			}
		}
		pDiff->z = 0;
		return false;
	}

	switch(GetDivision(p)){
	case 1:
		*pDiff = p - rvector(m_Pos.x+w2, m_Pos.y-h2, 0);
		return true;
	case 2:
		*pDiff = rvector(0, p.y - (m_Pos.y-h2), 0);
		return true;
	case 3:
		*pDiff = p - rvector(m_Pos.x-w2, m_Pos.y-h2, 0);
		return true;
	case 4:
		*pDiff = rvector(p.x - (m_Pos.x-w2), 0, 0);
		return true;
	case 5:
		*pDiff = p - rvector(m_Pos.x-w2, m_Pos.y+h2, 0);
		return true;
	case 6:
		*pDiff = rvector(0, p.y - (m_Pos.y+h2), 0);
		return true;
	case 7:
		*pDiff = p - rvector(m_Pos.x+w2, m_Pos.y+h2, 0);
		return true;
	case 8:
		*pDiff = rvector(p.x - (m_Pos.x+w2), 0, 0);
		return true;
	}

	return false;
}

float MRectangleObstacle::GetDistance(MObstacle* pObstacle)
{
	rvector Diff;
	bool bOut = pObstacle->GetDiffFromSurface(&Diff, m_Pos);
	Diff.z = 0;
	rvector p = m_Pos - Diff;
	bOut = GetDiffFromSurface(&Diff, p);
	float fDiff = Diff.GetMagnitude() * ((bOut==true)?1:-1);
	return fDiff;
}


float MRectangleObstacle::GetArea(void)
{
	return m_fWidth * m_fHeight;
}

void MRectangleObstacle::EnLarge(float fAdd)
{
	m_fWidth += (fAdd*2);
	m_fHeight += (fAdd*2);
}

MReverseRectangleObstacle::MReverseRectangleObstacle(rvector& p, float w, float h)
{
	m_Pos = p;
	m_fWidth = w;
	m_fHeight = h;
}


// MRayCastingDriver

void MRayCastingDriver::CheckOverlappedObstacleTemp(MObstacle* p, int nTemp)
{
	if(p->m_nTempUsed==nTemp) return;

	p->m_nTempUsed = nTemp;
	for(int i=0; i<p->m_OverlappedObstacles.GetCount(); i++){
		MObstacle* pObs = p->m_OverlappedObstacles.Get(i);
		CheckOverlappedObstacleTemp(pObs, nTemp);
	}
}

void MRayCastingDriver::GetArea(float* pLeftArea, float* pRightArea, MObstacle* pColObstacle, rvector& Pos, rvector& Target)
{
	rvector Dir(Target - Pos);

	for(int i=0; i<m_Obstacles.GetCount(); i++){
		MObstacle* pObs = m_Obstacles.Get(i);
		pObs->m_nTempUsed = 0;
	}
	CheckOverlappedObstacleTemp(pColObstacle, 1);

	float fLeftArea = 0;
	float fRightArea = 0;
	for(i=0; i<m_Obstacles.GetCount(); i++){
		MObstacle* pObs = m_Obstacles.Get(i);
		if(pObs->m_nTempUsed==1){
			rvector ObsDir(pObs->m_Pos-Pos);
			rvector z = CrossProduct(Dir, ObsDir);
			if(z.z<0){	// Left
				fLeftArea += pObs->GetArea();
			}
			else{
				fRightArea += pObs->GetArea();
			}
		}
	}

	*pLeftArea = fLeftArea;
	*pRightArea = fRightArea;
}

void MRayCastingDriver::GetArea(float* pLeftArea, float* pRightArea, MObstacle* pColObstacle, MObstacle* pAncestor, bool bAncestorLeft, rvector& Pos, rvector& Target, int nMagicNumber)
{
	if(pColObstacle->m_nTempUsed==nMagicNumber) return;	// 두번 반복하지 않기 위해서. 단, MagicNumber는 고유한 수여야 한다.
	pColObstacle->m_nTempUsed = nMagicNumber;

	rvector Dir = Target - Pos;

	float fLeftArea = 0;
	float fRightArea = 0;

	if(pAncestor==NULL){	// 처음 불리는 경우
		*pLeftArea = 0;
		*pRightArea = 0;

		rvector ObsDir(pColObstacle->m_Pos-Pos);
		rvector z = CrossProduct(Dir, ObsDir);
		if(z.z<0)	// Left
			fLeftArea = pColObstacle->GetArea();
		else
			fRightArea = pColObstacle->GetArea();
	}
	else{
		rvector AncDir(pAncestor->m_Pos-Pos);
		rvector ObsDir(pColObstacle->m_Pos-Pos);
		rvector z = CrossProduct(AncDir, ObsDir);

		if(bAncestorLeft==true){	// 이전 방향이 Left
			if(z.z<0){	// 이전 방향보다 Left
				fLeftArea = pColObstacle->GetArea();
			}
			else{	// 이전 방향보다 Right
				rvector z = CrossProduct(Dir, AncDir);
				if(z.z<0){	// 이전 방향이 Left 영역에 있는 경우
					rvector z = CrossProduct(Dir, ObsDir);
					if(z.z>0){
						fRightArea = pColObstacle->GetArea();
					}
					else{
						fLeftArea = pColObstacle->GetArea();
					}
				}
				else{
					fLeftArea = pColObstacle->GetArea();
				}
			}
		}
		else{						// 이전의 방향이 Right
			if(z.z>0){	// 이전 방향보다 Right
				fRightArea = pColObstacle->GetArea();
			}
			else{	// 이전 방향보다 Left
				rvector z = CrossProduct(Dir, AncDir);
				if(z.z>0){	// 이전 방향이 Right 영역에 있는 경우
					rvector z = CrossProduct(Dir, ObsDir);
					if(z.z<0){
						fLeftArea = pColObstacle->GetArea();
					}
					else{
						fRightArea = pColObstacle->GetArea();
					}
				}
				else{
					fRightArea = pColObstacle->GetArea();
				}
			}
		}
	}

	*pLeftArea += fLeftArea;
	*pRightArea += fRightArea;

	for(int i=0; i<pColObstacle->m_OverlappedObstacles.GetCount(); i++){
		MObstacle* pObs = pColObstacle->m_OverlappedObstacles.Get(i);
		GetArea(pLeftArea, pRightArea, pObs, pColObstacle, (fLeftArea>0), Pos, Target, nMagicNumber);
	}
}

MRayCastingDriver::MRayCastingDriver(void)
{
	m_pRRObstacle = NULL;
}

MRayCastingDriver::~MRayCastingDriver(void)
{
}

void MRayCastingDriver::AddObstacle(MObstacle* pObstacle)
{
	pObstacle->m_OverlappedObstacles.DeleteAll();

	for(int i=0; i<m_Obstacles.GetCount(); i++){
		MObstacle* pOtherObs = m_Obstacles.Get(i);
		if(pObstacle->IsOverlap(pOtherObs)==true){
			pObstacle->m_OverlappedObstacles.Add(pOtherObs);
			pOtherObs->m_OverlappedObstacles.Add(pObstacle);
		}
	}

	m_Obstacles.Add(pObstacle);
}

void MRayCastingDriver::AddObstacle(CMPtrList<MObstacle>* pOL)
{
	for(int i=0; i<pOL->GetCount(); i++){
		MObstacle* pObstacle = pOL->Get(i);
		AddObstacle(pObstacle);
	}
}

void MRayCastingDriver::AddObstacle(CMLinkedList<MObstacle>* pOL)
{
	for(int i=0; i<pOL->GetCount(); i++){
		MObstacle* pObstacle = pOL->Get(i);
		AddObstacle(pObstacle);
	}
}

void MRayCastingDriver::DeleteObstacle(void)
{
	m_Obstacles.DeleteAll();
}

bool MRayCastingDriver::IsCollision(rvector* pNewPos, rvector& Pos)
{
	bool bCollision = false;
	rvector NewDir(0, 0, 0);

	for(int i=0; i<m_Obstacles.GetCount(); i++){
		MObstacle* pObstacle = m_Obstacles.Get(i);
		rvector RefPos;
		if(pObstacle->IsCollision(&RefPos, Pos)==true){
			NewDir += (RefPos - Pos);
			bCollision = true;
		}
	}

	if(bCollision==true){
		*pNewPos = Pos + NewDir;
		return true;
	}

	return false;
}

void MRayCastingDriver::SetReverseRectangleObstacle(rvector& p, float w, float h)
{
	if(m_pRRObstacle!=NULL) delete m_pRRObstacle;
	m_pRRObstacle = new MReverseRectangleObstacle(p, w, h);
}

void MRayCastingDriver::RemoveReverseRectangleObstacle(void)
{
	if(m_pRRObstacle!=NULL){
		delete m_pRRObstacle;
		m_pRRObstacle = NULL;
	}
}

float GetMagnitude(rvector& v)
{
#define square(_x)	(_x*_x)
	float l = square(v.x) + square(v.y) + square(v.z);
	if(l==0.0f) return 0.0f;
	return sqrtf(l);
}

rvector MRayCastingDriver::GetNextDir(rvector Pos, rvector Dir, float fMagnitude, rvector CurDir)
{
	rvector Target = Pos + Dir * fMagnitude;

	rvector NextPath;
	if(IsCollision(&NextPath, Pos)==true){
		rvector NewDiff = NextPath - Pos;
		float fMag = GetMagnitude(NewDiff);
		if(fMag!=0.0f) return (NewDiff*(1/fMag));	// 거리가 0.0f인 경우, 충돌이라고 간주하지 않고 패스를 찾는다.
	}

	CMLinkedList<rvector> NextPaths;
	// 갈 방향으로 그어 장애물이 있으면, Edge 부분으로 타겟을 재설정한 다음 이를 반복
#define FIND_DEPTH	4
#define MN_IDENTIFYCOLLISION	20
	MObstacle* pTurnObstacle = NULL;
	for(int i=0; i<FIND_DEPTH; i++){
		NextPaths.Add(new rvector(Pos+Dir*fMagnitude));
		float t = 0;
		MObstacle* pColObstacle = NULL;
		for(int j=0; j<m_Obstacles.GetCount(); j++){
			MObstacle* pObstacle = m_Obstacles.Get(j);
			if(pObstacle==pTurnObstacle) continue;
			//if(pObstacle->IsIntersect(&t, Pos, Dir, fMagnitude)==true && (t>0.0f && t<0.99f)){
			if(pObstacle->IsIntersect(&t, Pos, Dir, fMagnitude)==true && (t>=0.0f && t<=1.0f)){
				fMagnitude *= t;
				pColObstacle = pObstacle;
			}
		}

		if(pColObstacle==NULL){
			break;
		}
		else{
			// Target이 안에 속해 있으면 좌우 판단하지 않고 그 방향으로 전진
			if(pColObstacle->IsCollision(Target)==true){
				break;
			}

			/*
			if(pColObstacle->m_nTempUsed==MN_IDENTIFYCOLLISION){
				float fLeftLength = (Target - Left).GetMagnitude();
				float fRightLength = (Target - Right).GetMagnitude();
				if(fLeftLength<=fRightLength) Dir = (Left - Pos);	// 짧은쪽으로 간다.
				else Dir = (Right - Pos);
				break;
			}
			*/

			rvector Left, Right;
			CMPtrList<MObstacle> LeftObsList;
			CMPtrList<MObstacle> RightObsList;
			pColObstacle->GetOverlappedEdge(&Left, &Right, Pos, &LeftObsList, &RightObsList);
			// 목표방향에 걸리는 오브젝트의 좌우 분포에 따라 방향 선택 ( 정확도는 떨어지지만, 보편적이다. )
			float fLeft = 0;
			float fRight = 0;
			GetArea(&fLeft, &fRight, pColObstacle, NULL, true, Pos, Target, (int)timeGetTime());
			//GetArea(&fLeft, &fRight, pColObstacle, Pos, Target);
			/*
			rvector pp;
			if(LeftObsList.GetCount()>0){
				MObstacle* pObstacle = LeftObsList.Get(0);
				rvector pp = pObstacle->GetCenter();
				for(int k=1; k<LeftObsList.GetCount(); k++){
					MObstacle* pObstacle = LeftObsList.Get(k);
					rvector p = pObstacle->GetCenter();
					fLeft += GetMagnitude(p - pp);
					pp = p;
				}
			}
			if(RightObsList.GetCount()>0){
				MObstacle* pObstacle = RightObsList.Get(0);
				rvector pp = pObstacle->GetCenter();
				for(int k=1; k<RightObsList.GetCount(); k++){
					MObstacle* pObstacle = RightObsList.Get(k);
					rvector p = pObstacle->GetCenter();
					fRight += GetMagnitude(p - pp);
					pp = p;
				}
			}
			*/
			if(fLeft<=fRight){
				Dir = (Left - Pos);	// 짧은쪽으로 간다.
				pTurnObstacle = LeftObsList.Get(LeftObsList.GetCount()-1);
			}
			else{
				Dir = (Right - Pos);
				pTurnObstacle = RightObsList.Get(RightObsList.GetCount()-1);
			}

			// 무조건 Right 로... -_-;;
			/*
			Dir = (Right - Pos);
			pTurnObstacle = RightObsList.Get(RightObsList.GetCount()-1);
			*/
			/*
			float lz = GetAngleOfVectors(Normalize(CurDir), Normalize(Left-Pos));
			float rz = GetAngleOfVectors(Normalize(CurDir), Normalize(Right-Pos));
			float f = GetAngleOfVectors(Normalize(Left-Pos), Normalize(Right-Pos));
			if(fabs(f)>pi*0.8){
			//if(fabs(lz-rz)>pi*0.9){
				if(fabs(lz)<fabs(rz)){
					Dir = (Left - Pos);	// 짧은쪽으로 간다.
					pTurnObstacle = LeftObsList.Get(LeftObsList.GetCount()-1);
				}
				else{
					Dir = (Right - Pos);
					pTurnObstacle = RightObsList.Get(RightObsList.GetCount()-1);
				}
			}
			else{
				if(fLeft<=fRight){
					Dir = (Left - Pos);	// 짧은쪽으로 간다.
					pTurnObstacle = LeftObsList.Get(LeftObsList.GetCount()-1);
				}
				else{
					Dir = (Right - Pos);
					pTurnObstacle = RightObsList.Get(RightObsList.GetCount()-1);
				}
			}
			*/

			// Edge로 좌우 방향 선택 ( 양 Edge가 한 장애물애만 걸리는 경우, 판단을 제대로 못하는 경우가 생긴다. )
			/*
			float fLeftLength = (Target - Left).GetMagnitude();
			float fRightLength = (Target - Right).GetMagnitude();
			if(fLeftLength<=fRightLength){
				Dir = (Left - Pos);	// 짧은쪽으로 간다.
				pTurnObstacle = LeftObsList.Get(LeftObsList.GetCount()-1);
			}
			else{
				Dir = (Right - Pos);
				pTurnObstacle = LeftObsList.Get(LeftObsList.GetCount()-1);
			}
			*/

			fMagnitude = Dir.GetMagnitude();
			if(fMagnitude==0){
				Dir *= 0;
				break;
			}
			else Dir *= (1/fMagnitude);
		}
	}

	//_ASSERT(i<=1);

	rvector *pNearPath = NextPaths.Get(NextPaths.GetCount()-1);
	rvector NewDiff = *pNearPath - Pos;
	float fMag = GetMagnitude(NewDiff);
	if(fMag==0.0f) return rvector(0, 0, 0);
	return (NewDiff*(1/fMag));
}

