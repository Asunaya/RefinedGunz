#include "MRayCastingDriver2.h"

void MRayCastingDriver2::CheckOverlappedObstacleTemp(MObstacle* p, int nTemp)
{
	if(p->m_nTempUsed==nTemp) return;

	p->m_nTempUsed = nTemp;
	for(int i=0; i<p->m_OverlappedObstacles.GetCount(); i++){
		MObstacle* pObs = p->m_OverlappedObstacles.Get(i);
		CheckOverlappedObstacleTemp(pObs, nTemp);
	}
}

void MRayCastingDriver2::GetArea(float* pLeftArea, float* pRightArea, MObstacle* pColObstacle, rvector& Pos, rvector& Target)
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

void MRayCastingDriver2::GetArea(float* pLeftArea, float* pRightArea, MObstacle* pColObstacle, MObstacle* pAncestor, bool bAncestorLeft, rvector& Pos, rvector& Target, int nMagicNumber)
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

MRayCastingDriver2::MRayCastingDriver2(void)
{
}

MRayCastingDriver2::~MRayCastingDriver2(void)
{
}

void MRayCastingDriver2::AddObstacle(MObstacle* pObstacle)
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

void MRayCastingDriver2::AddObstacle(CMPtrList<MObstacle>* pOL)
{
	for(int i=0; i<pOL->GetCount(); i++){
		MObstacle* pObstacle = pOL->Get(i);
		AddObstacle(pObstacle);
	}
}

void MRayCastingDriver2::AddObstacle(CMLinkedList<MObstacle>* pOL)
{
	for(int i=0; i<pOL->GetCount(); i++){
		MObstacle* pObstacle = pOL->Get(i);
		AddObstacle(pObstacle);
	}
}

void MRayCastingDriver2::DeleteObstacle(void)
{
	m_Obstacles.DeleteAll();
}

bool MRayCastingDriver2::IsCollision(rvector* pNewPos, rvector& Pos)
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

/*
rvector MRayCastingDriver2::GetNextDir(rvector Pos, rvector Dir, float fMagnitude, rvector CurDir)
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

			rvector Left, Right;
			CMPtrList<MObstacle> LeftObsList;
			CMPtrList<MObstacle> RightObsList;
			pColObstacle->GetOverlappedEdge(&Left, &Right, Pos, &LeftObsList, &RightObsList);
			// 목표방향에 걸리는 오브젝트의 좌우 분포에 따라 방향 선택 ( 정확도는 떨어지지만, 보편적이다. )
			float fLeft = 0;
			float fRight = 0;
			GetArea(&fLeft, &fRight, pColObstacle, NULL, true, Pos, Target, (int)timeGetTime());
			//GetArea(&fLeft, &fRight, pColObstacle, Pos, Target);

			if(fLeft<=fRight){
				Dir = (Left - Pos);	// 짧은쪽으로 간다.
				pTurnObstacle = LeftObsList.Get(LeftObsList.GetCount()-1);
			}
			else{
				Dir = (Right - Pos);
				pTurnObstacle = RightObsList.Get(RightObsList.GetCount()-1);
			}

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

*/

bool MRayCastingDriver2::FindPath(rvector& Pos, rvector& Dir, float fMagnitude)
{
	return true;
}
