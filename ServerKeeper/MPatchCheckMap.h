#ifndef _PATCH_CHECK_MAP
#define _PATCH_CHECK_MAP

#include <windows.h>
#include <map>
using namespace std;

#include "ServerKeeperConst.h"

class MPatchCheckMap : private map< JOB_STATE, JOB_STATE >
{
public :
	MPatchCheckMap();
	~MPatchCheckMap();

	void Init();
	
	void Check( const JOB_STATE nJob, const JOB_STATE nState );
	void Reset();
	
	bool IsPossibleAgentPatchDownload();
	bool IsPossibleServerPatchDownload();
	bool IsPossibleReset();
	bool IsJobSuccess( const JOB_STATE nJob );
	bool IsServerPrepareComplete();
	bool IsServerPatchComplete();
	bool IsAgentPrepareComplete();
	bool IsServerPatchDownloadComplete();
	bool IsAgentPatchDownloadComplete();
	
	const JOB_STATE GetLastJob()	{ return m_nLastJob; }
	const JOB_STATE	GetLastResult() { return m_nLastResult; }

	const JOB_STATE GetJobState( const JOB_STATE nJob );
	
private :
	void Lock()		{ EnterCriticalSection( &m_csJob ); }
	void Unlock()	{ LeaveCriticalSection( &m_csJob ); }

	void SetState( const JOB_STATE nJob, const JOB_STATE nState );
	void InsertJob( const JOB_STATE nJob, const JOB_STATE nState );

private :
	JOB_STATE	m_nLastJob;
	JOB_STATE	m_nLastResult;

	CRITICAL_SECTION	m_csJob;
};

#endif