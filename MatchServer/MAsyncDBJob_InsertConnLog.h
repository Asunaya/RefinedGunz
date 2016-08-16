#ifndef _MASYNCDBJOB_INSERTCONNLOG_H
#define _MASYNCDBJOB_INSERTCONNLOG_H


#include "MAsyncDBJob.h"



class MAsyncDBJob_InsertConnLog : public MAsyncJob {
protected:
	
protected:	// Input Argument
	unsigned long int		m_nAID;
	char 					m_szIP[64];
	string					m_strCountryCode3;
protected:	// Output Result

public:
	MAsyncDBJob_InsertConnLog()
		: MAsyncJob(MASYNCJOB_INSERTCONNLOG)
	{

	}
	virtual ~MAsyncDBJob_InsertConnLog()	{}

	bool Input(unsigned long int nAID, char* szIP, const string& strCountryCode3 );
	virtual void Run(void* pContext);
};





#endif