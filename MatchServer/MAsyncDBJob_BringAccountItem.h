#ifndef _MASYNCDBJOB_BRINGACCOUNTITEM_H
#define _MASYNCDBJOB_BRINGACCOUNTITEM_H

#include "MAsyncDBJob.h"

class MAsyncDBJob_BringAccountItem : public MAsyncJob {
protected:
	MUID				m_uid;
protected:	// Input Argument
	int		m_nAID;
	int		m_nCID;
	int		m_nAIID;
protected:	// Output Result
	unsigned int		m_nNewCIID;
	unsigned long int	m_nNewItemID;
	bool				m_bIsRentItem;
	int					m_nRentMinutePeriodRemainder;

public:
	MAsyncDBJob_BringAccountItem(const MUID& uid)
		: MAsyncJob(MASYNCJOB_BRINGACCOUNTITEM)
	{
		m_uid = uid;

		m_nNewCIID = 0;
		m_nNewItemID = 0;
		m_bIsRentItem = false;
		m_nRentMinutePeriodRemainder = RENT_MINUTE_PERIOD_UNLIMITED;
	}
	virtual ~MAsyncDBJob_BringAccountItem()	{}

	bool Input(const int nAID, const int nCID, const int nAIID);
	virtual void Run(void* pContext);

	const MUID& GetUID()			{ return m_uid; }
	unsigned long int GetNewCIID()		{ return m_nNewCIID; }
	unsigned long int GetNewItemID()	{ return m_nNewItemID; }
	bool GetRentItem()					{ return m_bIsRentItem; }
	int GetRentMinutePeriodRemainder()	{ return m_nRentMinutePeriodRemainder; }
};





#endif