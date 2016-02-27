#ifndef MStateStackMachine_H
#define MStateStackMachine_H

// 유한 상태 머신


#pragma warning(disable:4786)

#include <map>

using namespace std;

class MStateStackMachine;

// 상태
class MState{
friend class MStateStackMachine;
private:
	MStateStackMachine*	m_pSM;
protected:

protected:
	virtual bool OnCreate(void) = 0;
	virtual void OnDestroy(void) = 0;
	virtual bool OnRun(void) = 0;

	bool SetNextState(int nID);
public:
};

typedef map<int, MState*>		MSTATEMAP;
typedef MSTATEMAP::iterator		MSTATEMAPITOR;
typedef MSTATEMAP::value_type	MSTATEMAPVALTYPE;

// State Stack Machine
class MStateStackMachine{
protected:
	MSTATEMAP	m_States;
	MState*		m_pCurrState;
	int			m_nNextState;
public:
	MStateStackMachine(void);
	virtual ~MStateStackMachine(void);

	void AddState(int nID, MState* pState);
	void DelState(int nID);

	void AddActionOfState(int nActionID, int nStateID);

	bool SetState(int nID);		// State 전환
	bool SetNextState(int nID);	// 다음 OnRun() 이후 자동으로 State 전환

	bool Run(void);
};

#define NULLSTATE	0			// NULL State


#endif