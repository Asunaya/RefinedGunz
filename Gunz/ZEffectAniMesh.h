#ifndef _ZEffectAniMesh_h
#define _ZEffectAniMesh_h

#include "ZEffectManager.h"
#include "RVisualMeshMgr.h"

#include "mempool.h"

class ZEffectAniMesh : public ZEffect{
protected:
	RealSpace2::RVisualMesh m_VMesh;

	rvector	m_Scale;
	rvector	m_Pos;
	rvector	m_Dir,m_DirOrg;
	rvector	m_Up;

	float	m_nStartTime;
	float	m_fRotateAngle;
	int		m_nAlignType;

	float	m_nStartAddTime;

	int		m_nLifeTime;

	MUID	m_uid;
	bool	m_bDelay;
	bool	m_isCheck;
	bool	m_bLoopType;
	
	rvector m_vBackupPos;

public:

	ZEffectAutoAddType	m_nAutoAddEffect;

public:
	ZEffectAniMesh(RMesh* pMesh, const rvector& Pos, const rvector& Dir);
	virtual bool Draw(unsigned long int nTime);

	void CheckCross(rvector& Dir,rvector& Up);

	RVisualMesh* GetVMesh();

	virtual void SetUpVector(rvector& v);

	void SetUid(MUID uid);
	MUID GetUID() { return m_uid; }
	void SetDelayPos(MUID id);
	void SetScale(rvector s);
	void SetRotationAngle(float a);
	void SetAlignType(int type);
	void SetStartTime(DWORD _time);
	void SetLifeTime(int nLifeTime) { m_nLifeTime = nLifeTime; }
	virtual rvector GetSortPos();

};


class ZEffectSlash : public ZEffectAniMesh , public CMemPoolSm<ZEffectSlash> {
public:
	ZEffectSlash(RMesh* pMesh, const rvector& Pos, const rvector& Dir);
};

class ZEffectDash : public ZEffectAniMesh , public CMemPoolSm<ZEffectDash> {
public:
	ZEffectDash(RMesh* pMesh, rvector& Pos, rvector& Dir,MUID uidTarget);
	virtual bool Draw(unsigned long int nTime);
};

class ZEffectLevelUp : public ZEffectAniMesh , public CMemPoolSm<ZEffectLevelUp> {
public:
	ZEffectLevelUp(RMesh* pMesh, rvector& Pos, rvector& Dir,rvector& AddPos,ZObject* pObj);
	virtual bool Draw(unsigned long int nTime);

	RMeshPartsPosInfoType m_type;
	rvector m_vAddPos;
};

class ZEffectPartsTypePos : public ZEffectAniMesh , public CMemPoolSm<ZEffectPartsTypePos> {
public:
	ZEffectPartsTypePos(RMesh* pMesh, rvector& Pos, rvector& Dir,rvector& AddPos,ZObject* pObj);
	virtual bool Draw(unsigned long int nTime);

	RMeshPartsPosInfoType m_type;
	rvector m_vAddPos;
};

class ZEffectWeaponEnchant : public ZEffectAniMesh , public CMemPoolSm<ZEffectWeaponEnchant> {
public:
	ZEffectWeaponEnchant(RMesh* pMesh, rvector& Pos, rvector& Dir, ZObject* pObj);
	virtual bool Draw(unsigned long int nTime);

};

class ZEffectIcon : public ZEffectAniMesh {
public:
	ZEffectIcon(RMesh* pMesh, ZObject* pObj);
	virtual bool Draw(unsigned long int nTime);
	RMeshPartsPosInfoType m_type;
};

class ZEffectShot : public ZEffectAniMesh , public CMemPoolSm<ZEffectShot> {
public:
	ZEffectShot(RMesh* pMesh, rvector& Pos, rvector& Dir,ZObject* pObj);
	virtual bool Draw(unsigned long int nTime);

	void SetStartTime(DWORD _time) {
		m_nStartAddTime = _time;
	}

	void SetIsLeftWeapon(bool b) {
		m_isLeftWeapon = b;
	}

public:
//	MUID		m_uid;
//	ZCharacter* m_pCharacter;

	float		m_nStartAddTime;
	bool		m_isMovingPos;
	MUID		m_uid;
	bool		m_isLeftWeapon;

};


class ZEffectBerserkerIconLoop : public ZEffectIcon, public CMemPoolSm<ZEffectBerserkerIconLoop> {
private:
	unsigned int m_nElapsedTime;
public:
	ZEffectBerserkerIconLoop(RMesh* pMesh, ZObject* pObj);
	virtual bool Draw(unsigned long int nTime);
};


#endif//_ZEffectAniMesh_h