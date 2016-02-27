#ifndef MACTPARAM_H
#define MACTPARAM_H


#include "MUID.h"
#include <list>
using namespace std;

class MObject;


enum ACTPARAM_TYPE {
	ACTPARAM_TYPE_DAMAGE,
	ACTPARAM_TYPE_SKILL
};


class MActParam {
protected:
	ACTPARAM_TYPE	m_nType;
	int				m_nActionID;
	MUID			m_TargetUID;

public:
	MActParam(ACTPARAM_TYPE nType)		{ m_nType = nType; }
	virtual ~MActParam()						{}
	ACTPARAM_TYPE GetType()				{ return m_nType; }
	int GetActionID()					{ return m_nActionID; }
	void SetActionID(int nActionID)		{ m_nActionID = nActionID; }
	MObject* GetTarget();
	void SetTarget(MObject* pTarget);
};
typedef list<MActParam*>	MActParamList;


class MActParamDamage : public MActParam {
protected:
	int			m_nDamage;

public:
	MActParamDamage() : MActParam(ACTPARAM_TYPE_DAMAGE) {}
	virtual ~MActParamDamage()			{}
	int GetDamage()						{ return m_nDamage; }
	void SetDamage(int nDamage)			{ m_nDamage = nDamage; }
};

class MActParamSkill : public MActParam {
protected:
	int			m_nSkillID;

public:
	MActParamSkill() : MActParam(ACTPARAM_TYPE_SKILL) {}
	virtual ~MActParamSkill()			{}
	int GetSkillID()					{ return m_nSkillID; }
	void SetSkillID(int nSkillID)		{ m_nSkillID = nSkillID; }
};


#endif