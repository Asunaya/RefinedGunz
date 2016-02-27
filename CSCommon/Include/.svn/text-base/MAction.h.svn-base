#ifndef MACTION_H
#define MACTION_H


enum ACTION_TYPE {
	ACTION_TYPE_ONCE,	// 한번수행후 Turn에서 제외
	ACTION_TYPE_REPEAT	// 매Turn 반복
};


class MTurn;
class MActParam;
class MObject;


class MAction {
protected:
	int			m_nID;
	ACTION_TYPE	m_nType;

public:
	MAction()				{}
	virtual ~MAction()		{}

	int GetID()				{ return m_nID; }
	void SetID(int nID)		{ m_nID = nID; }
	ACTION_TYPE GetType()	{ return m_nType; }
	void SetType(ACTION_TYPE nType)	{ m_nType = nType; }

	virtual bool Run(int nTime, MObject* pActor, MActParam* pActParam) = 0;
};


enum ACTION_ID {
	ACTION_ID_WARNING,
	ACTION_ID_ATTACK,
	ACTION_ID_INVOKEDEFENSE,
	ACTION_ID_DEFENSE,
	ACTION_ID_SKILL
};


class MActionAttack : public MAction {
public:
	MActionAttack()		{ SetID(ACTION_ID_ATTACK);	SetType(ACTION_TYPE_REPEAT); }
	virtual bool Run(int nTime, MObject* pActor, MActParam* pActParam);
};

class MActionDefense : public MAction {
public:
	MActionDefense()	{ SetID(ACTION_ID_DEFENSE);	SetType(ACTION_TYPE_REPEAT); }
	virtual bool Run(int nTime, MObject* pActor, MActParam* pActParam);
};

class MActionSkill : public MAction {
public:
	MActionSkill()		{ SetID(ACTION_ID_SKILL);	SetType(ACTION_TYPE_REPEAT); }
	virtual bool Run(int nTime, MObject* pActor, MActParam* pActParam);
};


#endif
