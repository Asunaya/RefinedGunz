#ifndef _RVisualMesh_h
#define _RVisualMesh_h

#include "RVisualMeshUtil.h"

class RCharCloth;

_NAMESPACE_REALSPACE2_BEGIN

//////////////////////////////////////////////////
// frame 기능을 나눠주자..

class RVisualMesh;
class ROcclusionList;

class AniFrameInfo {
public:
	AniFrameInfo();

	void ClearFrame();

	void Frame(RAniMode amode,RVisualMesh* pVMesh);

	bool			m_isOncePlayDone;
	bool			m_isPlayDone;

	bool			m_bChangeAnimation;

	DWORD			m_nReserveTime;

	int				m_nFrame;
	int				m_nAddFrame;
	DWORD			m_save_time;
	DWORD			m_1frame_time;

	RAnimation*		m_pAniSet;
	RAnimation*		m_pAniSetNext;
	RAnimation*		m_pAniSetReserve;

	float			m_fSpeed;

	RAniSoundInfo	m_SoundInfo;

	// animation blend

	bool			m_bBlendAniSet;
	float			m_fMaxBlendTime;		// n초동안~
	float			m_fCurrentBlendTime;	// 0~1 비율
	DWORD			m_dwBackupBlendTime;

};

////////////////////////////////////////////////////////////////////////////
//

// 타이머는 전역타이머 받아서 쓰도록 수정하기..

class RFrameTime
{
public:
	RFrameTime() {

		m_bActive = false;
		m_bReturn = false;
		m_fMaxValue = 0.f;
		m_dwStartTime = 0;
		m_dwEndTime = 0;
		m_nType = 0;
		m_fCurValue = 0.f;
		m_dwReturnMaxTime = 0;

	}

	~RFrameTime() {

	}

public:
	void Start(float fMax,DWORD MaxTime,DWORD ReturnMaxTime);
	void Stop();

	void Update();

	float GetValue() {
		return m_fCurValue;
	}

public:

	int   m_nType;
	bool  m_bActive;
	bool  m_bReturn;
	float m_fMaxValue;
	float m_fCurValue;
	DWORD m_dwStartTime;
	DWORD m_dwEndTime;
	DWORD m_dwReturnMaxTime;
};

#define VISUAL_LIGHT_MAX 3

class RVisualLightMgr
{
public:
	RVisualLightMgr();

	int GetLightCount();

	void SetLight(int index,D3DLIGHT9* light,bool ShaderOnly);

	void UpdateLight();

	void Clone(RVisualMesh* pVMesh);

public:
	D3DLIGHT9 m_Light[VISUAL_LIGHT_MAX];
	int		  m_LightEnable[VISUAL_LIGHT_MAX];
};

class RQuery
{
public:
	RQuery() {

		m_nSpendingTime=NULL;
		m_DataCnt=NULL;
		m_Query=NULL;

	}

	virtual ~RQuery() {
		Destroy();
	}

	HRESULT Create( LPDIRECT3DDEVICE9 dev ) { 
		return dev->CreateQuery( D3DQUERYTYPE_OCCLUSION, &m_Query );
	}

	void Destroy() {

		if(m_Query) {
			m_Query->Release();
			m_Query = NULL;
		}
	}

	HRESULT Begin() {
		return m_Query->Issue( D3DISSUE_BEGIN );
	}

	HRESULT End() {
		return m_Query->Issue( D3DISSUE_END );
	}

	DWORD GetRenderCount() {

		if(m_Query==NULL) return 0;

		int nCnt = 0;
		m_DataCnt = 0;

		while( m_Query->GetData( (void*)&m_DataCnt,sizeof(DWORD),D3DGETDATA_FLUSH) == S_FALSE ) {
			m_nSpendingTime++;
			nCnt++;
			if(nCnt>5000) {			// 무한루프 돌면 안되니까..
				m_DataCnt = 1000;	// 대충 그려졌다고 본다..
				break;
			}
		}
		return m_DataCnt;
	}

	bool isNeedRender() {

		if(m_Query==NULL) return true; // 무조건 그려야 한다..

		DWORD Cnt = GetRenderCount();

		if( Cnt > 10 ) // 10 픽셀이상..
			return true;
		return false;
	}

public:

	int					m_nSpendingTime;
	DWORD				m_DataCnt;
	LPDIRECT3DQUERY9	m_Query;
};

// 캐릭터와 일반모델 일관성을 유지하면서 메모리 사용량만 줄이자~

class RVisualMesh {
public:
	RVisualMesh(); 
	~RVisualMesh();

	bool Create(RMesh* pMesh); 
	void Destroy();

	bool BBoxPickCheck(int x,int y);
	bool BBoxPickCheck(rvector& pos,rvector& dir);

	bool Pick(int x,int y,RPickInfo* pInfo);
	bool Pick(rvector* vInVec,RPickInfo* pInfo);
	bool Pick(rvector& pos,rvector& dir,RPickInfo* pInfo);
	bool Pick(rvector& pos,rvector& dir,rvector* v,float* t);

	int  GetMaxFrame(RAniMode amode);

	void Frame(RAniMode amode);
	void Frame();

	void Render(ROcclusionList* pOCCL);
	void Render(bool low = false ,bool render_buffer=false);
	void RenderWeapon();
	void RenderMatrix();

	void GetMotionInfo(int& sel_parts,int& sel_parts2,bool& bCheck,bool& bRender);

	void SetPos(rvector pos,rvector dir,rvector up);
	void SetWorldMatrix(rmatrix& mat);

	bool CalcBox();

	// weapon parts

	void SetParts(RMeshPartsType parts,RMeshNode* pMN);
	void SetParts(RMeshPartsType parts,char* name);

	void ClearParts();

	RMeshNode* GetParts(RMeshPartsType parts);

	void SetBaseParts(RMeshPartsType parts);

	// Animation

	bool SetAnimation(RAnimation* pAniSet,bool b=false);
	bool SetAnimation(char* ani_name,bool b=false);
	bool SetAnimation(RAniMode animode,RAnimation* pAniSet,bool b=false);
	bool SetAnimation(RAniMode animode,char* ani_name,bool b=false);

	bool SetBlendAnimation(RAnimation* pAniSet,float blend_time=0.5f,bool b=false);
	bool SetBlendAnimation(char* ani_name,float blend_time=0.5f,bool b=false);
	bool SetBlendAnimation(RAniMode animode,RAnimation* pAniSet,float blend_time=0.5f,bool b=false);
	bool SetBlendAnimation(RAniMode animode,char* ani_name,float blend_time=0.5f,bool b=false);

	bool SetNextAnimation(RAnimation* pAniSet);					
	bool SetNextAnimation(char* ani_name);
	bool SetNextAnimation(RAniMode animode,RAnimation* pAniSet);
	bool SetNextAnimation(RAniMode animode,char* ani_name);

	bool SetReserveAnimation(RAnimation* pAniSet,int tick);
	bool SetReserveAnimation(char* ani_name,int tick);
	bool SetReserveAnimation(RAniMode animode,RAnimation* pAniSet,int tick);
	bool SetReserveAnimation(RAniMode animode,char* ani_name,int tick);

	void ClearAnimation();
	void ClearFrame();

	void CheckAnimationType(RAnimation* pAniSet);

	// control

	void SetSpeed(float s,float s_up=4.8f); //4.8f ( 1배속 )

	void Play(RAniMode amode = ani_mode_lower);
	void Stop(RAniMode amode = ani_mode_lower);
	void Pause(RAniMode amode = ani_mode_lower);

	bool isOncePlayDone(RAniMode amode = ani_mode_lower);

	void SetScale(rvector& v);
	void ClearScale();

	// weapon

	void AddWeapon(RWeaponMotionType type,RMesh* pVMesh,RAnimation* pAni=NULL);
	void RemoveWeapon(RWeaponMotionType type);
	void RemoveAllWeapon();

	// weapon motion

//	RWeaponType		GetSelectWeaponType();
	int				GetSetectedWeaponMotionID();
	void			SelectWeaponMotion(RWeaponMotionType type);
	RVisualMesh*	GetSelectWeaponVMesh();

	void	SetRotXYZ(rvector v);

	void	SetLowPolyModel(RMesh* pMesh);
	RMesh*	GetLowPolyModle();

	void	SetDrawTracks(bool s);

	void	SetCheckViewFrustum(bool b);

	// world matrix 포함

	rmatrix GetCurrentWeaponPositionMatrix( bool right=false );
	rvector GetCurrentWeaponPosition( bool right=false );

	bool IsSelectWeaponGrenade();

	// 지금 설정된 하반신 에니메이션의 특정 프레임의 위치값을 얻는다.

	rvector			GetBipRootPos(int frame);
	D3DXQUATERNION	GetBipRootRot(int frame);
	rmatrix			GetBipRootMat(int frame);

	// 현재 실제 발의 위치를 리턴. 로컬 좌표계이다.

	rvector			GetFootPosition();

	// 한번은 그려지고 난 다음에 의미를 갖는다.

	void GetBipTypeMatrix(rmatrix *mat,RMeshPartsPosInfoType type);

	void GetHeadMatrix(rmatrix *mat);
	void GetRFootMatrix(rmatrix *mat);
	void GetLFootMatrix(rmatrix *mat);
	void GetRootMatrix(rmatrix *mat);		// 허리

	rvector GetBipTypePosition(RMeshPartsPosInfoType type);

	// 월드 좌표계이다
	rvector		GetHeadPosition();
	rvector		GetRFootPosition();
	rvector		GetLFootPosition();
	rvector		GetRootPosition();

	void	SetVisibility(float vis)	{ m_fVis = vis; }
	float	GetVisibility()				{ return m_fVis; }

	void SetDrawGrenade(bool b)	 { m_bDrawGrenade = b; }

	void OutputDebugString_CharacterState();

	bool IsClothModel();

	void DrawTracks(bool draw,RVisualMesh* pVWMesh,int mode,rmatrix& m);

	void DrawEnchant(RVisualMesh* pVWMesh,int mode,rmatrix& m);

	void DrawEnchantFire(RVisualMesh* pVWMesh,int mode,rmatrix& m);
	void DrawEnchantCold(RVisualMesh* pVWMesh,int mode,rmatrix& m);
	void DrawEnchantLighting(RVisualMesh* pVWMesh,int mode,rmatrix& m);
	void DrawEnchantPoison(RVisualMesh* pVWMesh,int mode,rmatrix& m);

	int	 GetLastWeaponTrackPos(rvector* pOutVec);

	AniFrameInfo* GetFrameInfo(RAniMode mode);

	void GetBBox(rvector& max,rvector& min);

	bool GetWeaponDummyMatrix(WeaponDummyType type,rmatrix* mat,bool bLeft);
	bool GetWeaponDummyPos(WeaponDummyType type,rvector* pos,bool bLeft);

	void SetNPCBlendColor(D3DCOLORVALUE color) { m_NPCBlendColor = color; }

	// test 용 함수 주의해서 사용하자..

	void SetSpRenderMode(int mode);
	void ClearPartInfo();

	void  GetWeaponPos(rvector* p,bool bLeft=false);
	float GetWeaponSize();
	bool  IsDoubleWeapon();

	void GetEnChantColor(DWORD* color);
	void SetEnChantType(REnchantType EnchantType) {
		m_EnchantType = EnchantType;
	}

	void SetCustomColor(DWORD a, DWORD b) {
		CustomColor[0] = a;
		CustomColor[1] = b;
	}

	void UpdateMotionTable();
	bool UpdateSpWeaponFire();

	void SetUVAnimation(float u,float v);
	void ClearUvAnimation();

	void UpdateWeaponDummyMatrix(RMeshNode* pMNode);// 무기 모델에 달린 더미일 경우..
	void UpdateWeaponPartInfo(RMeshNode* pMNode);	// 무기 더미의 위치를 보관한다..

	void OnRestore();
	void OnInvalidate();

public:

	// Cloth

	bool CreateCloth(RMeshNode* pMeshNode,float fAccel,int Numiter );
	void DestroyCloth();

	void ChangeChestCloth(float fAccel,int Numiter );

	void UpdateForce(D3DXVECTOR3& force);
	void SetClothState(int state);
	void UpdateCloth();
	void RenderCloth();

	bool isChestClothMesh()	{ return m_pCloth ? true:false; }

	void SetClothForce(D3DXVECTOR3& f);

//	bool GetClothMeshNodeRender();
//	void SetClothMeshNodeRender(bool b);

	void SetClothValue(bool bGame,float fDist);

private:
	DWORD CustomColor[2];

	float			m_fClothDist;
	bool			m_bClothGame;
	RCharCloth*		m_pCloth;

public:

	// Light Setting

	void SetLight(int index,D3DLIGHT9* light,bool ShaderOnly) {	m_LightMgr.SetLight(index,light,ShaderOnly);}
	void UpdateLight() { m_LightMgr.UpdateLight(); }

public:

//	RQuery			m_RenderQuery;

	bool			m_bIsNpc;
	bool			m_bIsCharacter;

	bool			m_bDrawTracksMotion[2];//모션에 따라서 좌우 그릴지 결정..
	bool			m_bDrawTracks;
	bool			m_isDrawWeaponState;
	bool			m_bDrawGrenade;

	bool			m_isScale;	// 우선은 한 모델에 대해서만... 
	rvector			m_vScale;	// 파츠마다 스케일 적용시는 테이블을 가지고 있어야함..

	rvector			m_vTargetPos;
	rvector			m_vRotXYZ;
	RFrameTime		m_FrameTime;

	rmatrix			m_RotMat;

	///////////////////////////////////////////////////////////////
	// toon 용 임시...디자이너 툴용

	rmatrix				m_ToonUVMat;
	LPDIRECT3DTEXTURE9	m_ToonTexture;	
	bool				m_bToonLighting;
	bool				m_bToonTextureRender;
	DWORD				m_bToonColor;

	///////////////////////////////////////////////////////////////

	rvector			m_vPos;
	rvector			m_vDir;
	rvector			m_vUp;
	rmatrix			m_WorldMat;
	rmatrix			m_ScaleMat;
	RMeshNode**		m_pTMesh;
	RMesh*			m_pMesh;
	RMesh*			m_pLowPolyMesh;

	RVisualLightMgr m_LightMgr;

	RWeaponTracks*	m_pTracks[2];

	REnchantType	m_EnchantType;

	int				m_id;
	float			m_fVis;

	int				m_nAnimationState;

	D3DCOLORVALUE	m_NPCBlendColor;

	AniFrameInfo	m_FrameInfo[ani_mode_end];

	D3DXVECTOR3		m_vBMax;
	D3DXVECTOR3		m_vBMin;

	RVisualMesh*	m_WeaponVisualMesh[eq_weapon_end];
	RPartsInfo		m_WeaponPartInfo[eq_parts_end];
	D3DXMATRIX		m_WeaponMatrixTemp;
	D3DXMATRIX		m_WeaponDummyMatrix[ weapon_dummy_end ];
	D3DXMATRIX		m_WeaponDummyMatrix2[ weapon_dummy_end ];//다른손..

	RWeaponMotionType	m_SelectWeaponMotionType;

	rmatrix*		m_pBipMatrix;

	rmatrix			m_UpperRotMat;

	RAnimationNode**	m_pAniNodeTable;
	int					m_nAniNodeTableCnt;

	ROcclusionList*		m_pTOCCL;

	float			m_fUAniValue;
	float			m_fVAniValue;
	bool			m_bUVAni;

	bool			m_bRenderMatrix;

	bool			m_bRenderInstantly;
	bool			m_bIsRender;
	bool			m_bIsRenderWeapon;
	bool			m_bIsRenderFirst;
	bool			m_bCheckViewFrustum;
	bool			m_bGrenadeRenderOnoff;
	bool			m_bGrenadeFire;
	bool			m_bAddGrenade;
	DWORD			m_GrenadeFireTime;
};

////////////////////////////////////////////////////////
// 일반 이펙트나 맵오브젝트 용과 캐릭터용을 구분하자..

class RCharacterVisualMesh : public RVisualMesh {
public:
	RCharacterVisualMesh() {

	}
	~RCharacterVisualMesh() {

	}

public:

};

_NAMESPACE_REALSPACE2_END

#endif//_RVisualMesh_h