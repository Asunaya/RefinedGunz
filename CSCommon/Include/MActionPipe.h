#ifndef MACTIONPIPE_H
#define MACTIONPIPE_H


#include <list>
using namespace std;
class MObject;
class MAction;
class MActParam;


enum PIPE_STATE {
	PIPE_STATE_STOPPED,
	PIPE_STATE_PENDING,
	PIPE_STATE_RUNNING	
};


class MActionPipe {
protected:
	int				m_nID;
	PIPE_STATE		m_nState;
	MObject*		m_pActor;

	list<MAction*>	m_ActionList;

public:
	MActionPipe();
	virtual ~MActionPipe(); 

	int GetID()				{ return m_nID; }
	void SetID(int nID)		{ m_nID = nID; }
	PIPE_STATE GetState()	{ return m_nState; }
	void SetState(PIPE_STATE nState) { m_nState = nState; }
	MObject* GetActor()		{ return m_pActor; }
	void SetActor(MObject* pActor)	{ m_pActor = pActor; }

	void AddAction(MAction* pAction);
	void DelAction(int nID);
	int GetActionCount()	{ return (int)m_ActionList.size(); }
	void ClearAction();
	list<MAction*>::iterator GetActionBeginItor()	{ return m_ActionList.begin(); }
	list<MAction*>::iterator GetActionEndItor()		{ return m_ActionList.end(); }

	virtual void Start()	{ SetState(PIPE_STATE_RUNNING); }
	virtual void Pause()	{ SetState(PIPE_STATE_PENDING); }
	virtual void Stop()		{ SetState(PIPE_STATE_STOPPED); }

	virtual bool CheckTurn(int nTime) = 0;
	virtual bool Run(int nTime, MObject* pActor, MActParam* pParam);
};


enum PIPE_ID {
	PIPE_ID_ACTIVE,
	PIPE_ID_DEFENSE,
	PIPE_ID_SKILL,
	PIPE_ID_BUFF
};


class MActivePipe : public MActionPipe {
protected:
	int		m_nPrimaryInterval;
	int		m_nLastPrimaryRunTime;

	int		m_nSecondaryInterval;
	int		m_nLastSecondaryRunTime;

protected:
	int GetPrimaryInterval()				{ return m_nPrimaryInterval; }
	void SetPrimaryInterval(int nInterval)	{ m_nPrimaryInterval = nInterval; }
	int GetLastPrimaryRunTime()				{ return m_nLastPrimaryRunTime; }
	void SetLastPrimaryRunTime(int nTime)	{ m_nLastPrimaryRunTime = nTime; }

	int GetSecondaryInterval()				{ return m_nSecondaryInterval; }
	void SetSecondaryInterval(int nInterval){ m_nSecondaryInterval = nInterval; }
	int GetLastSecondaryRunTime()			{ return m_nLastSecondaryRunTime; }
	void SetLastSecondaryRunTime(int nTime)	{ m_nLastSecondaryRunTime = nTime; }

public:
	MActivePipe();
	virtual ~MActivePipe();

	virtual bool CheckTurn(int nTime);
	virtual bool Run(int nTime, MObject* pActor, MActParam* pParam);

	void GiveDelayPenalty(int nTime, int nDelay);
};

class MDefensePipe : public MActionPipe {
public:
	MDefensePipe();
	virtual ~MDefensePipe();

	virtual bool CheckTurn(int nTime);
};

class MSkillPipe : public MActionPipe {
protected:
	int		m_nInterval;
	int		m_nLastRunTime;

protected:
	enum SKILLSTATE {
		SKILLSTATE_IDLE,
		SKILLSTATE_ENTER,
		SKILLSTATE_EXCUTE,
		SKILLSTATE_EXIT
	};

protected:

	MSkillPipe::SKILLSTATE	m_nSkillState;
	MSkillPipe::SKILLSTATE	m_nNextSkillState;
	int						m_nEnterDelay;
	int						m_nExitDelay;
	int						m_nTimeStamp;

	bool CheckStateReady(MSkillPipe::SKILLSTATE nState) {
		if ((m_nSkillState == nState) || (m_nNextSkillState != nState)) return false;
		return true;
	}
	void SetNextSkillState(MSkillPipe::SKILLSTATE nState)	{ m_nNextSkillState = nState; }
//	void UnifySkillState()			{ m_nSkillState = m_nNextSkillState; }
/*	
//	vector<int>				m_nStateTracer;
	bool CheckStateReady(MSkillPipe::SKILLSTATE nState) {
		if (m_nStateTracer.size() <= 0) return false;
		if (m_nStateTracer[0] == nState) return true;
		else return false;
	}
	void SetSkillState(MSkillPipe::SKILLSTATE nState)	{ 
		m_nSkillState = nState; 
		m_nStateTracer.push_back((int)nState);
	}
	void ClearStateTracer()			{ m_nStateTracer.clear(); }*/
	int GetTimeStamp()				{ return m_nTimeStamp; }
	void SetTimeStamp(int nTime)	{ m_nTimeStamp = nTime; }
	int GetEnterDelay()				{ return m_nEnterDelay; }
	void SetEnterDelay(int nDelay)	{ m_nEnterDelay = nDelay; }
	int GetExitDelay()				{ return m_nExitDelay; }
	void SetExitDelay(int nDelay)	{ m_nExitDelay = nDelay; }

protected:
	virtual bool OnEnter(int nTime, MObject* pActor, MActParam* pParam);	// On Enter State
	virtual bool OnExcute(int nTime, MObject* pActor, MActParam* pParam);	// On Excute
	virtual bool OnExit(int nTime, MObject* pActor, MActParam* pParam);		// On Exit State

public:
	MSkillPipe();
	virtual ~MSkillPipe();

	int GetInterval()				{ return m_nInterval; }
	void SetInterval(int nInterval)	{ m_nInterval = nInterval; }
	int GetLastRunTime()			{ return m_nLastRunTime; }
	void SetLastRunTime(int nTime)	{ m_nLastRunTime = nTime; }

	virtual void Start() { MActionPipe::Start(); SetNextSkillState(MSkillPipe::SKILLSTATE_ENTER); }
	virtual void Stop()	 { MActionPipe::Stop(); SetNextSkillState(MSkillPipe::SKILLSTATE_IDLE); }

	virtual bool CheckTurn(int nTime);
	virtual bool Run(int nTime, MObject* pActor, MActParam* pParam);
};

class MBuffPipe : public MActionPipe {
public:
	MBuffPipe();
	virtual ~MBuffPipe();

};


#endif