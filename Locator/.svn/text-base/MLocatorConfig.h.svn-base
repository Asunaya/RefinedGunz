#pragma once

#include "MMatchTransDataType.h"


#ifdef _DEBUG
	#define LOCATOR_CONFIG "./DbgLocator.ini"
#else 
	#define LOCATOR_CONFIG "./Locator.ini"
#endif


class MLocatorConfig
{
public:
	virtual ~MLocatorConfig(void);

	bool LoadConfig();

	// DB
	const CString& GetDBDSN() const			{ return m_strDBDSN; }
	const CString& GetDBUserName() const	{ return m_strDBUserName; }
	const CString& GetDBPassword() const	{ return m_strDBPassword; }

	// Network
	const string& GetLocatorIP()	const { return m_strLocatorIP; }
	const int GetLocatorPort()		const { return m_nLocatorPort; }

	// Environment
	const DWORD			GetLocatorID() const							{ return m_dwID; }
	inline const MUID&	GetLocatorUID() const							{ return m_uidLocatorUID; }
	inline const DWORD	GetMaxElapsedUpdateServerStatusTime() const		{ return m_dwMaxElapsedUpdateServerStatusTime; }
	inline const DWORD	GetUDPLiveTime() const							{ return m_dwUDPLiveTime; }
	inline const DWORD	GetMaxFreeUseCountPerLiveTime() const			{ return m_dwMaxFreeUseCountPerLiveTime; }
	inline const DWORD	GetBlockTime() const							{ return m_dwBlockTime; }
	inline const DWORD	GetUpdateUDPManagerElapsedTime() const			{ return m_dwUpdateUDPManagerElapsedTime; }
	inline const DWORD	GetMarginOfErrorMin() const						{ return m_dwMarginOfErrorMin; }
	inline const DWORD	GetElapsedTimeUpdateLocatorLog() const			{ return m_dwElapsedTimeUpdateLocatorLog; }

	inline const bool IsUseCountryCodeFilter() const	{ return m_bIsUseCountryCodeFilter; }
	inline const bool IsInitCompleted() const			{ return m_bIsInitCompleted; }
	inline const bool IsAcceptInvalidIP() const			{ return m_bIsAcceptInvaildIP; }
	inline const bool IsTestServerOnly() const			{ return m_bIsTestServerOnly; }

	static MLocatorConfig* GetLocatorConfigInst()
	{
		static MLocatorConfig LocatorConfig;
		return &LocatorConfig;
	}

private :
	MLocatorConfig(void);

	bool LoadDBConfig();
	bool LoadNetConfig();
	bool LoadEnvConfig();

	// DB
	bool SetDBDSN( const char* pszDSN );
	bool SetDBUserName( const char* pszUserName );
	bool SetDBPassword( const char* pszPassword );

	// Network
	void SetLocatorIP( const string& strLocatorIP )		{ m_strLocatorIP = strLocatorIP; }
	void SetLocatorPort( const int nPort )				{ m_nLocatorPort = nPort; }

	// Environment
	void SetLocatorID( const DWORD dwID )								{ m_dwID = dwID; }
	void SetLocatorUID( const MUID& uid )								{ m_uidLocatorUID = uid; }
	void SetMaxElapsedUpdateServerSTatusTime( const DWORD dwTime )		{ m_dwMaxElapsedUpdateServerStatusTime = dwTime; }
	void SetUDPLiveTime( const DWORD dwLiveTime )						{ m_dwUDPLiveTime = dwLiveTime; }
	void SetMaxFreeUseCountPerLiveTime( const DWORD dwCount )			{ m_dwMaxFreeUseCountPerLiveTime = dwCount; }
	void SetBlockTime( const DWORD dwBlockTime )						{ m_dwBlockTime = dwBlockTime; }
	void SetUpdateUDPManagerElapsedTime( const DWORD dwElapsedTime )	{ m_dwUpdateUDPManagerElapsedTime = dwElapsedTime; }
	void SetMarginOfErrorMin( const DWORD dwMin )						{ m_dwMarginOfErrorMin = dwMin; }
	void SetCountryCodeFilterStatus( const bool bIsUse )				{ m_bIsUseCountryCodeFilter = bIsUse; }
	void SetElapsedTimeUpdateLocaorLog( const DWORD dwTime )			{ m_dwElapsedTimeUpdateLocatorLog = dwTime; }
	void SetAcceptInvalidIP( const bool IsAccept )						{ m_bIsAcceptInvaildIP = IsAccept; }
	void SetTestServerOnly(const bool IsTestServerOnly)					{ m_bIsTestServerOnly = IsTestServerOnly; }
	
private :
	// Network
	string	m_strLocatorIP;
	int		m_nLocatorPort;

	// Evironment
	DWORD	m_dwID;									// DB에서 Locator를 구별하는 ID.
	MUID	m_uidLocatorUID;
	DWORD	m_dwMaxElapsedUpdateServerStatusTime;	// /ms 서버의 정보를 DB에서 가져오는 간격.
	DWORD	m_dwUDPLiveTime;						// /ms UDP큐에 저장되있을수 있는 시간.
	DWORD	m_dwMaxFreeUseCountPerLiveTime;			// 큐에 저장되 있는동안 받을수 있는 최대 값. 넘어서면 공격자로 취급.
	DWORD	m_dwBlockTime;							// /ms Block리스트에 등록되있는 시간.
	DWORD	m_dwUpdateUDPManagerElapsedTime;		// UDP큐 업데이트 간격. Recv,Send,Block큐가 모두 같이 업데이트 됨.
	DWORD	m_dwMarginOfErrorMin;					// 서버정보 마지막 업데이트 시간중 가장 큰값과 비교하여 
													//  이값이상 차이가 나면 죽은서버로 간주.
	DWORD	m_dwGMTDiff;
	bool	m_bIsUseCountryCodeFilter;				// 국가코드 필터를 사용할지 결정.
	DWORD	m_dwElapsedTimeUpdateLocatorLog;

	bool	m_bIsAcceptInvaildIP;
	bool	m_bIsTestServerOnly;					// 테스트서버만 Locating
	
	// DB
	CString m_strDBDSN;
	CString m_strDBUserName;
	CString m_strDBPassword;

	bool	m_bIsInitCompleted;
};


MLocatorConfig* GetLocatorConfig();