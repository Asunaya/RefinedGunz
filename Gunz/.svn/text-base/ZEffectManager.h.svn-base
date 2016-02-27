#ifndef ZEFFECTMANAGER_H
#define ZEFFECTMANAGER_H

#include <list>
using namespace std;

#include "ZItem.h"

#include "RTypes.h"
#include "RMesh.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"

#include "ZEffectBase.h"
#include "ZEffectBulletMarkList.h"
#include "ZEffectLightFragmentList.h"
#include "ZEffectBillboardList.h"
#include "ZCharacter.h"


_USING_NAMESPACE_REALSPACE2

class ZEffectBillboardSource;
class ZEffectMeshSource;
class ZEffectWeaponEnchant;
class ZEffectAniMesh;

enum ZEffectAutoAddType {
	ZEffectAutoAddType_None = 0,
	ZEffectAutoAddType_Methor,
	ZEffectAutoAddType_End,
};

enum ZEffectDrawMode{
	ZEDM_ALPHAMAP=0,
	ZEDM_ADD,
	ZEDM_NONE,	// 모델이펙트도 이것
	ZEDM_COUNT,	// 마지막값이 개수가 된다.
};

enum ZEffectType { // 몇몇 effect 식별을 위해.. class 단위가 아님.. 같은 class 라도 서로다른..식별..
	ZET_NONE = 0,

	ZET_HEAL ,
	ZET_REPARE,

	ZET_END
};

enum ZCHARACTERICON {
	ZCI_ALLKILL	=0,
	ZCI_UNBELIEVABLE,
	ZCI_EXCELLENT,
	ZCI_FANTASTIC,
	ZCI_HEADSHOT,

	ZCI_END
};

class ZEffect
{
public:
	ZEffectDrawMode	m_nDrawMode;
	ZEffectType		m_nType;
public:
	ZEffect();
	virtual ~ZEffect(void);

	virtual bool CheckRenderAble(int level,float dist);
	virtual void CheckWaterSkip(int mode,float height);
	
	virtual bool Draw(unsigned long int nTime);

	virtual void Update() {}

	virtual bool IsDeleteTime() { return false; }

	ZEffectDrawMode	GetDrawMode(void);

	bool	isEffectType(ZEffectType type);
	void    SetEffectType(ZEffectType type);

	virtual rvector GetSortPos();

	void SetDistOption(float l0,float l1,float l2);

	bool  m_bWaterSkip;
	bool  m_bisRendered;	// for debug
	bool  m_bRender;
	float m_fHideDist[3];	// 3단계 옵션에 따른 그려지는 거리제한
	float m_fDist;
};

typedef list<ZEffect*>	ZEffectList;	// DrawMode에 따라 정렬되는 이펙트 리스트

enum ZTargetType{
	ZTT_CHARACTER,
	ZTT_OBJECT,
	ZTT_CHARACTER_GUARD,
	ZTT_NOTHING,
};

enum SlashEffectMotion {

	SEM_None = 0,

	SEM_ManSlash1,
	SEM_ManSlash2,
	SEM_ManSlash3,
	SEM_ManSlash4,
	SEM_ManSlash5,

	SEM_ManDoubleSlash1,
	SEM_ManDoubleSlash2,
	SEM_ManDoubleSlash3,
	SEM_ManDoubleSlash4,
	SEM_ManDoubleSlash5,

	SEM_ManGreatSwordSlash1,
	SEM_ManGreatSwordSlash2,
	SEM_ManGreatSwordSlash3,
	SEM_ManGreatSwordSlash4,
	SEM_ManGreatSwordSlash5,

	SEM_ManUppercut,

	SEM_WomanSlash1,
	SEM_WomanSlash2,
	SEM_WomanSlash3,
	SEM_WomanSlash4,
	SEM_WomanSlash5,

	SEM_WomanDoubleSlash1,
	SEM_WomanDoubleSlash2,
	SEM_WomanDoubleSlash3,
	SEM_WomanDoubleSlash4,
	SEM_WomanDoubleSlash5,

	SEM_WomanGreatSwordSlash1,
	SEM_WomanGreatSwordSlash2,
	SEM_WomanGreatSwordSlash3,
	SEM_WomanGreatSwordSlash4,
	SEM_WomanGreatSwordSlash5,

	SEM_WomanUppercut,

	SEM_End,
};

#define MAX_PARTICLE_ANI_LIST	100

class ZParticleManager{
public:
	ZParticleManager() {
		
	}
	virtual ~ZParticleManager() {
		
	}

public:
	
	ZEffectBillboardTexAniList*	m_TexAni[MAX_PARTICLE_ANI_LIST];
	
};

class ZEffectManager{
protected:
	/// Effect List
public:
	ZEffectList	m_Effects[ZEDM_COUNT];

protected:

	// Effect Billboard Sources
#define MUZZLESMOKE_COUNT	4
	ZEffectBillboardSource*	m_pEBSMuzzleSmoke[MUZZLESMOKE_COUNT];

#define MUZZLESMOKE_SHOTGUN_COUNT	2
	ZEffectBillboardSource*	m_pEBSMuzzleSmokeShotgun[MUZZLESMOKE_SHOTGUN_COUNT];

#define SMOKE_COUNT			4
	ZEffectBillboardSource*	m_pEBSSmokes[SMOKE_COUNT];
#define BLOOD_COUNT			5
	ZEffectBillboardSource*	m_pEBSBloods[BLOOD_COUNT];
#define BLOODMARK_COUNT		5
	ZEffectBillboardSource*	m_pEBSBloodMark[BLOODMARK_COUNT];
	ZEffectBillboardSource*	m_pEBSLightTracer;

	ZEffectBillboardSource*	m_pEBSBulletMark[3];

#define SHOTGUNFIRE_COUNT	4
	ZEffectBillboardSource*	m_pEBSShotGunFire[SHOTGUNFIRE_COUNT];
	
#define RIFLEFIRE_COUNT		4
	ZEffectBillboardSource*	m_pEBSRifleFire[RIFLEFIRE_COUNT][2];
							
#define GUNFIRE_COUNT		3
	ZEffectBillboardSource*	m_pEBSGunFire[GUNFIRE_COUNT][2];

	ZEffectBillboardSource*	m_pEBSDash[2];
	ZEffectBillboardSource*	m_pEBSRing[2];

	ZEffectBillboardSource*	m_pEBSLanding;

	ZEffectBillboardSource* m_pEBSWaterSplash;

	ZEffectBillboardSource* m_pEBSWorldItemEaten;

#define EMPTYCARTRIDGE_COUNT	2
	RMesh*	m_pMeshEmptyCartridges[EMPTYCARTRIDGE_COUNT];

	RMesh*	m_pSworddam[4];
	RMesh*	m_pSwordglaze;
	RMesh*	m_pRangeDamaged[6];

	RMesh*	m_pDashEffect;
	RMesh*	m_pGrenadeEffect;
	RMesh*	m_pGrenadeExpEffect;
	RMesh*	m_pRocketEffect;

	RMesh*	m_pFragment[6];

	RMesh*	m_pSwordDefenceEffect[3];
//	RMesh*	m_pRocketSmokeEffect;
	RMesh*	m_pSwordWaveEffect[2];
	RMesh*	m_pSwordEnchantEffect[4];
	RMesh*	m_pMagicDamageEffect;
	RMesh*	m_pMagicEffectWall[3];
	RMesh*	m_pSwordUppercutEffect;
	RMesh*	m_pSwordUppercutDamageEffect;
	RMesh*	m_pLevelUpEffect[2];
	RMesh*	m_pReBirthEffect;
	RMesh*	m_pFlameMG;
	RMesh*	m_pFlamePistol;
	RMesh*	m_pFlameRifle;
	RMesh*	m_pFlameShotgun;
	RMesh*	m_pHealEffect;
	RMesh*	m_pEatBoxEffect;
	RMesh*	m_pExpanseAmmoEffect;
	RMesh*	m_pRepireEffect;
	RMesh*	m_pWaterSplash;
	RMesh*	m_pWorldItemEaten;
	RMesh*	m_pDaggerUpper;
//	RMesh*	m_pLighteningEffect;
	RMesh*  m_pBlizzardEffect;

	RMesh*	m_pSwordFire;
	RMesh*	m_pSwordElec;
	RMesh*	m_pSwordCold;
	RMesh*	m_pSwordPoison;


#define BULLETONWALL_COUNT	2
	RMesh*	m_pBulletOnWallEffect[BULLETONWALL_COUNT];

	RMesh*	m_pCharacterIcons[ZCI_END];
	RMesh*	m_pCommandIcons[2];

	RMesh*	m_pLostConIcon;
	RMesh*	m_pChatIcon;

	RMesh*	m_pBerserkerEffect;

	ZEffectWeaponEnchant* m_pWeaponEnchant[ZC_ENCHANT_END];

	RMeshMgr* m_pEffectMeshMgr;

	ZEffectBulletMarkList		m_BulletMarkList;
	ZEffectLightFragmentList	m_LightFragments;

#define ROCKET_SMOKE_COUNT	1

#define BILLBOARDLISTS_COUNT	SMOKE_COUNT+ROCKET_SMOKE_COUNT
	ZEffectBillboardList		m_BillboardLists[BILLBOARDLISTS_COUNT];

	ZEffectShadowList			m_ShadowList;

#define BILLBOARDTEXANILIST_COUNT 5
	ZEffectBillboardTexAniList	m_BillBoardTexAniList[BILLBOARDTEXANILIST_COUNT];

	void Add(ZEffect* pNew);

public:

	int m__skip_cnt;
	int m__cnt;
	int m__rendered;

public:

	ZEffectManager(void);
	virtual ~ZEffectManager(void);

	bool Create();
	void CheckWaterSkip(int mode,float height);

	int GetEffectCount(int mode) { return (int)m_Effects[mode].size(); }

	void OnInvalidate();
	void OnRestore();

	void Clear();
	
	void Update(float fElapsed);
	void Draw(unsigned long int nTime);
	void Draw(unsigned long int nTime,int mode,float height);// 물 알파 떄문에 임시..

	int  DeleteSameType(ZEffectAniMesh* pNew);

	ZEffectWeaponEnchant* GetWeaponEnchant(ZC_ENCHANT type);

	RMeshMgr* GetEffectMeshMgr() { return m_pEffectMeshMgr; }

	bool RenderCheckEffectLevel();

	void AddLevelUpEffect(ZObject* pObj);
	void AddReBirthEffect(rvector& Target);

	void AddLandingEffect(rvector& Target, rvector& TargetNormal);
	void AddGrenadeEffect(rvector& Target, rvector& TargetNormal);
	void AddRocketEffect(rvector& Target, rvector& TargetNormal);
	void AddRocketSmokeEffect(rvector& Target);

	void AddMapSmokeSSEffect(rvector& Target,rvector& dir,rvector& acc,DWORD color,DWORD delay,float fLife,float fStartScale,float fEndScale);
	void AddMapSmokeSTEffect(rvector& Target,rvector& dir,rvector& acc,rvector& acc2,DWORD color,DWORD delay,float fLife,float fStartScale,float fEndScale);
	void AddMapSmokeTSEffect(rvector& Target,rvector& dir,rvector& acc,DWORD color,DWORD delay,float fLife,float fStartScale,float fEndScale);

	void AddSwordDefenceEffect(rvector& Target,rvector& vDir);
	void AddSwordWaveEffect(rvector& Target,DWORD start_time,ZObject* pObj);
	void AddSwordEnchantEffect(ZC_ENCHANT type,rvector& Target,DWORD start_time, float fScale=1.0f);
	void AddMagicEffect(rvector& Target,DWORD start_time, float fScale=1.0f);

//	void AddLighteningEffect(rvector& Target);
	void AddBlizzardEffect(rvector& Target,int nCnt);
	void AddMethorEffect(rvector& Target,int nCnt);

	void AddMagicEffectWall(int type,rvector& Target,rvector& vDir,DWORD start_time, float fScale=1.0f);

	void AddSwordUppercutDamageEffect(rvector& Target,MUID uidTarget, DWORD time = 0 );

	void AddEatBoxEffect(rvector& Target,ZObject* pObj);
	void AddHealEffect(rvector& Target,ZObject* pObj);
	void AddRepireEffect(rvector& Target,ZObject* pObj);
	void AddExpanseAmmoEffect(rvector& Target,ZObject* pObj);

	void AddBulletMark(rvector& Target, rvector& TargetNormal);
	void AddShotEffect(rvector* pSource,int size,rvector& Target, rvector& TargetNormal, ZTargetType nTargetType,MMatchWeaponType wtype,ZObject* pObj,bool bDrawFireEffects,bool bDrawTargetEffects );
	void AddShotgunEffect(rvector &pos,rvector &out,rvector &dir,ZObject* pObj);

	void AddTrackFire(rvector &pos);
	void AddTrackCold(rvector &pos);
	void AddTrackPoison(rvector &pos);
	void AddTrackMagic(rvector &pos);
	void AddTrackMethor(rvector &pos);

//	void AddEnchantFire(ZObject* pObj);
//	void AddEnchantCold(ZObject* pObj);
//	void AddEnchantPoison(ZObject* pObj);

	float GetEnchantDamageObjectSIze(ZObject* pObj);

	void AddEnchantFire2(ZObject* pObj);
	void AddEnchantCold2(ZObject* pObj);
	void AddEnchantPoison2(ZObject* pObj);

	void AddBloodEffect(rvector& Target, rvector& TargetNormal);
	void AddSlashEffect(rvector& Target, rvector& TargetNormal,int nType);
	void AddSlashEffectWall(rvector& Target, rvector& TargetNormal,int nType);
	void AddLightFragment(rvector Target,rvector TargetNormal);

	void AddDashEffect(rvector& Target,rvector& TargetNormal,ZObject* pObj);
	void AddSkillDashEffect(rvector& Target,rvector& TargetNormal,ZObject* pObj);

	void AddSmokeEffect( rvector& Target );
	void AddSmokeEffect( ZEffectBillboardSource* pEffectBillboardSource,rvector& Pos, rvector& Velocity, rvector& Accel, float fMinScale, float fMaxScale, float fLifeTime);
	void AddSmokeGrenadeEffect( rvector& Target );
	void AddGrenadeSmokeEffect( rvector& Target ,float min,float max,float time);

	void AddWaterSplashEffect( rvector& Target, rvector& Scale );
	void AddWorldItemEatenEffect( rvector& pos );

	void AddCharacterIcon(ZObject* pObj,int nIcon);
	void AddCommanderIcon(ZObject* pObj,int nTeam);
	void AddBerserkerIcon(ZObject* pObj);

	void AddChatIcon(ZObject* pObj);
	void AddLostConIcon(ZObject* pObj);

	void AddChargingEffect(ZObject* pObj);
	void AddChargedEffect(ZObject* pObj);

	void AddShadowEffect(rmatrix& m,DWORD _color);

	// 온게임넷 짱 아이콘
	void AddStarEffect(ZObject* pObj);

	// 일반적인 이펙트
	void Add(const char* szName,const rvector& pos, const rvector& dir,const MUID& uidOwner,int nLifeTime);
	void AddSp(const char* szName,int nCnt,const rvector& pos, const rvector& dir,const MUID& uidOwner);
	void AddPartsPosType(const char* szName,const MUID& uidOwner,RMeshPartsPosInfoType type,int nLifeTime);

};

// 이펙트 디테일 레벨..옵션

void SetEffectLevel(int level);
int	 GetEffectLevel();

#endif