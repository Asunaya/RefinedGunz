#pragma once

#include "GlobalTypes.h"
#include "MUID.h"

#define LOCATOR_CONFIG "./Locator.ini"


class MLocatorConfig
{
public:
	virtual ~MLocatorConfig(void);

	bool LoadConfig();

	// DB
#ifdef MFC
	const CString& GetDBDSN() const			{ return m_strDBDSN; }
	const CString& GetDBUserName() const	{ return m_strDBUserName; }
	const CString& GetDBPassword() const	{ return m_strDBPassword; }
#endif

	// Network
	const std::string& GetLocatorIP()	const { return m_strLocatorIP; }
	const int GetLocatorPort()		const { return m_nLocatorPort; }

	// Environment
	const u32			GetLocatorID() const							{ return m_dwID; }
	inline const MUID&	GetLocatorUID() const							{ return m_uidLocatorUID; }
	inline const u32	GetMaxElapsedUpdateServerStatusTime() const		{ return m_dwMaxElapsedUpdateServerStatusTime; }
	inline const u32	GetUDPLiveTime() const							{ return m_dwUDPLiveTime; }
	inline const u32	GetMaxFreeUseCountPerLiveTime() const			{ return m_dwMaxFreeUseCountPerLiveTime; }
	inline const u32	GetBlockTime() const							{ return m_dwBlockTime; }
	inline const u32	GetUpdateUDPManagerElapsedTime() const			{ return m_dwUpdateUDPManagerElapsedTime; }
	inline const u32	GetMarginOfErrorMin() const						{ return m_dwMarginOfErrorMin; }
	inline const u32	GetElapsedTimeUpdateLocatorLog() const			{ return m_dwElapsedTimeUpdateLocatorLog; }

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
	void SetLocatorID( const u32 dwID )								{ m_dwID = dwID; }
	void SetLocatorUID( const MUID& uid )								{ m_uidLocatorUID = uid; }
	void SetMaxElapsedUpdateServerSTatusTime( const u32 dwTime )		{ m_dwMaxElapsedUpdateServerStatusTime = dwTime; }
	void SetUDPLiveTime( const u32 dwLiveTime )						{ m_dwUDPLiveTime = dwLiveTime; }
	void SetMaxFreeUseCountPerLiveTime( const u32 dwCount )			{ m_dwMaxFreeUseCountPerLiveTime = dwCount; }
	void SetBlockTime( const u32 dwBlockTime )						{ m_dwBlockTime = dwBlockTime; }
	void SetUpdateUDPManagerElapsedTime( const u32 dwElapsedTime )	{ m_dwUpdateUDPManagerElapsedTime = dwElapsedTime; }
	void SetMarginOfErrorMin( const u32 dwMin )						{ m_dwMarginOfErrorMin = dwMin; }
	void SetCountryCodeFilterStatus( const bool bIsUse )				{ m_bIsUseCountryCodeFilter = bIsUse; }
	void SetElapsedTimeUpdateLocaorLog( const u32 dwTime )			{ m_dwElapsedTimeUpdateLocatorLog = dwTime; }
	void SetAcceptInvalidIP( const bool IsAccept )						{ m_bIsAcceptInvaildIP = IsAccept; }
	void SetTestServerOnly(const bool IsTestServerOnly)					{ m_bIsTestServerOnly = IsTestServerOnly; }
	
private :
	// Network
	string	m_strLocatorIP;
	int		m_nLocatorPort;

	// Evironment
	u32	m_dwID;									// DB에서 Locator를 구별하는 ID.
	MUID	m_uidLocatorUID;
	u32	m_dwMaxElapsedUpdateServerStatusTime;	// /ms 서버의 정보를 DB에서 가져오는 간격.
	u32	m_dwUDPLiveTime;						// /ms UDP큐에 저장되있을수 있는 시간.
	u32	m_dwMaxFreeUseCountPerLiveTime;			// 큐에 저장되 있는동안 받을수 있는 최대 값. 넘어서면 공격자로 취급.
	u32	m_dwBlockTime;							// /ms Block리스트에 등록되있는 시간.
	u32	m_dwUpdateUDPManagerElapsedTime;		// UDP큐 업데이트 간격. Recv,Send,Block큐가 모두 같이 업데이트 됨.
	u32	m_dwMarginOfErrorMin;					// 서버정보 마지막 업데이트 시간중 가장 큰값과 비교하여 
													//  이값이상 차이가 나면 죽은서버로 간주.
	u32	m_dwGMTDiff;
	bool	m_bIsUseCountryCodeFilter;				// 국가코드 필터를 사용할지 결정.
	u32	m_dwElapsedTimeUpdateLocatorLog;

	bool	m_bIsAcceptInvaildIP;
	bool	m_bIsTestServerOnly;					// 테스트서버만 Locating
	
	// DB
#ifdef MFC
	CString m_strDBDSN;
	CString m_strDBUserName;
	CString m_strDBPassword;
#endif

	bool	m_bIsInitCompleted;
};


MLocatorConfig* GetLocatorConfig();