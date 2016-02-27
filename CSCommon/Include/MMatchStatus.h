#ifndef _MMATCHSTATUS_H
#define _MMATCHSTATUS_H

class MMatchServer;

#define MATCHSTATUS_DUMP_LEN		4096

class MMatchStatus
{
private:
	bool			m_bCreated;
	MMatchServer*	m_pMatchServer;
protected:
	unsigned long int	m_nStartTime;
	unsigned long int	m_nTotalCommandQueueCount;
	unsigned long int	m_nTickCommandQueueCount;

#define MSTATUS_MAX_CMD_COUNT		100000
#define MSTATUS_MAX_DBQUERY_COUNT	60
#define MSTATUS_MAX_CMD_HISTORY		20

	unsigned long int	m_nCmdCount[MSTATUS_MAX_CMD_COUNT][3];
	unsigned long int	m_nDBQueryCount[MSTATUS_MAX_DBQUERY_COUNT][3];
	unsigned long int	m_nCmdHistory[MSTATUS_MAX_CMD_HISTORY];
	int					m_nHistoryCursor;
	int					m_nRunStatus;
	void AddCmdHistory(unsigned long int nCmdID);

	char				m_szDump[MATCHSTATUS_DUMP_LEN];
public:
	MMatchStatus();
	virtual ~MMatchStatus();
	static MMatchStatus* GetInstance();
	bool Create(MMatchServer* pMatchServer);
public:
	void SaveToLogFile();
	void AddCmdCount(unsigned long int nCmdCount)
	{
		m_nTickCommandQueueCount = nCmdCount;
		m_nTotalCommandQueueCount += m_nTickCommandQueueCount;
	}
	void AddCmd(unsigned long int nCmdID, int nCount = 1, unsigned long int nTime = 0)
	{
		AddCmdHistory(nCmdID);

		if (nCmdID >= MSTATUS_MAX_CMD_COUNT) return;
		m_nCmdCount[nCmdID][0] += nCount;
		m_nCmdCount[nCmdID][1] += nTime;
		m_nCmdCount[nCmdID][2] = nTime;
	}
	void AddDBQuery(unsigned long int nDBQueryID, unsigned long int nTime)
	{
		if (nDBQueryID >= MSTATUS_MAX_DBQUERY_COUNT) return;

		m_nDBQueryCount[nDBQueryID][0]++;
		m_nDBQueryCount[nDBQueryID][1] += nTime;

		m_nDBQueryCount[nDBQueryID][2] = nTime;
	}
	inline void SaveCmdHistory();
	void SetRunStatus(int value) { m_nRunStatus = value; }
	void SetLog(const char* szDump);
	inline void Dump();
};

inline MMatchStatus* MGetServerStatusSingleton() 
{
	return MMatchStatus::GetInstance();
}

inline void MMatchStatus::SaveCmdHistory()
{
	int t=0;
	for (int i = m_nHistoryCursor; i < MSTATUS_MAX_CMD_HISTORY; i++)
	{
		mlog("History(%d): %u\n", t++, m_nCmdHistory[i]);
	}
	for (int i = 0; i < m_nHistoryCursor; i++)
	{
		mlog("History(%d): %u\n", t++, m_nCmdHistory[i]);
	}

	mlog("RunStatus : %d\n", m_nRunStatus);
}

inline void MMatchStatus::Dump()
{
	SaveCmdHistory();

//	mlog("Dump: ");
//	mlog(m_szDump);
}
#endif