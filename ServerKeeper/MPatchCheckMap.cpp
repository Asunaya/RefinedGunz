#include "stdafx.h"
#include "MPatchCheckMap.h"
#include "MDebug.h"


MPatchCheckMap::MPatchCheckMap()
{
	// Init();
}


MPatchCheckMap::~MPatchCheckMap()
{
}


void MPatchCheckMap::Init()
{
	InitializeCriticalSection( &m_csJob );

	Lock();

	/// 공통.
	InsertJob( JOB_SUCCESS_CONNECT_FTP_SERVER,			JOB_FAIL_CONNECT_FTP_SERVER );

	/// MatchServer.
	InsertJob( JOB_SUCCESS_DOWNLOAD_SERVER_PATCH_FILE,	JOB_FAIL_DOWNLOAD_SERVER_PATCH_FILE );
	InsertJob( JOB_SUCCESS_PREPARE_SERVER_PATCH,		JOB_FAIL_PREPARE_SERVER_PATCH );
	InsertJob( JOB_SUCCESS_STOP_SERVER,					JOB_FAIL_STOP_SERVER );
	InsertJob( JOB_SUCCESS_PATCH_SERVER,				JOB_FAIL_PATCH_SERVER );

	/// MatchAgent.
	InsertJob( JOB_SUCCESS_DOWNLOAD_AGENT_PATCH_FILE,	JOB_FAIL_DOWNLOAD_AGENT_PATCH_FILE );
	InsertJob( JOB_SUCCESS_PREPARE_AGENT_PATCH,			JOB_FAIL_PREPARE_AGENT_PATCH );
	InsertJob( JOB_SUCCESS_STOP_AGENT,					JOB_FAIL_STOP_AGENT );
	InsertJob( JOB_SUCCESS_PATCH_AGENT,					JOB_FAIL_PATCH_AGENT );

	m_nLastJob		= JOB_NON;
	m_nLastResult	= JOB_NON;
	
	Unlock();
}


void MPatchCheckMap::Check( const JOB_STATE nJob, const JOB_STATE nState )
{
	if( (0 <= nState) && (nState < JOB_END) )
	{
		Lock();

		iterator itJob = find( nJob );
		if( end() != itJob )
			itJob->second = nState;	// 등록된 작업일경우 상테 업데이트.

		// 등록되지 않은 작업이라도 경과 상태를 저장함.
		m_nLastJob		= nJob;		
		m_nLastResult	= nState;

		Unlock();
	}
}


void MPatchCheckMap::Reset()
{
	Lock();

	/// 공통.
	SetState( JOB_SUCCESS_CONNECT_FTP_SERVER,			JOB_FAIL_CONNECT_FTP_SERVER );

	/// MatchServer.
	SetState( JOB_SUCCESS_DOWNLOAD_SERVER_PATCH_FILE,	JOB_FAIL_DOWNLOAD_SERVER_PATCH_FILE );
	SetState( JOB_SUCCESS_PREPARE_SERVER_PATCH,			JOB_FAIL_PREPARE_SERVER_PATCH );
	SetState( JOB_SUCCESS_STOP_SERVER,					JOB_FAIL_STOP_SERVER );
	SetState( JOB_SUCCESS_PATCH_SERVER,					JOB_FAIL_PATCH_SERVER );

	/// MatchAgent.
	SetState( JOB_SUCCESS_DOWNLOAD_AGENT_PATCH_FILE,	JOB_FAIL_DOWNLOAD_AGENT_PATCH_FILE );
	SetState( JOB_SUCCESS_PREPARE_AGENT_PATCH,			JOB_FAIL_PREPARE_AGENT_PATCH );
	SetState( JOB_SUCCESS_STOP_AGENT,					JOB_FAIL_STOP_AGENT );
	SetState( JOB_SUCCESS_PATCH_AGENT,					JOB_FAIL_PATCH_AGENT );

	m_nLastJob		= JOB_NON;
	m_nLastResult	= JOB_NON;

	Unlock();
}


bool MPatchCheckMap::IsJobSuccess( const JOB_STATE nJob )	
{ 
	iterator it = find( nJob );
	if( end() != it )
	{
		Lock();

		bool bResult = (it->first == it->second);

		Unlock();

		return bResult;
	}

	return false;
}


bool MPatchCheckMap::IsServerPrepareComplete()	
{ 
	return ( IsJobSuccess(JOB_SUCCESS_CONNECT_FTP_SERVER) && 
			 IsJobSuccess(JOB_SUCCESS_DOWNLOAD_SERVER_PATCH_FILE) &&
			 IsJobSuccess(JOB_SUCCESS_PREPARE_SERVER_PATCH) );
}


bool MPatchCheckMap::IsServerPatchComplete()		
{ 
	return ( IsServerPrepareComplete() && IsJobSuccess(JOB_SUCCESS_PATCH_SERVER) ); 
}


void MPatchCheckMap::SetState( const JOB_STATE nJob, const JOB_STATE nState )
{
	if( (0 <= nState) && (nState < JOB_END) )
	{
		Lock();

		iterator itJob = find( nJob );
		if( end() == itJob )
		{
			ASSERT( 0 );	// 등록되지 않은 작업.
			return;
		}

		itJob->second = nState;

		Unlock();
	}
	else
	{
		ASSERT( 0 );
	}
}


void MPatchCheckMap::InsertJob( const JOB_STATE nJob, const JOB_STATE nState )
{
	Lock();

	iterator itDupJob = find( nJob );
	if( end() != itDupJob )
	{
		ASSERT( 0 );	// 중복.
		return;
	}

	insert( map<JOB_STATE, JOB_STATE>::value_type(nJob,	nState) );

	Unlock();
}


const JOB_STATE MPatchCheckMap::GetJobState( const JOB_STATE nJob )
{
	JOB_STATE nState = JOB_END;
	if( JOB_END > nJob )
	{
		Lock();
		iterator itJob = find( nJob );
		if( end() != itJob )
			nState = itJob->second;
		Unlock();
	}

	return nState;
}


bool MPatchCheckMap::IsAgentPrepareComplete()
{
	return ( IsJobSuccess(JOB_SUCCESS_CONNECT_FTP_SERVER) &&
			 IsJobSuccess(JOB_SUCCESS_DOWNLOAD_AGENT_PATCH_FILE) &&
			 IsJobSuccess(JOB_SUCCESS_PREPARE_AGENT_PATCH) );
}

bool MPatchCheckMap::IsServerPatchDownloadComplete()
{
	return IsJobSuccess( JOB_SUCCESS_DOWNLOAD_SERVER_PATCH_FILE );
}


bool MPatchCheckMap::IsAgentPatchDownloadComplete()
{
	return IsJobSuccess( JOB_SUCCESS_DOWNLOAD_AGENT_PATCH_FILE );
}


bool MPatchCheckMap::IsPossibleAgentPatchDownload()
{
	return ( (JOB_DOWNLOADING_AGENT_PATCH	!= GetJobState(JOB_SUCCESS_DOWNLOAD_AGENT_PATCH_FILE)) &&
			 (JOB_PREPARING_AGENT_PATCH		!= GetJobState(JOB_SUCCESS_PREPARE_AGENT_PATCH)) &&
			 (JOB_PATCHING_AGENT			!= GetJobState(JOB_SUCCESS_PATCH_AGENT)) );
}


bool MPatchCheckMap::IsPossibleServerPatchDownload()
{
	return ( (JOB_DOWNLOADING_SERVER_PATCH	!= GetJobState(JOB_SUCCESS_DOWNLOAD_SERVER_PATCH_FILE)) &&
			 (JOB_PREPARING_SERVER_PATCH	!= GetJobState(JOB_SUCCESS_PREPARE_SERVER_PATCH)) &&
			 (JOB_PATCHING_SERVER			!= GetJobState(JOB_SUCCESS_PATCH_SERVER)) );
}


bool MPatchCheckMap::IsPossibleReset()
{
	return ( IsPossibleAgentPatchDownload() && IsPossibleServerPatchDownload() );
}