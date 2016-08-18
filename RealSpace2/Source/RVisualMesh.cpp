#include "stdafx.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "MDebug.h"
#include "RealSpace2.h"
#include "MProfiler.h"
#include "RCharCloth.h"
#include <functional>
#include "MeshManager.h"

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

/*
#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else
*/
#define __BP(i,n) ;
#define __EP(i) ;
//#endif

//////////////////////////////////////////////////////////////
// AniFrameInfo : frame 기능을 나눠주자

AniFrameInfo::AniFrameInfo()
{
	m_isOncePlayDone = false;
	m_isPlayDone	 = false;
	m_bChangeAnimation = false;

	m_nReserveTime	= 0;
	m_nFrame		= 0;
	m_nAddFrame		= 0;

	m_save_time		= 0;
	m_1frame_time	= 0;

	m_pAniSet			= NULL; 
	m_pAniSetNext		= NULL;
	m_pAniSetReserve	= NULL;

	m_bBlendAniSet	= false;
	m_fMaxBlendTime = 1.0f;
	m_fCurrentBlendTime = 0.5f;
	m_dwBackupBlendTime = 0;

	m_fSpeed = 4.8f;

	m_SoundInfo.Clear();
}

void AniFrameInfo::ClearFrame()
{
	m_nFrame			= 0;
	m_save_time			= GetGlobalTimeMS();
	m_pAniSetNext		= NULL;
	m_pAniSetReserve	= NULL;
	m_nReserveTime		= 0;
}

void AniFrameInfo::Frame(RAniMode amode,RVisualMesh* pVMesh)
{
	if( pVMesh==NULL )	return;

	if(m_pAniSet==NULL) return;

	DWORD cur = GetGlobalTimeMS();//전역 타이머 생길때까지..

	if(m_bChangeAnimation) {

		m_bChangeAnimation	= false;
		m_save_time			= cur;
		m_1frame_time		= cur;
		m_isPlayDone		= false;
		m_isOncePlayDone	= false;

		m_nFrame = m_nAddFrame;	// 모션상태는 같고 무기만 바뀐경우 때문에

		if(m_pAniSet->IsHaveSoundFile()) {
			RAniSoundInfo* pSInfo = &m_SoundInfo;

			if(strcmp(m_pAniSet->GetName(),"jumpD") != 0) // 임시 점프 다운은 사운드 파일 정보만 가지고 착지 시점에 play
				pSInfo->isPlay = true;

			pSInfo->SetName( m_pAniSet->GetSoundFileName() );
			pSInfo->SetRelatedToBsp( m_pAniSet->IsSoundRelatedToMap() );
			pSInfo->SetPos( pVMesh->m_WorldMat._41, pVMesh->m_WorldMat._42, pVMesh->m_WorldMat._43 );
		}
	}

	AnimationLoopType looptype = m_pAniSet->GetAnimationLoopType();
	int max_frame = m_pAniSet->GetMaxFrame();

	if( m_isPlayDone ) {// 1번은 play 한후..

		if( m_pAniSet->GetAnimationLoopType() == RAniLoopType_Loop)
			if( m_pAniSet->IsHaveSoundFile()) {
				RAniSoundInfo* pSInfo = &m_SoundInfo;
				pSInfo->isPlay = true;
				pSInfo->SetName(m_pAniSet->GetSoundFileName());
				pSInfo->SetRelatedToBsp(m_pAniSet->IsSoundRelatedToMap());
				pSInfo->SetPos(pVMesh->m_WorldMat._41,pVMesh->m_WorldMat._42,pVMesh->m_WorldMat._43);
			}

			m_isPlayDone = false;

			if( m_pAniSetNext ) {

				pVMesh->SetAnimation( amode,m_pAniSetNext );
				m_pAniSetNext = NULL;

			} else 	{	//	다음번 등록이 없다면..
				//	모션 이름으로 된것들 상태들이 정해지면 ID 로 바꾸기...

				if( looptype == RAniLoopType_OnceIdle) {
					m_nFrame = max_frame - 1;
					m_save_time = cur;
					m_1frame_time = cur;
					return;
				}
				else if( looptype == RAniLoopType_HoldLastFrame) {
					m_nFrame = max_frame - 1;
					m_isPlayDone = true;
					m_save_time = cur;
					m_1frame_time = cur;
					return;
				}
				else if( looptype == RAniLoopType_OnceLowerBody) {
					m_nFrame = max_frame - 1;
					m_pAniSet = NULL;
					m_save_time = cur;
					return;
				}
				else if( looptype == RAniLoopType_Normal) {//loop

				}
				else if( looptype == RAniLoopType_Loop) {

				}
			}
	}

	if( m_pAniSetReserve ) {

		if( cur > m_nReserveTime ) {

			pVMesh->SetAnimation(amode,m_pAniSetReserve);
			m_pAniSetReserve = NULL;
			m_nReserveTime = 0;
			m_save_time = cur;
			m_isPlayDone = false;
		}
	}

	// 에니메이션 상태의 것을 써야함..
	int bf = 0;
	int ef = max_frame;

	DWORD delta;

	if( m_save_time==0 ) {
		delta = 0;
		m_save_time = cur;
	}
	else 
		delta = cur - m_1frame_time;

//	m_fSpeed = 2.1f;

	int FrameAdvance = delta * m_fSpeed;

	if (m_pAniSet->PlaybackRate != 1)
	{
		FrameAdvance *= m_pAniSet->PlaybackRate;
	}

	m_nFrame += FrameAdvance;

	// frame 을 나눠서 사용할 경우 대비
	if(bf != 0) {
		m_nFrame += bf;
	}

	if ( m_nFrame >=  ef) {

		if( looptype == RAniLoopType_HoldLastFrame ) {
			m_nFrame = max_frame - 1;
		}
		else if( looptype == RAniLoopType_OnceIdle ) {
			m_nFrame = max_frame - 1;
		}
		else if( looptype == RAniLoopType_OnceLowerBody) {
			m_nFrame = max_frame - 1;
		}
		else if( looptype == RAniLoopType_Loop) {
			// 반복 타잎일 경우
			if(ef!=0)
				m_nFrame %= ef;
		}

		m_isPlayDone		= true;	// loop 떄마다..
		m_isOncePlayDone	= true; // 에니가 바뀔때까지 안변함. (1번은 플래이 되었다는걸 보장)
		m_save_time			= cur;
	}

	m_1frame_time = cur;
}


void RFrameTime::Start(float fMax,DWORD MaxTime,DWORD ReturnMaxTime) {

	m_fMaxValue = fMax;
	m_dwStartTime = GetGlobalTimeMS();
	m_dwEndTime = m_dwStartTime + MaxTime;
	m_dwReturnMaxTime = ReturnMaxTime;
	m_bActive = true;

}

void RFrameTime::Stop() {

	m_fCurValue = 0.f;
	m_fMaxValue = 0.f;
	m_bActive = false;
	m_bReturn = false;

}

void RFrameTime::Update() {

	if(!m_bActive) return;

	DWORD dwThisTime = GetGlobalTimeMS();

	if(dwThisTime > m_dwEndTime ) {

		if( m_bReturn || (m_dwReturnMaxTime==0) ) {
			Stop();
		}
		else { // return
			Start(m_fMaxValue, m_dwReturnMaxTime , 0);
			m_bReturn = true;
		}
	}

	// 여러가지 그래프 지원..

	if(m_nType==0) {

		// 시간에 따라 점차증가..점차감소 +-값에 따라..

		if(m_bReturn) {
			m_fCurValue = m_fMaxValue - (((dwThisTime - m_dwStartTime)/float(m_dwEndTime-m_dwStartTime)) * m_fMaxValue);
		}
		else {
			m_fCurValue = ((dwThisTime - m_dwStartTime)/float(m_dwEndTime-m_dwStartTime)) * m_fMaxValue;
		}


	}
	else if(m_nType==1) {

	}
}

//////////////////////////////////////////////////////////////////////////////////
RVisualLightMgr::RVisualLightMgr()
{
	for(int i=0;i<VISUAL_LIGHT_MAX;i++) {

		m_LightEnable[i] = 0;
	}
}

int RVisualLightMgr::GetLightCount()
{
	int nCnt = 0;

	for(int i=0;i<VISUAL_LIGHT_MAX;i++) {
		if(m_LightEnable[i]) nCnt++;
	}
	return nCnt;
}

void RVisualLightMgr::Clone(RVisualMesh* pVMesh)
{
	if(!pVMesh) return;
		 
	for(int i=0;i<VISUAL_LIGHT_MAX;i++) 
	{
		pVMesh->m_LightMgr.m_Light[i]		= m_Light[i];
		pVMesh->m_LightMgr.m_LightEnable[i] = m_LightEnable[i];
	}
}

void RVisualLightMgr::SetLight(int index,D3DLIGHT9* light,bool ShaderOnly)
{
	if(light) {
		m_Light[index] = *light;
		if(ShaderOnly)
			m_LightEnable[index] = 2;
		else 
			m_LightEnable[index] = 1;
	}
	else {
		m_LightEnable[index] = 0;
	}
}

void RVisualLightMgr::UpdateLight()
{
	for(int i=0;i<VISUAL_LIGHT_MAX;i++) {

		if(m_LightEnable[i]==1) {

			RGetDevice()->SetLight( i, &m_Light[i] );
			RGetDevice()->LightEnable( i, TRUE );

			if( RShaderMgr::mbUsingShader ) 
			{
				RGetShaderMgr()->setLight( i, &m_Light[i] );
				RGetShaderMgr()->LightEnable( i, TRUE );
			}
		}
		else if(m_LightEnable[i]==2) // shaderonly
		{
			RGetDevice()->LightEnable( i, FALSE );

			if( RShaderMgr::mbUsingShader ) 
			{
				RGetShaderMgr()->setLight( i, &m_Light[i] );
				RGetShaderMgr()->LightEnable( i, TRUE );
			}
		}
		else {
			RGetDevice()->LightEnable( i, FALSE );
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// RVisualMesh

RVisualMesh::RVisualMesh() {

	m_pMesh = NULL;
	m_pLowPolyMesh = NULL;

	m_pTMesh = NULL;

	m_id = -1;

	m_nAnimationState = APState_Stop;

	m_vBMax = rvector( 1.f, 1.f, 1.f);
	m_vBMin = rvector(-1.f,-1.f,-1.f);

	m_isScale = false;
	m_vScale = rvector(1.f,1.f,1.f);

	m_bIsRender = false;
	m_bIsRenderWeapon = false;
	m_bIsRenderFirst = true;

	m_isDrawWeaponState = false;

	for(int i=0;i<eq_weapon_end;i++) {
		m_WeaponVisualMesh[i] = NULL;
	}

	for(int i=0;i<eq_parts_end;i++) {
		m_WeaponPartInfo[i].Init();
	}

	for(int i=0;i<weapon_dummy_end;i++) {

		D3DXMatrixIdentity(&m_WeaponDummyMatrix[i]);
		D3DXMatrixIdentity(&m_WeaponDummyMatrix2[i]);
	}
	
	m_SelectWeaponMotionType = eq_weapon_etc;

	m_vRotXYZ = rvector(0.f,0.f,0.f);

	m_vTargetPos = rvector(0.f,0.f,0.f);

	D3DXMatrixIdentity(&m_ToonUVMat);

	D3DXMatrixIdentity(&m_RotMat);

	D3DXMatrixIdentity( &m_UpperRotMat );

	D3DXMatrixScaling(&m_ScaleMat,1.f,1.f,1.f);
	
	m_pTracks[0] = NULL;
	m_pTracks[1] = NULL;

	m_bDrawTracks = false;

	m_bDrawTracksMotion[0] = true;
	m_bDrawTracksMotion[1] = true;

	m_bCheckViewFrustum = true;
	m_bGrenadeRenderOnoff = true;
	m_bGrenadeFire = false;
	m_bAddGrenade = false;
	m_bDrawGrenade = true;

	m_fVis = 1.0f;

	m_vPos = rvector(0.f,0.f,0.f);
	m_vDir = rvector(0.f,0.f,0.f);
	m_vUp = rvector(0.f,0.f,0.f);

	D3DXMatrixIdentity( &m_WorldMat );

	m_bIsCharacter = false;
	m_bIsNpc = false;

	m_pBipMatrix = NULL;

	m_bRenderInstantly = false;

	m_pAniNodeTable = NULL;
	m_nAniNodeTableCnt = 0;

	m_bRenderMatrix = false;

	m_EnchantType = REnchantType_None;
//	m_EnchantType = REnchantType_Fire;

	m_GrenadeFireTime = 0;

	m_pTOCCL = NULL;

	m_NPCBlendColor.r = 0.6f;
	m_NPCBlendColor.g = 0.6f;
	m_NPCBlendColor.b = 0.6f;
	m_NPCBlendColor.a = 1.0f;

	m_fUAniValue = 0.f;
	m_fVAniValue = 0.f;
	m_bUVAni = false;

	m_pCloth = NULL;
	m_bClothGame = false;
	m_fClothDist = 0.f;

	m_ToonTexture	= NULL;//임시..
	m_bToonLighting = true;
	m_bToonTextureRender = false;
	m_bToonColor = 0xffffffff;

	SetCustomColor(0, 0);
}

RVisualMesh::~RVisualMesh() 
{
	if (IsDynamicResourceLoad())
		GetMeshManager()->OnDestroyObject(this);

	Destroy();
}

void RVisualMesh::Destroy()
{
	DestroyCloth();

	RemoveAllWeapon();

	if(m_pTracks[0]) {
		delete m_pTracks[0];
		m_pTracks[0] = NULL;
	}
	//이도류~
	if(m_pTracks[1]) {
		delete m_pTracks[1];
		m_pTracks[1] = NULL;
	}

	if(m_pTMesh) {
		if (IsDynamicResourceLoad())
		{
			for (int i = 0; i < eq_parts_end; i++)
			{
				if (!m_pTMesh[i])
					continue;

				switch (i)
				{
				case eq_parts_head:
				case eq_parts_chest:
				case eq_parts_hands:
				case eq_parts_legs:
				case eq_parts_feet:
					GetMeshManager()->Release(m_pTMesh[i]);
				}
			}
		}

		delete [] m_pTMesh;
		m_pTMesh = NULL;
	}

	if(m_pBipMatrix) {
		delete [] m_pBipMatrix;
		m_pBipMatrix = NULL;
	}

	if(m_pAniNodeTable) {
		delete [] m_pAniNodeTable;
		m_pAniNodeTable = NULL;
	}
}

bool RVisualMesh::Create(RMesh* pMesh) {

	m_pMesh = pMesh;
//	D3DXMatrixIdentity(&m_WorldMat);

	if (m_pMesh) {
		m_bIsCharacter = m_pMesh->m_isCharacterMesh;
		m_bIsNpc = m_pMesh->m_isNPCMesh;
	}

	// 캐릭터라면~
	if( m_bIsCharacter || m_bIsNpc) {

		if(m_bIsCharacter) {
			if(m_pTMesh==NULL) {

				m_pTMesh = new RMeshNode*[eq_parts_end];

				for(int i=0;i<eq_parts_end;i++){
					m_pTMesh[i] = NULL;
				}
			}
		}

		if(m_pBipMatrix==NULL) {

			m_pBipMatrix = new rmatrix [eq_parts_pos_info_end];

			for(int i=0;i<eq_parts_pos_info_end;i++){
				D3DXMatrixIdentity( &m_pBipMatrix[i] );
			}
		}
	}

//	if(RIsQuery())
//		m_RenderQuery.Create( RGetDevice() );

	return true;
}

bool RVisualMesh::BBoxPickCheck(int mx,int my)
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();

	int sw = RGetScreenWidth();
	int sh = RGetScreenHeight();

	rvector pos,dir;

	rmatrix matProj = RProjection;

	rvector v;

	v.x =  ( ( ( 2.0f * mx ) / sw ) - 1 ) / matProj._11;
	v.y = -( ( ( 2.0f * my ) / sh ) - 1 ) / matProj._22;
	v.z =  1.0f;

	rmatrix m,matView = RView;

	D3DXMatrixInverse( &m, NULL, &matView );

	dir.x  = v.x*m._11 + v.y*m._21 + v.z*m._31;
	dir.y  = v.x*m._12 + v.y*m._22 + v.z*m._32;
	dir.z  = v.x*m._13 + v.y*m._23 + v.z*m._33;

	pos.x = m._41;
	pos.y = m._42;
	pos.z = m._43;

	D3DXVec3Normalize(&dir,&dir);

	return BBoxPickCheck(pos,dir);
}

void BBoxSubCalc(D3DXVECTOR3* max,D3DXVECTOR3* min)
{
	float t;

	if(max->x < min->x) {	t = max->x;	max->x = min->x; min->x = t; }
	if(max->y < min->y) {	t = max->y;	max->y = min->y; min->y = t; }
	if(max->z < min->z) {	t = max->z; max->z = min->z; min->z = t; }
}

bool RVisualMesh::BBoxPickCheck(const rvector& pos, const rvector& dir)
{
	rvector min,max;

	CalcBox();

	D3DXVec3TransformCoord(&min,&m_vBMax,&m_WorldMat);
	D3DXVec3TransformCoord(&max,&m_vBMin,&m_WorldMat);

	BBoxSubCalc(&max,&min);

	return D3DXBoxBoundProbe(&min,&max,&pos,&dir) ? true : false;
}

bool RVisualMesh::Pick(int x,int y,RPickInfo* pInfo)
{
	if(m_pMesh) {
		if(!BBoxPickCheck(x,y))
			return false;

		return m_pMesh->Pick(x,y,pInfo,&m_WorldMat);
	}
	return false;
}

bool RVisualMesh::Pick(const rvector* vInVec,RPickInfo* pInfo)
{
	if(m_pMesh) {
		if(!BBoxPickCheck(vInVec[0],vInVec[1]))
			return false;

		return m_pMesh->Pick(vInVec,pInfo,&m_WorldMat);
	}
	return false;
}

bool RVisualMesh::Pick(const rvector& pos, const rvector& dir,RPickInfo* pInfo)
{
	if(m_pMesh) {

		if(!BBoxPickCheck(pos,dir))
			return false;

		return m_pMesh->Pick(pos,dir,pInfo,&m_WorldMat);
	}
	return false;
}

bool RVisualMesh::Pick(const rvector& pos, const rvector& dir,rvector* v,float* t)
{
	if(m_pMesh) {

		if(!BBoxPickCheck(pos,dir))
			return false;

		RPickInfo info;
		bool hr = m_pMesh->Pick(pos,dir,&info,&m_WorldMat);

		*v = info.vOut;
		*t = info.t;

		return hr;
	}
	return false;
}

void RVisualMesh::Play(RAniMode amode) {

	AniFrameInfo* pInfo = GetFrameInfo(amode);
	if( pInfo == NULL ) return;

	m_nAnimationState = APState_Play;

	pInfo->m_isPlayDone		= false;
	pInfo->m_isOncePlayDone	= false;
	pInfo->m_save_time		= GetGlobalTimeMS();
}

void RVisualMesh::Stop(RAniMode amode) {

	AniFrameInfo* pInfo = GetFrameInfo(amode);
	if( pInfo == NULL ) return;

	m_nAnimationState = APState_Stop;

	if(m_pMesh) {
		pInfo->m_nFrame = 0;
	}

	pInfo->m_isPlayDone = false;
//	pInfo->m_isOncePlayDone = false;
}

// 작동 안함 전역타이머가 없다면 틱을 보관해 줘야함...별로 사용할 필요가 없을것 같음..
void RVisualMesh::Pause(RAniMode amode) {

	AniFrameInfo* pInfo = GetFrameInfo(amode);
	if( pInfo == NULL ) return;

	m_nAnimationState = APState_Pause;//m_nFrame 은 유지..

	pInfo->m_isPlayDone = false;
}

int RVisualMesh::GetMaxFrame(RAniMode amode) 
{
	AniFrameInfo* pInfo = GetFrameInfo(amode);
	if( pInfo == NULL ) return 0;

	if( pInfo->m_pAniSet ) {
		return pInfo->m_pAniSet->GetMaxFrame();
	}
	return 0;
}


void RVisualMesh::Frame(RAniMode amode)
{
	AniFrameInfo* pInfo = GetFrameInfo(amode);

	if( pInfo == NULL ) return;

	pInfo->Frame(amode,this);
}

void RVisualMesh::Frame() {

	if(m_pMesh) {

		if( m_nAnimationState == APState_Play )	{

			Frame(ani_mode_lower);
			Frame(ani_mode_upper);
		}
		else if( m_nAnimationState == APState_Pause ) {

		}
	}
}

//매트릭스위치들만 갱신한다.

void RVisualMesh::RenderMatrix()
{
	m_bRenderMatrix = true;
	
	Render(false,false);

	m_bRenderMatrix = false;
}

void RVisualMesh::Render(ROcclusionList* pOCCL)
{
	m_pTOCCL = pOCCL;

	Render(false,false);

	m_pTOCCL = NULL;
}

void RVisualMesh::Render(bool low,bool render_buffer) {

	if(m_pMesh) {

		rboundingbox bbox;

		CalcBox(); // 최초만 계산됨.

		rboundingbox srcbox;

		m_bAddGrenade = false;

		/*	
		if(m_isScale) { // 스케일이 사용된 경우만...
			srcbox.vmax = m_vBMax * m_ScaleMat;
			srcbox.vmin = m_vBMin * m_ScaleMat;
		}else
		*/
		{
			srcbox.vmax = m_vBMax;
			srcbox.vmin = m_vBMin;
		}

		TransformBox(&bbox,srcbox,m_WorldMat);


//		D3DXVec3TransformCoord(&bbox.vmin,&m_vBMax,&m_WorldMat);
//		D3DXVec3TransformCoord(&bbox.vmax,&m_vBMin,&m_WorldMat);

//		BBoxSubCalc(&bbox.vmax,&bbox.vmin);

//		if( m_pMesh->m_bEffectSort ) {

			if(m_fVis < 0.001f) {//죽어 있는 경우등..
				m_bIsRender = false;
				return;
			}

			if(m_bCheckViewFrustum) {//tool 이라면 false
				if(isInViewFrustumWithZ( &bbox, RGetViewFrustum()) == false) {
					m_bIsRender = false;
					return;
				}
			}

//			draw_box(&GetIdentityMatrix(),bbox.vmax,bbox.vmin,0xffffFFF0);

//		}
		
		///////////////////////////////////////////////////////////////////

		// 몬스터와 캐릭터 추가하고  ( 캐릭터도 자신의 것은 체크안하고 ) 
		//  미리계산해줄것들..
		//	거리가 가까워서 보일거라고 판단되는것은 사용안함,, 
		//  캐릭터나 몬스터는 기본적으로 picking 한번 해봐서 벽이 안집히면..그려주기..( 무조건 보인다는 뜻이니까 )
		// 이펙트는? 인터페이스는 무조건 off m_bQuery 로 사용할지 여부를 visual mesh 가 가지고 있기..
		// 
/*
		if( RIsQuery() ) {

			m_RenderQuery.Begin();

			draw_query_fill_box(&m_WorldMat,m_vBMax,m_vBMin,0x00ff0000);

			m_RenderQuery.End();

			// 이전프레임에 그려졌었는가?

			if(m_RenderQuery.isNeedRender()==false) {
				m_bIsRender = false;
//				char temp[1024];
//				sprintf_safe(temp,"%s skip query render \n ", m_pMesh->GetFileName() );
//				OutputDebugString( temp );
				return;
			}


		}
*/
		///////////////////////////////////////////////////////////////////

		m_bIsRender = true; // 그려졌다고 표시

		// 모델 부위별로 scale 먹이는것 고려..체형을 결정할 수 있게 된다..

		if(m_isScale) {		
			m_pMesh->SetScale(m_vScale);
		}

		bool isRenderedLowPolyModel = false;

		__BP(199, "RVisualMeshMgr::Render::MeshRenderLow");

		AniFrameInfo* pAniLow = GetFrameInfo(ani_mode_lower);
		AniFrameInfo* pAniUp  = GetFrameInfo(ani_mode_upper);

		m_FrameTime.Update(); //--------------------------------

		if(low) {

			if(m_pLowPolyMesh) {

				m_pLowPolyMesh->SetAnimation( pAniLow->m_pAniSet,pAniUp->m_pAniSet );
				m_pLowPolyMesh->SetFrame(pAniLow->m_nFrame,pAniUp->m_nFrame);
				m_pLowPolyMesh->SetMeshVis(m_fVis);
				m_pLowPolyMesh->SetVisualMesh(this);
				m_pLowPolyMesh->Render(&m_WorldMat,true); // 파츠교환없이 자신만 그리기..

				m_vBMax = m_pLowPolyMesh->m_vBBMax;
				m_vBMin = m_pLowPolyMesh->m_vBBMin;

				isRenderedLowPolyModel = true;
			}
		}

		__EP(199);

		__BP(201, "RVisualMeshMgr::Render::MeshRender");

		if(!isRenderedLowPolyModel) {

			m_pMesh->SetAnimation( pAniLow->m_pAniSet,pAniUp->m_pAniSet );

			m_pMesh->SetFrame( pAniLow->m_nFrame,pAniUp->m_nFrame);
			m_pMesh->SetMeshVis(m_fVis);

			m_pMesh->SetVisualMesh(this);
/*
			if(render_buffer)
				m_pMesh->RenderS(&m_WorldMat, false);
			else  */
				m_pMesh->Render(&m_WorldMat, false);

			if(m_pMesh->GetPhysiqueMesh()) { 
				
				m_vBMax = m_pMesh->m_vBBMaxNodeMatrix * 1.1f;
				m_vBMin = m_pMesh->m_vBBMinNodeMatrix * 1.1f;
			}
			else {

				// RMesh::m_max, m_min 제대로 계산되어있지 않다.
				/*
				if(m_pMesh->m_is_map_object) {
					m_vBMax = m_pMesh->m_max;
					m_vBMin = m_pMesh->m_min;
				}
				*/
			}
		}

		__EP(201);

		RenderWeapon();

		// debug

//		draw_box(&m_WorldMat,m_vBMax,m_vBMin,0xffff0000);
//		draw_box(&m_WorldMat,rvector(5,10,5),rvector(-5,0,-5),0xffffffff);
//		RMeshNode* pMeshNode = m_pMesh->GetMeshData("eq_chest_a001");
	}
}

bool RVisualMesh::UpdateSpWeaponFire()
{
	AniFrameInfo* pAniUp  = GetFrameInfo(ani_mode_upper);

	if(!pAniUp) return false;

	m_bGrenadeRenderOnoff = true;// 기본적으로 항상 보인다...

	if( m_bDrawGrenade ) {// 남은 양이 없다면 쥐고 있지도 않는다~

		if(pAniUp->m_pAniSet) {

			if( strcmp(pAniUp->m_pAniSet->GetName(),"attackS")==0 ) {

				if( m_bGrenadeFire ) { // 발사 상태라면

					// 특수무기를 던져야하는 시점

					if( pAniUp->m_nFrame > 2 * 160 ) {
						m_bGrenadeRenderOnoff = false;
						m_bGrenadeFire = false;
						m_bAddGrenade = true;
						return true;
					}
				}
			}
		}
	}

	// alt + tab 등의 비정상적인경우

	if(m_bGrenadeFire) {

		if( m_GrenadeFireTime + 70 < GetGlobalTimeMS() ) {

			m_bGrenadeRenderOnoff = false;
			m_bGrenadeFire = false;
			m_bAddGrenade = true;
			return true;
		}
	}

	return false;
}

void RVisualMesh::GetMotionInfo(int& sel_parts,int& sel_parts2,bool& bCheck,bool& bRender)
{
	AniFrameInfo* pAniUp  = GetFrameInfo(ani_mode_upper);

	switch(m_SelectWeaponMotionType) {

		case eq_ws_dagger:	
			{ 
				sel_parts = eq_parts_right_dagger;	
			}		
			break;

		case eq_wd_dagger:	
			{ 
				sel_parts  = eq_parts_right_dagger; 
				sel_parts2 = eq_parts_left_dagger; 
				bCheck = true;
			}		
			break;

		case eq_wd_katana:	
			{ 
				sel_parts = eq_parts_right_katana; 
			}		
			break;

		case eq_wd_sword:	
			{ 
				sel_parts = eq_parts_right_sword; 
			}		
			break;

		case eq_wd_blade:	
			{ 
				sel_parts  = eq_parts_right_blade;
				sel_parts2 = eq_parts_left_blade;
				bCheck = true;
			}
			break;

		case eq_ws_pistol:	
			{ 
				sel_parts = eq_parts_right_pistol; 
			}		
			break;

		case eq_wd_pistol://양손에 하나씩...
			{
				sel_parts  = eq_parts_right_pistol;
				sel_parts2 = eq_parts_left_pistol;
				bCheck = true;
			}
			break;

		case eq_ws_smg:
			{
				sel_parts = eq_parts_right_smg;
			}
			break;

		case eq_wd_smg://양손에 하나씩...
			{
				sel_parts  = eq_parts_right_smg;
				sel_parts2 = eq_parts_left_smg;
				bCheck = true;
			}
			break;

		case eq_wd_shotgun:
			{
				sel_parts = eq_parts_right_shotgun;
			}
			break;

		case eq_wd_rifle:
			{
				sel_parts = eq_parts_right_rifle;
			}
			break;
			
		case eq_wd_grenade:
			{
				if( m_bDrawGrenade ) {//남은 양이 없다면 쥐고 있지도 않는다~
				
					bRender = m_bGrenadeRenderOnoff;
				}
				else {
					bRender = false;
				}

				sel_parts = eq_parts_right_grenade;
			}
			break;

		case eq_wd_item:
			{
				sel_parts = eq_parts_right_item;
			}
			break;

		case eq_wd_rlauncher:
			{
				sel_parts = eq_parts_right_rlauncher;
			}
			break;

		default:

			break;
	}

}

/*

남자 ( 우 좌 무 쌍 )
여자 ( 좌 우 좌 우 )

0 무
1 좌 
2 우
3 쌍

*/

static int g_bDrawWeaponTrack[2][4] = {
	{2,1,0,3},
	{1,2,1,2},
};

// 무기가 쌍칼류인 경우만..

void GetRenderTrack(int isMan,int nMotion,bool& left,bool& right)
{
	
}

void RVisualMesh::DrawEnchant(RVisualMesh* pVWMesh,int mode,rmatrix& m)
{
	return;

	if(m_EnchantType==REnchantType_None)
		return;

	if(m_EnchantType==REnchantType_Fire) {
		DrawEnchantFire(pVWMesh,mode,m);
	}
	else if(m_EnchantType==REnchantType_Cold) {
		DrawEnchantCold(pVWMesh,mode,m);
	}
	else if(m_EnchantType==REnchantType_Lightning) {
		DrawEnchantLighting(pVWMesh,mode,m);
	}
	else if(m_EnchantType==REnchantType_Poison) {
		DrawEnchantPoison(pVWMesh,mode,m);
	}
}

// 마지막으로 추가한 궤적위치를 얻어낸다..최대 4개..

void RVisualMesh::SetSpRenderMode(int mode) 
{
	if(m_pMesh) {
		m_pMesh->SetSpRenderMode(mode);

		if(m_WeaponVisualMesh[m_SelectWeaponMotionType])
			m_WeaponVisualMesh[m_SelectWeaponMotionType]->SetSpRenderMode(mode);
	}
}

void RVisualMesh::ClearPartInfo()
{
	for(int i=0;i<eq_parts_end;i++) {
		m_WeaponPartInfo[i].isUpdate = false;

	}
}


int	RVisualMesh::GetLastWeaponTrackPos(rvector* pOutVec)
{
	int cnt = 0;

	if(m_pTracks[0]) {
		cnt += m_pTracks[0]->GetLastAddVertex(&pOutVec[0]);
	}

	if(m_pTracks[1]) {
		cnt += m_pTracks[1]->GetLastAddVertex(&pOutVec[cnt]);
	}

	return cnt;
}

static bool g_toggle = false;

void RVisualMesh::DrawEnchantFire(RVisualMesh* pVWMesh,int mode,rmatrix& m)
{
	static RFireEffectTexture m_FireEffectTexture;

	if(g_toggle==false) {
		m_FireEffectTexture.Create( RGetDevice(), 128,128 );
		g_toggle = true;
	}

//	if(GetGlobalTimeMS()%3==0)
	m_FireEffectTexture.ProcessFire(1);

	m_FireEffectTexture.UpdateTexture();

	// 기본적으로지금무기의 bbox 로 상위로 향하는 불길을 만든다..

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	if( m_WeaponVisualMesh[ m_SelectWeaponMotionType ] ) {

		RVisualMesh* pVWMesh = m_WeaponVisualMesh[ m_SelectWeaponMotionType ];

		rvector vmax,vmin,vcenter;
		rvector vpos[10];

		vmax = pVWMesh->m_vBMax;
		vmin = pVWMesh->m_vBMin;

		vcenter = (vmax-vmin)/2;

		vpos[0] = vmin + vcenter;
		vpos[1] = vmin + vcenter;

		vpos[0].y = vmax.y;
		vpos[1].y = vmin.y + 35.f; // 손잡이 만큼 뺴기..
/*
		vpos[2] = vmax*0.25f;
		vpos[3] = vmin*0.25f;

		vpos[4] = vmax*0.5f;
		vpos[5] = vmin*0.5f;

		vpos[6] = vmax*0.75f;
		vpos[7] = vmin*0.75f;

		vpos[8] = vmax;
		vpos[9] = vmin;
*/
		vpos[2] = vmax;
		vpos[3] = vmin;

		D3DXVec3TransformCoord(&vpos[0],&vpos[0],&m);
		D3DXVec3TransformCoord(&vpos[1],&vpos[1],&m);
		D3DXVec3TransformCoord(&vpos[2],&vpos[2],&m);
		D3DXVec3TransformCoord(&vpos[3],&vpos[3],&m);
/*
		D3DXVec3TransformCoord(&vpos[4],&vpos[4],&m);
		D3DXVec3TransformCoord(&vpos[5],&vpos[5],&m);
		D3DXVec3TransformCoord(&vpos[6],&vpos[6],&m);
		D3DXVec3TransformCoord(&vpos[7],&vpos[7],&m);
		D3DXVec3TransformCoord(&vpos[8],&vpos[8],&m);
		D3DXVec3TransformCoord(&vpos[9],&vpos[9],&m);
*/
		static RLVertex pVert[10];

		rvector add[2];

		add[0] = rvector(0,0,0);
		add[1] = add[0];
/*
		if(GetGlobalTimeMS()%15==0) {

			add[0].x = rand()%5;
			add[0].y = rand()%5;

			add[1].x = rand()%5;
			add[1].y = rand()%5;
		}
*/		
		DWORD color = 0xaf9f9f9f;

//------------------------------------

		pVert[0].p = vpos[0];
		pVert[0].color = color;
		pVert[0].tu = 0.0f;
		pVert[0].tv = 0.9f;

		pVert[1].p = vpos[1];
		pVert[1].color = color;
		pVert[1].tu = 1.0f;
		pVert[1].tv = 0.9f;

//------------------------------------
/*
		pVert[2].p = vpos[2];
		pVert[2].color = color;
		pVert[2].tu = 0.0f;
		pVert[2].tv = 0.9f;

		pVert[3].p = vpos[3];
		pVert[3].color = color;
		pVert[3].tu = 1.0f;
		pVert[3].tv = 0.9f;

//------------------------------------

		pVert[4].p = vpos[4];
		pVert[4].color = color;
		pVert[4].tu = 0.0f;
		pVert[4].tv = 0.9f;

		pVert[5].p = vpos[5];
		pVert[5].color = color;
		pVert[5].tu = 1.0f;
		pVert[5].tv = 0.9f;

//------------------------------------

		pVert[6].p = vpos[6];
		pVert[6].color = color;
		pVert[6].tu = 0.0f;
		pVert[6].tv = 0.9f;

		pVert[7].p = vpos[7];
		pVert[7].color = color;
		pVert[7].tu = 1.0f;
		pVert[7].tv = 0.9f;

//------------------------------------

		pVert[8].p = vpos[8];
		pVert[8].p.x += add[0].x;
		pVert[8].p.y += add[0].y;
		pVert[8].p.z += 50.f;
		pVert[8].color = color;
		pVert[8].tu = 0.0f;
		pVert[8].tv = 0.0f;

		pVert[9].p = vpos[9];
		pVert[9].p.x += add[1].x;
		pVert[9].p.y += add[1].y;
		pVert[9].p.z += 50.f;
		pVert[9].color = color;
		pVert[9].tu = 1.0f;
		pVert[9].tv = 0.0f;
*/
		pVert[2].p = vpos[2];
		pVert[2].p.x += add[0].x;
		pVert[2].p.y += add[0].y;
		pVert[2].p.z += 50.f;
		pVert[2].color = color;
		pVert[2].tu = 0.0f;
		pVert[2].tv = 0.0f;

		pVert[3].p = vpos[3];
		pVert[3].p.x += add[1].x;
		pVert[3].p.y += add[1].y;
		pVert[3].p.z += 50.f;
		pVert[3].color = color;
		pVert[3].tu = 1.0f;
		pVert[3].tv = 0.0f;


//		RDrawLine(vpos[0],vpos[1],0xff00ff00);
//		RDrawLine(vpos[2],vpos[3],0xff00ffff);

		static D3DXMATRIX _init_mat = GetIdentityMatrix();
		dev->SetTransform( D3DTS_WORLD, &_init_mat );

		dev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		dev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
//		dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
//		dev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		dev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

		dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		dev->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
		dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE);
//		dev->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
//		dev->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		dev->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
		dev->SetRenderState( D3DRS_ALPHAREF,         0x08 );
		dev->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );

		dev->SetRenderState( D3DRS_LIGHTING, FALSE );
		dev->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

//		dev->SetTexture(0, NULL);
		dev->SetTexture(0, m_FireEffectTexture.GetTexture());
		dev->SetFVF( RLVertexType );

		dev->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2 ,(LPVOID)pVert, sizeof(RLVertex));

		dev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
		dev->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);
		dev->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE);
		dev->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		dev->SetRenderState( D3DRS_LIGHTING, TRUE );
		
	}
}

void RVisualMesh::DrawEnchantCold(RVisualMesh* pVWMesh,int mode,rmatrix& m)
{

}

void RVisualMesh::DrawEnchantLighting(RVisualMesh* pVWMesh,int mode,rmatrix& m)
{

}

void RVisualMesh::DrawEnchantPoison(RVisualMesh* pVWMesh,int mode,rmatrix& m)
{

}

void RVisualMesh::RenderWeapon()
{
	if( m_WeaponVisualMesh[ m_SelectWeaponMotionType ] ) {

		// 부모가 특수한 알파처리라면? 무기의 궤적은 그릴 필요 없다..

		bool btrack_render = true;

		if(m_pMesh)
			if(m_pMesh->m_nSpRenderMode==1)
				btrack_render = false;

		RVisualMesh* pVWMesh = m_WeaponVisualMesh[ m_SelectWeaponMotionType ];

		m_LightMgr.Clone(pVWMesh);//라이트 설정 정보를 복사...
		
		bool b2hCheck = false;	// 양손인지 체크
		bool bRender = true;

		rmatrix m,m2;
		float vis,vis2;

		int sel_parts = 0;
		int sel_parts2 = 0;

		GetMotionInfo(sel_parts,sel_parts2,b2hCheck,bRender);

		m = m_WeaponPartInfo[sel_parts].mat * m_WorldMat;
		vis = m_WeaponPartInfo[sel_parts].vis;

		// 부모가 뷰 체크 안하면 무기 더미도 안한다~

		if( m_bCheckViewFrustum==false ) {
			pVWMesh->SetCheckViewFrustum(false);
		}

		if( m_bRenderMatrix )
			pVWMesh->m_bRenderMatrix = true;

		pVWMesh->SetWorldMatrix(m);

		if( bRender && vis) {
			pVWMesh->SetVisibility(min(m_fVis,vis)); // 자기 캐릭터와 자신의 visibility 중 더 작은것..
			pVWMesh->Render();
			m_bIsRenderWeapon = true;
		}
		else { // 위치는 쓸지도 모르니 계산해주자..ex > 수류탄을 던진경우..
			pVWMesh->CalcBox();//최초만계산
			m_bIsRenderWeapon = false;
		}

		// 장비한 무기에 따라서

		if(m_isScale) {
			pVWMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash] = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash];
			pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge01]  = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge01];
			pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge02]  = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge02];
		}

		m_WeaponDummyMatrix[weapon_dummy_muzzle_flash] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash]*m;
		m_WeaponDummyMatrix[weapon_dummy_cartridge01 ] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge01]*m;
		m_WeaponDummyMatrix[weapon_dummy_cartridge02 ] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge02]*m;

		if(m_bRenderMatrix)
			pVWMesh->m_bRenderMatrix = false;

		if(btrack_render) {
			DrawTracks(m_bDrawTracks,pVWMesh,0,m);
			DrawEnchant(pVWMesh,0,m);
		}

		// 양손 무기의 경우

		if(b2hCheck) {

			m = m_WeaponPartInfo[sel_parts2].mat * m_WorldMat;
			vis2 = m_WeaponPartInfo[sel_parts2].vis;

			if( m_bRenderMatrix )
				pVWMesh->m_bRenderMatrix = true;

			pVWMesh->SetWorldMatrix(m);

			if(vis2) {
				pVWMesh->SetVisibility(min(m_fVis,vis2));
				pVWMesh->Render();
			}
			else {
				pVWMesh->CalcBox();
			}

			if(m_isScale) {
				pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_muzzle_flash] = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_muzzle_flash];
				pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_cartridge01]  = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_cartridge01];
				pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_cartridge02]  = m_ScaleMat * pVWMesh->m_WeaponDummyMatrix2[weapon_dummy_cartridge02];
			}

			m_WeaponDummyMatrix2[weapon_dummy_muzzle_flash] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_muzzle_flash]*m;
			m_WeaponDummyMatrix2[weapon_dummy_cartridge01 ] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge01]*m;
			m_WeaponDummyMatrix2[weapon_dummy_cartridge02 ] = pVWMesh->m_WeaponDummyMatrix[weapon_dummy_cartridge02]*m;

			if(m_bRenderMatrix)
				pVWMesh->m_bRenderMatrix = false;

			if(btrack_render) {
				DrawTracks(m_bDrawTracks,pVWMesh,1,m);
				DrawEnchant(pVWMesh,0,m);
			}
		}
	}

}

void RVisualMesh::GetWeaponPos(rvector* pOut,bool bLeft)
{
	if(!pOut) return;

	RVisualMesh* pVWMesh = m_WeaponVisualMesh[ m_SelectWeaponMotionType ];

	if(!pVWMesh) return;

	rvector vmax,vmin,vcenter,p1,p2;

	vmax = pVWMesh->m_vBMax;
	vmin = pVWMesh->m_vBMin;

	vmax.y = 0;
	vmin.y = 0;

	vcenter = (vmax-vmin)/2;

	 p1 = pVWMesh->m_vBMax - vcenter;
	 p2 = pVWMesh->m_vBMin + vcenter;

	 bool b2hCheck = false;	// 양손인지 체크
	 bool bRender = true;

	 rmatrix m,m2;

	 int sel_parts = 0;
	 int sel_parts2 = 0;

	 GetMotionInfo(sel_parts,sel_parts2,b2hCheck,bRender);

	 if(bLeft)
		m = m_WeaponPartInfo[sel_parts2].mat * m_WorldMat;
	 else	
		m = m_WeaponPartInfo[sel_parts].mat * m_WorldMat;

	D3DXVec3TransformCoord(&p1,&p1,&m);
	D3DXVec3TransformCoord(&p2,&p2,&m);

	pOut[0] = p1;
	pOut[1] = p2;

}

float RVisualMesh::GetWeaponSize() 
{
	RVisualMesh* pVWMesh = m_WeaponVisualMesh[ m_SelectWeaponMotionType ];

	if(!pVWMesh) return 0.f;

	float fWeaponSize = Magnitude(pVWMesh->m_vBMin - pVWMesh->m_vBMax);

	return fWeaponSize;
}

bool RVisualMesh::IsDoubleWeapon()
{
	int sel_parts = 0;
	int sel_parts2 = 0;
	bool bCheck = false;//양손인가?
	bool bRender = false;

	GetMotionInfo(sel_parts,sel_parts2,bCheck,bRender);

	return bCheck;
}

void RVisualMesh::GetEnChantColor(DWORD* color)
{
	if(!color) return;

	if (CustomColor[0] || CustomColor[1])
	{
		color[0] = CustomColor[0];
		color[1] = CustomColor[1];
		return;
	}

	if(m_EnchantType==REnchantType_Fire) {
		color[0] = 0x4fff6666;
		color[1] = 0x0fff6666;
	}
	else if(m_EnchantType==REnchantType_Cold) {
		color[0] = 0x4f6666ff;
		color[1] = 0x0f6666ff;
	}
	else if(m_EnchantType==REnchantType_Lightning) {
		color[0] = 0x4f66ffff;
		color[1] = 0x0f66ffff;
	}
	else if(m_EnchantType==REnchantType_Poison) {
		color[0] = 0x4f66ff66;
		color[1] = 0x0f66ff66;
	}
	else {
		color[0] = 0x4fffffff;
		color[1] = 0x0fffffff;
	}
}

void RVisualMesh::DrawTracks(bool draw,RVisualMesh* pVWMesh,int mode,rmatrix& m) 
{
	if( pVWMesh == NULL )	return;

	if( m_bRenderMatrix ) return;

	if( mode < 0 || mode > 1 )	return;

	if( draw && m_bDrawTracksMotion[mode] ) {

		// 칼 종류에 따라서 길이가 틀리도록 적용해보기..
		// 이도류 , 카타나 , 대검.. 이도류의 휘어진 칼이 문제..

		rvector vmax,vmin,vcenter;
		rvector vpos[2];

		vmax = pVWMesh->m_vBMax;
		vmin = pVWMesh->m_vBMin;

		vcenter = (vmax-vmin)/2;

		vpos[0] = vmin + vcenter;
		vpos[1] = vmin + vcenter;

		vpos[0].y = vmax.y;
		vpos[1].y = vmin.y + 35.f; // 손잡이 만큼 뺴기..

		D3DXVec3TransformCoord(&vpos[0],&vpos[0],&m);
		D3DXVec3TransformCoord(&vpos[1],&vpos[1],&m);

		
		static RLVertex pVert[2];
		static DWORD color[2];

		GetEnChantColor(color);

		pVert[0].p = vpos[0];
		pVert[0].color = color[0];

		pVert[1].p = vpos[1];
		pVert[1].color = color[1];

//		static RWeaponTracks pTracks;
//		무기,몬스터등 여러 VisualMesh 가있으니가 사용시점에서 생성..

		if(!m_pTracks[mode]) {
			m_pTracks[mode] = new RWeaponTracks;
			m_pTracks[mode]->Create(10);//궤적길이..
		}

		m_pTracks[mode]->m_vSwordPos[0] = vpos[0];
		m_pTracks[mode]->m_vSwordPos[1] = vpos[1];

//		m_pTracks[mode]->AddVertexSpline( pVert );
		m_pTracks[mode]->AddVertex(pVert);
		m_pTracks[mode]->Render();

//		draw_box(&m,vmax,vmin,0xff00ff00);

	} else {
		if(m_pTracks[mode]) {
			m_pTracks[mode]->Clear();
		}
	}
}


rvector RVisualMesh::GetCurrentWeaponPosition(bool right)
{
	rmatrix m = GetCurrentWeaponPositionMatrix(right);

	rvector v;

	v.x = m._41;
	v.y = m._42;
	v.z = m._43;

	return v;
}

rmatrix RVisualMesh::GetCurrentWeaponPositionMatrix(bool right)
{
	int sel_parts = 0;
	int sel_parts2 = 0;
	bool bCheck = false;//양손인가?
	bool bRender = false;

	GetMotionInfo(sel_parts,sel_parts2,bCheck,bRender);

	rmatrix m;

	if(right && bCheck)
		m = m_WeaponPartInfo[sel_parts2].mat * m_WorldMat;
	else
		m = m_WeaponPartInfo[sel_parts].mat * m_WorldMat;

	return m;
}

// 렌더링 하기전에 사용할경우

bool RVisualMesh::CalcBox()
{
	_BP("RVisualMesh::CalcBox");

	if(m_pMesh) {
		m_pMesh->SetVisualMesh(this);

		if(m_pMesh->GetPhysiqueMesh()) {
			// 노드단위
			if(m_bIsRenderFirst) {
				m_pMesh->CalcBoxNode(&m_WorldMat);
				m_bIsRenderFirst = false;

				m_vBMax = m_pMesh->m_vBBMaxNodeMatrix * 1.2f;// 20% 정도 더 크게
				m_vBMin = m_pMesh->m_vBBMinNodeMatrix * 1.2f;
			}
		}
		else {
			// 점단위
			if(m_bIsRenderFirst) 
			{
				m_pMesh->CalcBox(&m_WorldMat);
				m_bIsRenderFirst = false;

				m_vBMax = m_pMesh->m_vBBMax;		
				m_vBMin = m_pMesh->m_vBBMin;
			}
		}

		_EP("RVisualMesh::CalcBox");
		return true;
	}
	_EP("RVisualMesh::CalcBox");
	return false;
}

void RVisualMesh::SetPos(rvector pos,rvector dir,rvector up) {

	m_vPos	= pos;
	m_vDir	= dir;
	m_vUp	= up;

//	MakeWorldMatrix(&m_WorldMat,m_vPos,m_vDir,m_vUp);
}

void RVisualMesh::SetWorldMatrix(rmatrix& mat) {
	m_WorldMat = mat;
}

/////////////////////////////////////////////////////////////////////////

void RVisualMesh::AddWeapon(RWeaponMotionType type,RMesh* pMesh,RAnimation* pAni)
{
	if((type < eq_weapon_etc)||(type > eq_weapon_end-1)) {
		mlog("RVisualMesh::AddWeapon 니가 RVisualMesh 를 깨먹는구나~~~\n");
	}

	RemoveWeapon(type);

	RVisualMesh* pVMesh = new RVisualMesh;
	pVMesh->Create(pMesh);

	if(pAni) {
		pVMesh->SetAnimation( ani_mode_upper , pAni );
		pVMesh->Stop();//원하는 타이밍에 play..
	}

	m_WeaponVisualMesh[type] = pVMesh;
}

void RVisualMesh::RemoveWeapon(RWeaponMotionType type)
{
	if(m_WeaponVisualMesh[type]) {
		delete m_WeaponVisualMesh[type];
		m_WeaponVisualMesh[type] = NULL;
	}
}

void RVisualMesh::RemoveAllWeapon()
{
	for(int i=0;i<eq_weapon_end;i++) {
		if(m_WeaponVisualMesh[i]) {
			delete m_WeaponVisualMesh[i];
			m_WeaponVisualMesh[i] = NULL;
		}
	}
}

void RVisualMesh::ClearParts() {

	if(m_pTMesh==NULL) return;

	if (IsDynamicResourceLoad())
	{
		for (int i = 0; i < eq_parts_end; i++) {
			if (!m_pTMesh[i])
				continue;

			switch (i)
			{
			case eq_parts_head:
			case eq_parts_chest:
			case eq_parts_hands:
			case eq_parts_legs:
			case eq_parts_feet:
				GetMeshManager()->Release(m_pTMesh[i]);
			};

			m_pTMesh[i] = nullptr;
		}
	}
}

void RVisualMesh::SetParts(RMeshPartsType parts, RMeshNode* pMN) {

	if(parts < 0 && parts >= eq_parts_end ) 
		return;

	if(m_pTMesh==NULL) return;

//	if(m_pTMesh[parts] != pMN) 
	{
		m_pTMesh[parts] = pMN;
		if(m_pMesh)	m_pMesh->ConnectPhysiqueParent(pMN);
	}
}

void RVisualMesh::SetParts(RMeshPartsType parts, const char* name)
{
	if(parts < 0 && parts >= eq_parts_end ) 
		return;

	if(m_pTMesh==NULL) return;

	if(!m_pMesh) return;

	if (IsDynamicResourceLoad())
	{
		GetMeshManager()->GetAsync(m_pMesh->GetName(), name, this, [this, saved_name = std::string(name), parts](RMeshNode *pNode)
		{
			RMeshNode *pPreviousNode = m_pTMesh[parts];

			if (pPreviousNode)
			{
				GetMeshManager()->Release(pPreviousNode);
			}

			if (!pNode)
				pNode = m_pMesh->GetPartsNode(saved_name.c_str());

			if (!pNode)
			{
				MLog("RVisualMesh::SetParts(): Failed to find parts %s\n", saved_name.c_str());
				return;
			}

			m_pTMesh[parts] = pNode;
			m_pMesh->ConnectPhysiqueParent(pNode);
		}
		);
	}
	else
	{
		RMeshNode* pNode = m_pMesh->GetPartsNode(name);

		if (pNode)
		{
			m_pTMesh[parts] = pNode;
			m_pMesh->ConnectPhysiqueParent(pNode);
		}
	}
}

RMeshNode* RVisualMesh::GetParts(RMeshPartsType parts) {

	if(parts < 0 || parts >= eq_parts_end ) 
		return NULL;

	if(m_pTMesh==NULL) return NULL;

	return m_pTMesh[parts];
}

void RVisualMesh::ClearFrame()
{
	for(int i=0;i<ani_mode_end;i++) {
		m_FrameInfo[i].ClearFrame();
	}
}

// 임시 이름으로 에니메이션 타잎 정해주기

enum apm_type {
	apm_attack1 = 1,// ani play mode -> 한번하고 idle 로 바뀜.
	apm_attack2,	// 특수공격들..
	apm_walk,		
	apm_run,		// loop
	apm_die,
};

void RVisualMesh::CheckAnimationType(RAnimation* pAniSet)
{
	if(!pAniSet) return;
/*
	if(pAniSet->NameCheck("attack")) {
		m_play_mode =  apm_attack1;
	}
	else if( pAniSet->NameCheck("run") ) {
		m_play_mode =  apm_attack1;
	}
*/
}

// 보류~~

bool RVisualMesh::SetBlendAnimation(RAnimation* pAniSet,float blend_time,bool b)
{
	return SetBlendAnimation(ani_mode_lower,pAniSet,blend_time,b);
}

bool RVisualMesh::SetBlendAnimation(const char* ani_name,float blend_time,bool b)
{
	return SetBlendAnimation(ani_mode_lower,ani_name,blend_time,b);
}

bool RVisualMesh::SetBlendAnimation(RAniMode animode,RAnimation* pAniSet,float blend_time,bool b)
{
	RAnimation* pAS[2];

	// 우선 위아래 모두 테스트

	AniFrameInfo* pInfo = GetFrameInfo(animode);

	if( pInfo->m_bBlendAniSet==false ) {

		pInfo->m_bBlendAniSet = true;
		pInfo->m_fMaxBlendTime = blend_time;
		pInfo->m_fCurrentBlendTime = 0.f;
	}
	else { // 이미블랜딩중이라면? 우선은 같이~

		pInfo->m_bBlendAniSet = true;
		pInfo->m_fMaxBlendTime = blend_time;
		pInfo->m_fCurrentBlendTime = 0.f;
	}

	pAS[0] = GetFrameInfo(ani_mode_lower)->m_pAniSet; // lower
	pAS[1] = GetFrameInfo(ani_mode_upper)->m_pAniSet; // upper

	if( SetAnimation( animode, pAniSet , b ) ) {

		GetFrameInfo( ani_mode_blend_lower )->m_pAniSet = pAS[0];
		GetFrameInfo( ani_mode_blend_upper )->m_pAniSet = pAS[1];

		return true;
	}

	return false;
}

bool RVisualMesh::SetBlendAnimation(RAniMode animode, const char* ani_name,float blend_time,bool b)
{
	RAnimation* pAS[2];

	// 우선 위아래 모두 테스트

	AniFrameInfo* pInfo = GetFrameInfo(animode);

	if( pInfo->m_bBlendAniSet==false ) {
		pInfo->m_bBlendAniSet = true;
		pInfo->m_fMaxBlendTime = blend_time;
		pInfo->m_fCurrentBlendTime = 0.f;
	}
	else { // 이미블랜딩중이라면? 우선은 같이~

		pInfo->m_bBlendAniSet = true;
		pInfo->m_fMaxBlendTime = blend_time;
		pInfo->m_fCurrentBlendTime = 0.f;
	}

	pAS[0] = GetFrameInfo( ani_mode_lower )->m_pAniSet; // lower
	pAS[1] = GetFrameInfo( ani_mode_upper )->m_pAniSet; // upper

	// 전부 옮겨야함

	if( SetAnimation( animode, ani_name , b ) ) {

		GetFrameInfo( ani_mode_blend_lower )->m_pAniSet = pAS[0];
		GetFrameInfo( ani_mode_blend_upper )->m_pAniSet = pAS[1];

		return true;
	}

	return false;
}

// 외부에서 직접 사용도 가능하도록..
bool RVisualMesh::SetAnimation(RAnimation* pAniSet,bool b)
{
	return SetAnimation(ani_mode_lower,pAniSet,b);
}

bool RVisualMesh::SetAnimation(const char* ani_name,bool b)
{
	return SetAnimation(ani_mode_lower,ani_name,b);
}

bool RVisualMesh::SetAnimation(RAniMode animode,RAnimation* pAniSet,bool b)
{
//	if(animode == ani_mode_lower)
	if(!pAniSet) 
		return false;

	_BP("VMesh::SetAnimation");

	if(pAniSet->GetAniNodeCount()==0) {
		//의미없다.. 빈 에니메이션
		return false;
	}

	AniFrameInfo* pInfo = GetFrameInfo(animode);

	if(pInfo==NULL) return false;

	bool bChange = false;
//	bool bChange = true;
	bool bSaveFrame = false;

	//강제..
	if(b) { 
		bChange = true;
	}
	else { 
		if( pInfo->m_pAniSet ) {
			if( pInfo->m_pAniSet != pAniSet) {//모델 포인터만 바뀔 수 도 있다..
				bChange = true;
			}
		}
		else {
			bChange = true;
		}
	}

	if(bChange) {

		// 프레임을 초기화하고
		// 다음 에니메이션과
		// 예약된 에니메이션을 초기화

		// 모션은 바뀌었으나 상태가 같다면 (무기만 바꾼경우 )이전 프레임 유지

		//이름이같고 모션 타입이 다르다면...

		if( pInfo->m_pAniSet && pAniSet) {
			if( strcmp( pInfo->m_pAniSet->GetName() , pAniSet->GetName() ) == 0 ) {
				if( pInfo->m_pAniSet->m_weapon_motion_type != pAniSet->m_weapon_motion_type) {
					bSaveFrame = true;
				}
			}
		}

		if(bSaveFrame) {

			float ff = 0.f;

			if( pInfo->m_pAniSet->GetMaxFrame())
				ff = pInfo->m_nFrame / (float) pInfo->m_pAniSet->GetMaxFrame();

			pInfo->m_nAddFrame = pAniSet->GetMaxFrame() * ff; 
		}
		else {
			pInfo->m_nAddFrame = 0;
		}

//		ClearFrame();
		pInfo->m_bChangeAnimation = true;
		pInfo->m_pAniSet = pAniSet;
//		CheckAnimationType(m_pAniSet[animode]);

//		자기 모델에 대한 에니메이션 테이블 작성

//		UpdateMotionTable();//<-------------------

		Play(animode);
	}

	_EP("VMesh::SetAnimation");

	return true;
}

void RVisualMesh::UpdateMotionTable()
{
	if(m_pMesh==NULL) return;

	int meshnode_cnt = m_pMesh->m_data_num;

	if( m_nAniNodeTableCnt != meshnode_cnt) {//모델이 바뀔일은 없지만..
		if(m_pAniNodeTable) {
			delete [] m_pAniNodeTable;
			m_pAniNodeTable = NULL;
		}
	}

	if(m_pAniNodeTable == NULL) {
		m_pAniNodeTable = new RAnimationNode*[meshnode_cnt];
		m_nAniNodeTableCnt = meshnode_cnt;
		memset(m_pAniNodeTable,0,sizeof(RAnimationNode*)*meshnode_cnt);
	}
	else {
		memset(m_pAniNodeTable,0,sizeof(RAnimationNode*)*m_nAniNodeTableCnt);
	}

	// 연결..

	RAnimation* pAL = GetFrameInfo(ani_mode_lower)->m_pAniSet;
	RAnimation* pAU = GetFrameInfo(ani_mode_upper)->m_pAniSet;

	if(!pAL && !pAU) 
		return;

	// 하반신등록

	RAnimationNode* pANode = NULL;
	RMeshNode* pM = NULL;

	int pid = -1;

	if(pAL) {

		int node_cnt = pAL->GetAniNodeCount();

		for(int i=0;i<node_cnt;i++) {

			pANode = pAL->GetAniNode(i);

//			pid = m_pMesh->FindMeshId(pANode);
			pid = m_pMesh->_FindMeshId(pANode->GetName());

			if(pid != -1) {

				RMeshNode* pM = m_pMesh->m_data[pid];

				if(pM) {
//					if(pM->m_CutPartsType == cut_parts_lower_body) 
					m_pAniNodeTable[pid] = pANode;
				}
			}
		}
	}

	if(pAU) {

		int node_cnt = pAU->GetAniNodeCount();

		for(int i=0;i<node_cnt;i++) {

			pANode = pAU->GetAniNode(i);

//			pid = m_pMesh->FindMeshId(pANode);
			pid = m_pMesh->_FindMeshId(pANode->GetName());

			if(pid != -1) {

				RMeshNode* pM = m_pMesh->m_data[pid];

				if(pM) {
					if( pM->m_CutPartsType == cut_parts_upper_body )
						m_pAniNodeTable[pid] = pANode;
				}
			}
		}
	}

}

// 캐릭터 상위에서 자신의 상태와 들고 있는 무기에 따라서 에니메이션 연결해주기

bool RVisualMesh::SetAnimation(RAniMode animode, const char* ani_name,bool b)
{
	if(!m_pMesh)
		return false;

	AniFrameInfo* pAniLow = GetFrameInfo(ani_mode_lower);
	AniFrameInfo* pAniUp  = GetFrameInfo(ani_mode_upper);


	if(!ani_name) {
		if(animode==ani_mode_upper) {
			pAniUp->m_pAniSet = NULL;
//			m_pAniSet[ani_mode_upper] = NULL;
		}
		return false;
	}

	if(ani_name[0]==0) {//이름이 없을 경우
		if(animode==ani_mode_upper){
			pAniUp->m_pAniSet = NULL;
//			m_pAniSet[ani_mode_upper] = NULL;
		}
		return false;
	}

	int wtype = -1;

	if(m_SelectWeaponMotionType!=eq_weapon_etc)//기본값이면 그냥 -1
		wtype = GetSetectedWeaponMotionID();

	if(animode == ani_mode_upper) {
		if( pAniLow->m_pAniSet ) {
			// load,reload 는 상관안함..
			// 공격의 경우에는 idle 상태나 칼이나 수류탄인 경우만 섞어준다..
			if(strcmp(ani_name,"attackS")==0 ) {

				if(strcmp( pAniLow->m_pAniSet->GetName() ,"idle") != 0) {//idle 는 무조건..그외에는..

					if( (wtype != eq_wd_katana) && 
						(wtype != eq_wd_grenade) && 
						(wtype != eq_ws_dagger) && 
//						(wtype != eq_wd_rifle) &&
//						(wtype != eq_wd_rlauncher) &&
//						(wtype != eq_ws_smg) &&
//						(wtype != eq_wd_smg) &&
//						(wtype != eq_ws_pistol) &&
//						(wtype != eq_wd_pistol) &&
						(wtype != eq_wd_shotgun)
						)
					{
						pAniUp->m_pAniSet = NULL;
						return true;
					}

				}
			}
		}
	}

	RAnimation* pAniSet = m_pMesh->m_ani_mgr.GetAnimation(ani_name,wtype);

	return SetAnimation(animode,pAniSet,b);
}

bool RVisualMesh::SetNextAnimation(RAnimation* pAniSet)
{
	return SetNextAnimation(ani_mode_lower,pAniSet);
}

bool RVisualMesh::SetNextAnimation(const char* ani_name)
{
	return SetNextAnimation(ani_mode_lower,ani_name);
}

bool RVisualMesh::SetNextAnimation(RAniMode animode,RAnimation* pAniSet)
{
	if(!pAniSet) return false;

	AniFrameInfo* pInfo = GetFrameInfo(animode);

	if( pInfo->m_pAniSet ) {
		if( strstr( pInfo->m_pAniSet->GetName(),pAniSet->GetName()) ) {
			return false;
		}
	}

	pInfo->m_pAniSetNext = pAniSet;

//	CheckAnimationType(pInfo->m_pAniSet);

	Play(animode);

	return true;
}

bool RVisualMesh::SetNextAnimation(RAniMode animode, const char* ani_name)
{
	if(!m_pMesh)
		return false;

	int wtype = -1;

	if(m_SelectWeaponMotionType != eq_weapon_etc)//기본값이면 그냥 -1
		wtype = GetSetectedWeaponMotionID();

	RAnimation* pAniSet = m_pMesh->m_ani_mgr.GetAnimation(ani_name,wtype);

	return SetNextAnimation(animode,pAniSet);
}

bool RVisualMesh::SetReserveAnimation(RAnimation* pAniSet,int tick)
{
	return SetReserveAnimation(ani_mode_lower,pAniSet,tick);
}

bool RVisualMesh::SetReserveAnimation(const char* ani_name,int tick)
{
	return SetReserveAnimation(ani_mode_lower,ani_name,tick);
}

bool RVisualMesh::SetReserveAnimation(RAniMode animode,RAnimation* pAniSet,int tick)
{
	if(!pAniSet) return false;

	AniFrameInfo* pInfo = GetFrameInfo(animode);

	if(pInfo==NULL) return false;

	pInfo->m_nReserveTime = GetGlobalTimeMS() + tick;
	pInfo->m_pAniSetReserve = pAniSet;

//	CheckAnimationType(m_pAniSet[animode]);

	Play(animode);

	return true;
}

bool RVisualMesh::SetReserveAnimation(RAniMode animode, const char* ani_name,int tick)
{
	if(!m_pMesh)
		return false;

	int wtype = -1;

	if(m_SelectWeaponMotionType!=eq_weapon_etc)//기본값이면 그냥 -1
		wtype = GetSetectedWeaponMotionID();

	RAnimation* pAniSet = m_pMesh->m_ani_mgr.GetAnimation(ani_name,wtype);

	return SetReserveAnimation(animode,pAniSet,tick);
}

#define AddText(s)		 { str.Add(#s,false); str.Add(" :",false); str.Add(s);}
#define AddTextEnum(s,e) { str.Add(#s,false); str.Add(" :",false); str.Add(#e);}

void RVisualMesh::OutputDebugString_CharacterState()
{
	return;

	RDebugStr str;

	str.Add("//////////////////   visual mesh   ////////////////////" );

	AddText(m_id);
//	AddText(m_fSpeed[0]);
//	AddText(m_fSpeed[1]);
	AddText(m_nAnimationState);

//	AddText(m_isOncePlayDone[ani_mode_lower]);
//	AddText(m_isOncePlayDone[ani_mode_upper]);

//	AddText(m_isPlayDone[ani_mode_lower]);
//	AddText(m_isPlayDone[ani_mode_upper]);

//	AddText(m_nFrame[ani_mode_lower]);
//	AddText(m_nFrame[ani_mode_upper]);

//	if(m_pAniSet[ani_mode_lower]) {
//		AddText(m_pAniSet[ani_mode_lower]->m_filename);
//	}

//	if(m_pAniSet[ani_mode_upper]) {
//		AddText(m_pAniSet[ani_mode_upper]->m_filename);
//	}

	str.PrintLog();
}

void RVisualMesh::ClearAnimation()
{
	m_pMesh = NULL;	

	Stop();
}

void RVisualMesh::SetBaseParts(RMeshPartsType parts) {

	if(parts < 0 && parts >= eq_parts_end ) 
		return;

	if(m_pTMesh==NULL) return;

	m_pTMesh[parts] = NULL;
}

rvector	RVisualMesh::GetBipRootPos(int frame)
{
	rvector v = rvector(0.f,0.f,0.f);

	RAnimation* pAni = GetFrameInfo(ani_mode_lower)->m_pAniSet;

	if(pAni) {
		if(pAni->GetBipRootNode()) {
			v = pAni->GetBipRootNode()->GetPosValue(frame);

			if(m_isScale) v = v * m_ScaleMat;
		}
	}

	return v;
}

rvector	RVisualMesh::GetFootPosition()
{
	rvector v = rvector(0.f,0.f,0.f);

	AniFrameInfo* pAniLow = GetFrameInfo( ani_mode_lower );
	AniFrameInfo* pAniUp  = GetFrameInfo( ani_mode_upper );

	if( pAniLow == NULL || pAniUp==NULL )
		return v;

	if(m_pMesh) {

		m_pMesh->SetAnimation( pAniLow->m_pAniSet,pAniUp->m_pAniSet );
		m_pMesh->SetFrame( pAniLow->m_nFrame,pAniUp->m_nFrame );
		m_pMesh->SetMeshVis(m_fVis);
		m_pMesh->SetVisualMesh(this);

		m_pMesh->RenderFrame();

		RMeshNode* pNode = NULL;

		pNode = m_pMesh->FindNode(eq_parts_pos_info_LFoot);

		if(pNode) { 

			v.x = pNode->m_mat_result._41;
			v.y = pNode->m_mat_result._42;
			v.z = pNode->m_mat_result._43;

			if(m_isScale) v = v * m_ScaleMat;

		}

		pNode = m_pMesh->FindNode(eq_parts_pos_info_RFoot);

		if(pNode) { 

			v.x += pNode->m_mat_result._41;
			v.y += pNode->m_mat_result._42;
			v.z += pNode->m_mat_result._43;

			v *= .5f;
			v.y-=12.f;	// 신발 높이 ;)

			if(m_isScale) v = v * m_ScaleMat;
		}
	}

	return v;
}

void RVisualMesh::GetBipTypeMatrix(rmatrix *mat,RMeshPartsPosInfoType type) 
{
	if(m_pBipMatrix) 
	{
		if(m_isScale) {
			*mat = m_pBipMatrix[type]* m_ScaleMat * m_WorldMat;
		}
		else  {
			*mat = m_pBipMatrix[type] * m_WorldMat;
		}
	}
}

void RVisualMesh::GetRFootMatrix(rmatrix *mat) { GetBipTypeMatrix(mat,eq_parts_pos_info_RFoot); }
void RVisualMesh::GetLFootMatrix(rmatrix *mat) { GetBipTypeMatrix(mat,eq_parts_pos_info_LFoot); }
void RVisualMesh::GetHeadMatrix(rmatrix *mat)  { GetBipTypeMatrix(mat,eq_parts_pos_info_Head);  }
void RVisualMesh::GetRootMatrix(rmatrix *mat)  { GetBipTypeMatrix(mat,eq_parts_pos_info_Root);  }

rvector RVisualMesh::GetBipTypePosition(RMeshPartsPosInfoType type)
{
	rvector rv;
	rmatrix m;
	GetBipTypeMatrix(&m,type);

	rv.x = m._41;
	rv.y = m._42;
	rv.z = m._43;

	return rv;
}

rvector	RVisualMesh::GetRFootPosition() 
{
	rvector rv;
	rmatrix m;
	GetRFootMatrix(&m);

	rv.x = m._41;
	rv.y = m._42;
	rv.z = m._43;

	return rv;
}

rvector RVisualMesh::GetLFootPosition() 
{
	rvector rv;
	rmatrix m;
	GetLFootMatrix(&m);

	rv.x = m._41;
	rv.y = m._42;
	rv.z = m._43;

	return rv;
}

rvector	RVisualMesh::GetHeadPosition() 
{
	rvector rv;
	rmatrix m;
	GetHeadMatrix(&m);

	rv.x = m._41;
	rv.y = m._42;
	rv.z = m._43;

	// 위치를 좀 조정해준다
	rvector root=GetRootPosition()-rv;
	Normalize(root);

	return rv;// +10.f * root;
}

rvector	RVisualMesh::GetRootPosition() 
{
	rvector rv;
	rmatrix m;
	GetRootMatrix(&m);

	rv.x = m._41;
	rv.y = m._42;
	rv.z = m._43;

	return rv;
}

D3DXQUATERNION RVisualMesh::GetBipRootRot(int frame)
{
	D3DXQUATERNION q = D3DXQUATERNION( 0.f,0.f,0.f,0.f );

	RAnimation* pAni = GetFrameInfo(ani_mode_lower)->m_pAniSet;

	if( pAni ) {
		if( pAni->GetBipRootNode() ) {
			q = pAni->GetBipRootNode()->GetRotValue(frame);
		}
	}

	return q;
}

rmatrix	RVisualMesh::GetBipRootMat(int frame)
{
	rmatrix m;

	D3DXQUATERNION q = GetBipRootRot(frame);
	D3DXVECTOR3 v = GetBipRootPos(frame);

	D3DXMatrixRotationQuaternion(&m,&q);

	m._41 = v.x;
	m._42 = v.y;
	m._43 = v.z;

	return m;
}

void RVisualMesh::SetSpeed(float s,float s_up)
{
	GetFrameInfo(ani_mode_lower)->m_fSpeed = s;
	GetFrameInfo(ani_mode_upper)->m_fSpeed = s_up;
}

bool RVisualMesh::isOncePlayDone(RAniMode amode) 
{
	AniFrameInfo* pInfo = GetFrameInfo(amode);
	if(pInfo)
		return pInfo->m_isOncePlayDone;
	return false;
}

void RVisualMesh::SetScale(const rvector& v)
{
	m_vScale = v;
	D3DXMatrixScaling(&m_ScaleMat,v.x,v.y,v.z);
	m_isScale = true;
}

void RVisualMesh::ClearScale() 
{
	m_vScale = rvector(1.f,1.f,1.f);
	D3DXMatrixScaling(&m_ScaleMat,1.f,1.f,1.f);
	m_isScale = false;
}

void RVisualMesh::GetBBox(rvector& vMax,rvector& vMin)
{
	vMax = m_vBMax;
	vMin = m_vBMin;
}
/*
RWeaponType RVisualMesh::GetSelectWeaponType() 
{
	RVisualMesh* pVMesh = GetSelectWeaponVMesh();

	if(pVMesh) {
		if(	pVMesh->m_pMesh ) {
			return pVMesh->m_pMesh->GetMeshWeaponType();
		}
	}
	return z_weapon_etc;
}
*/
int RVisualMesh::GetSetectedWeaponMotionID() 
{
	return (int)m_SelectWeaponMotionType;
}

void RVisualMesh::SelectWeaponMotion(RWeaponMotionType type) 
{
	m_SelectWeaponMotionType = type;
}

RVisualMesh* RVisualMesh::GetSelectWeaponVMesh() 
{
	return m_WeaponVisualMesh[m_SelectWeaponMotionType];
}

void RVisualMesh::SetRotXYZ(rvector v) {
	m_vRotXYZ = v;
}

void RVisualMesh::SetLowPolyModel(RMesh* pMesh) {
	m_pLowPolyMesh = pMesh;
}

RMesh* RVisualMesh::GetLowPolyModle() {
	return m_pLowPolyMesh;
}

void RVisualMesh::SetDrawTracks(bool s) {
	m_bDrawTracks = s;
}

void RVisualMesh::SetCheckViewFrustum(bool b) {
	m_bCheckViewFrustum = b;
}

bool RVisualMesh::IsSelectWeaponGrenade() 
{
	if(m_SelectWeaponMotionType == eq_wd_grenade)
		return true;
	return false;
}

bool RVisualMesh::IsClothModel() 
{
	if(m_pTMesh==NULL) return false;

	if(m_pTMesh[eq_parts_chest]) {
		if(m_pTMesh[eq_parts_chest]->m_point_color_num)
			return true;
	}

	return false;
}

bool RVisualMesh::GetWeaponDummyMatrix(WeaponDummyType type,rmatrix* mat,bool bLeft)
{
	if(type > weapon_dummy_etc && type < weapon_dummy_end)
	{
		rmatrix m;

		if(bLeft) {
			m = m_WeaponDummyMatrix2[type];
		}
		else {
			m = m_WeaponDummyMatrix[type];
		}

		if(m_bIsNpc)//npc 의경우는 scale 처리와 worldmatrix 처리가 자동으로 안 되어있다..
		{
			if(m_isScale)
				m = m_ScaleMat * m;
			m = m * m_WorldMat;
		}

		*mat = m;

		return true;
	}
	return false;
}

bool RVisualMesh::GetWeaponDummyPos(WeaponDummyType type,rvector* pos,bool bLeft)
{
	rmatrix m;

	if( GetWeaponDummyMatrix(type,&m,bLeft)) {
		pos->x = m._41;	
		pos->y = m._42;	
		pos->z = m._43;	
		return true;
	}
	return false;
}

void RVisualMesh::SetUVAnimation(float u,float v)
{
	m_fUAniValue = u;
	m_fVAniValue = v;
	m_bUVAni = true;
}

void RVisualMesh::ClearUvAnimation()
{
	m_fUAniValue = 0.f;
	m_fVAniValue = 0.f;
	m_bUVAni = false;
}

// 무기 모델에 달린 더미일 경우..
void RVisualMesh::UpdateWeaponDummyMatrix(RMeshNode* pMeshNode)
{
	if(!pMeshNode) return;

	if( pMeshNode->m_WeaponDummyType != weapon_dummy_etc) { // 무기에 달린 더미
		m_WeaponDummyMatrix[ pMeshNode->m_WeaponDummyType ] = pMeshNode->m_mat_result;
	}
}

// 무기 더미의 위치값을 보관..

void RVisualMesh::UpdateWeaponPartInfo(RMeshNode* pMeshNode)
{
	if(!pMeshNode) return;

	if( pMeshNode->m_PartsType != eq_parts_etc ) 
	{
		if(pMeshNode->m_pAnimationNode) 
		{
			m_WeaponPartInfo[ pMeshNode->m_PartsType ].mat = pMeshNode->m_mat_result;
			m_WeaponPartInfo[ pMeshNode->m_PartsType ].vis = pMeshNode->GetNodeVisValue();
			m_WeaponPartInfo[ pMeshNode->m_PartsType ].isUpdate = true;
		}
	}
}


AniFrameInfo* RVisualMesh::GetFrameInfo(RAniMode mode) 
{
	return &m_FrameInfo[mode];
}

void RVisualMesh::OnRestore()
{
	if(m_pCloth) 
		m_pCloth->OnRestore();

//	m_RenderQuery.Create( RGetDevice() );
}

void RVisualMesh::OnInvalidate()
{
	if(m_pCloth) 
		m_pCloth->OnInvalidate();

//	m_RenderQuery.Destroy();
}

////////////////////////////////////////////////////////////////

bool RVisualMesh::CreateCloth(RMeshNode* pMeshNode,float fAccel,int Numiter)
{
	DestroyCloth();// 이미 있다면 제거..

	m_pCloth = new RCharCloth;
	m_pCloth->create( m_pMesh, pMeshNode );

	m_pCloth->SetAccelationRatio( fAccel );
	m_pCloth->SetNumIteration( Numiter );

	pMeshNode->m_bClothMeshNodeSkip = false;//그린다..

	return true;
}

void RVisualMesh::DestroyCloth()
{
	DEL(m_pCloth);
}

void RVisualMesh::SetClothState(int state)
{
	if(m_pCloth) {
		m_pCloth->SetStatus( state );
	}
}

void RVisualMesh::UpdateForce(D3DXVECTOR3& force)
{
	if(m_pCloth) {
		m_pCloth->setForce(force);
	}
}

void RVisualMesh::UpdateCloth()
{	
	if(m_pCloth)
		m_pCloth->update( m_bClothGame , &m_WorldMat , m_fClothDist );
}

void RVisualMesh::RenderCloth()
{
	if(m_pCloth)
		m_pCloth->render();
}
/*
bool RVisualMesh::GetClothMeshNodeRender()
{
	if(m_pCloth&&m_pCloth->mpMeshNode)
		return m_pCloth->mpMeshNode->m_bClothMeshNodeSkip;
	return false;	
}

void RVisualMesh::SetClothMeshNodeRender(bool b)
{
	if(m_pCloth&&m_pCloth->mpMeshNode)
		m_pCloth->mpMeshNode->m_bClothMeshNodeSkip = b;
}
*/
void RVisualMesh::ChangeChestCloth(float fAccel,int Numiter  )
{
	RMeshNode* pMeshNode = GetParts( eq_parts_chest );

	// mesh node를 검색하면서 컬러값을 가진 노드를 걸러낸다

	if( pMeshNode == NULL ) // 파츠가 아닌 기본 모델에서 찾아본다..
	{
		int nMeshNode = m_pMesh->m_data_num;

		RMeshNode* pTMN;

		for( int i = 0 ; i < nMeshNode; ++i )
		{
			pTMN = m_pMesh->m_data[i];

			if( pTMN->m_PartsType == eq_parts_chest ) {	
				pMeshNode = pTMN;
				break;	
			}
		}
	}

	// 같으면 바꿀 필요 없다..

	if(m_pCloth && (m_pCloth->mpMeshNode == pMeshNode) )
		return;

	// 가슴 장비 모델이 버텍스 컬러 값을 가지고 있다면...

	if(pMeshNode && pMeshNode->m_point_color_num > 0 )
	{
		CreateCloth(pMeshNode,fAccel,Numiter);
	}
	else {
		DestroyCloth();
	}
}

void RVisualMesh::SetClothForce(D3DXVECTOR3& f)	
{ 
	if(m_pCloth) 
		m_pCloth->setForce( f ); 
}

void RVisualMesh::SetClothValue(bool bGame,float fDist)
{
	m_fClothDist = fDist;
	m_bClothGame = bGame;
}


_NAMESPACE_REALSPACE2_END
