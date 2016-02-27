#include "stdafx.h"

#include "ZGame.h"
#include "ZApplication.h"
#include "ZEffectStaticMesh.h"
#include "Physics.h"
#include "ZCharacter.h"
#include "ZObject.h"
#include "RMaterialList.h"
#include "MDebug.h"

ZEffectMesh::ZEffectMesh(RMesh* pMesh, rvector& Pos, rvector& Velocity)
{
	m_VMesh.Create(pMesh);
//	bool bRet=m_VMesh.SetAnimation("play");
//	m_pMesh = pMesh;

	m_Pos = Pos;
	m_Velocity = Velocity;
	m_fRotateAngle = 0;
	m_nStartTime = timeGetTime();

	while(1) {

		m_RotationAxis = rvector((rand()%100), (rand()%100), (rand()%100));

		if(m_RotationAxis.x!=0 && m_RotationAxis.y!=0 && m_RotationAxis.z!=0){
			Normalize(m_RotationAxis);
			break;
		}
	}

	m_nDrawMode = ZEDM_NONE;

	m_Up = rvector(0.f,0.f,1.f);
//	g_effect_empty_cartridge_cnt++;
}

ZEffectStaticMesh::ZEffectStaticMesh(RMesh* pMesh, rvector& Pos, rvector& Velocity, MUID uid )
: ZEffectMesh(pMesh,Pos,Velocity)
{
	m_uid = uid;
}

#define EC_ROTATION	1.2f
#define EC_LIFETIME	1000
#define EC_LIMIT_DISTANCE	500.f

bool ZEffectStaticMesh::Draw(unsigned long int nTime)
{
	if(m_VMesh.m_pMesh==NULL) 
		return false;

	////////////////////////////////////////////////////////////////////////////////////
/*
	static char _buffer[40];
	sprintf(_buffer,"ZEffectStaticMesh::Draw : %d \n",g_effect_empty_cartridge_cnt);
	OutputDebugString(_buffer);
*/
	////////////////////////////////////////////////////////////////////////////////////

	DWORD dwDiff = nTime-m_nStartTime;

	float fSec = (float)dwDiff/1000.0f;	// msec에서 sec로 변환
	rvector Distance = ParabolicMotion(m_Velocity, fSec) * 100;	// *100은 미터에서 센티로 변환
	rvector Pos = m_Pos + Distance;
	float fOpacity = (EC_LIFETIME-dwDiff)/(float)EC_LIFETIME;

	//float fOpacity = 1.0f;
	rvector Dir(1,0,0);
	rvector Up = m_Up;
	rmatrix World;
	unsigned long int Opacity = 0xFF*fOpacity;
	MakeWorldMatrix(&World, Pos, Dir, Up);
	rmatrix Rotation;
	D3DXMatrixRotationAxis(&Rotation, &m_RotationAxis, m_fRotateAngle);
	m_fRotateAngle+=EC_ROTATION;
 	World = Rotation * World;
//	m_pMesh->Render(&World, (Opacity<<24)+(Opacity<<16)+(Opacity<<8)+Opacity);

//	m_VMesh.SetScale(m_Scale);
	m_VMesh.SetWorldMatrix(World);
//	m_VMesh.Frame();					// 시간은 흘러 가야 한다~

	if(m_bRender) {
		m_VMesh.Render();
		m_bisRendered = m_VMesh.m_bIsRender;
	} 
	else m_bisRendered = false;

	static const char* base_snd_name = "fx_slugdrop_";
	static char buffer[64];
	
	if( dwDiff > EC_LIFETIME )
	{
		ZObject* pObj = ZGetObjectManager()->GetObject(m_uid);
		
//		if( MIsDerivedFromClass(ZCharacterObject, pObj )==false) return false;
//		ZCharacterObject* pCObj = (ZCharacterObject*)pObj;

		ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pObj);

		if(!pCObj) return false;

		if( pCObj->m_pSoundMaterial == 0 )	// 기본소리 출력
		{
			ZGetSoundEngine()->PlaySound("fx_slugdrop_mt_con",Pos);
		} 
		else 
		{
			if( pCObj == NULL )
			{
				ZGetSoundEngine()->PlaySound("fx_slugdrop_mt_con",Pos);
			}
			else
			{
				strcpy( buffer, base_snd_name );
				strcat( buffer, pCObj->m_pSoundMaterial );

				ZGetSoundEngine()->PlaySoundElseDefault(buffer,"fx_slugdrop_mt_con",Pos);
			}			
		}
		return false;
		
	}
	return true;
}
