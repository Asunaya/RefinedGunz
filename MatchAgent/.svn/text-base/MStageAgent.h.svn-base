#ifndef MSTAGEAGENT_H
#define MSTAGEAGENT_H


class MAgentClient;


class MStageAgent {
protected:
	MUID			m_uidStage;
	MUIDRefCache	m_ObjUIDCaches;

	unsigned long	m_tmCreateTime;

public:
	bool Create(const MUID& uid);
	void Destroy();
	MUID GetUID()							{ return m_uidStage; }

	size_t GetObjCount()					{ return m_ObjUIDCaches.size(); }
	MUIDRefCache::iterator GetObjBegin()	{ return m_ObjUIDCaches.begin(); }
	MUIDRefCache::iterator GetObjEnd()		{ return m_ObjUIDCaches.end(); }

	void AddObject(const MUID& uid, const MAgentClient* pObj);
	void RemoveObject(const MUID& uid);
	bool IsExistClient(const MUID& uid);

	bool CheckDestroy();
	void Tick(unsigned long nClock);
};


class MStageAgents : public map<MUID, MStageAgent*> {
	MUID	m_uidGenerate;
public:
	MStageAgents()			{	m_uidGenerate = MUID(0,0);	}
	virtual ~MStageAgents()	{	}
	MUID UseUID()				{	m_uidGenerate.Increase();	return m_uidGenerate;	}
	void Insert(const MUID& uid, MStageAgent* pStage)	{	insert(value_type(uid, pStage));	}
};


#endif