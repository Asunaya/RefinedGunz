#ifndef _RAnimation_h
#define _RAnimation_h

#include "RAnimationFile.h"

_NAMESPACE_REALSPACE2_BEGIN

class RAnimation : public RBaseObject
{
public:
	RAnimation();
	~RAnimation();

	bool LoadAni(char* filename);

	void  SetFileName(char* name);
	char* GetFileName();

	AnimationType GetAnimationType();

	int GetMaxFrame();
	int GetAniNodeCount();

	RAnimationNode* GetAniNode(int i);
	RAnimationNode* GetBipRootNode();

	void SetWeaponMotionType(int wtype);
	int  GetWeaponMotionType();

	bool CheckWeaponMotionType(int wtype);

	RAnimationNode* GetNode(char* name);

	void SetLoadDone(bool b);
	bool IsLoadDone();

	// Sound Link

	void  ClearSoundFile(void);
	bool  SetSoundFileName(char* pSoundName);
	char* GetSoundFileName();
	bool  IsHaveSoundFile();

	bool  IsSoundRelatedToMap() { return m_bSoundRelatedToMap; }
	void  SetSoundRelatedToMap(bool bValue) { m_bSoundRelatedToMap = bValue; }

	AnimationLoopType	GetAnimationLoopType();
	void				SetAnimationLoopType(AnimationLoopType type);

public:

	RAnimationFile*		m_pAniData;

	char				m_filename[256];// idle, move,
	char				m_sound_name[256];

	bool				m_bIsHaveSound;
	bool				m_bSoundRelatedToMap;		// 맵과 연동되는 사운드인지 체크
	int					m_sID;

	int					m_NameID;

	int					m_weapon_motion_type;//어떤무기모델에 대한 에니메이션그룹인가를 표시..
	
	bool				m_isConnected;
	bool				m_isLoadDone;

private:

	AnimationLoopType	m_ani_loop_type;
};

_NAMESPACE_REALSPACE2_END

#endif//_RAnimation_h