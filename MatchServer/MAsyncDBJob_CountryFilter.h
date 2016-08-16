#ifndef _MAsyncDBJob
#define _MAsyncDBJob

#include "MAsyncDBJob.h"
#include "MMatchServer.h"

class MAsyncDBJob_UpdateIPtoCountryList : public MAsyncJob
{
public :
	MAsyncDBJob_UpdateIPtoCountryList() : 
	   MAsyncJob( MASYNCJOB_UPDATEIPTOCOUNTRYLIST ) {}

	virtual void Run( void* pContext );
};


class MAsyncDBJob_UpdateBlockCountryCodeList : public MAsyncJob
{
public :
	MAsyncDBJob_UpdateBlockCountryCodeList() : 
	   MAsyncJob( MASYNCJOB_UPDATEBLOCKCOUNTRYCODELIST ) {}

	virtual void Run( void* pContext );
};


class MAsyncDBJob_UpdateCustomIPList : public MAsyncJob
{
public :
	MAsyncDBJob_UpdateCustomIPList() : 
	   MAsyncJob( MASYNCJOB_UPDATECUSTOMIPLIST ) {}

	virtual void Run( void* pContext );
};

#endif