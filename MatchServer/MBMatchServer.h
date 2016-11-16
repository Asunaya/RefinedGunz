#ifndef MBMATCHSERVER_H
#define MBMATCHSERVER_H

#include "MMatchServer.h"
#include "MBMatchServerConfigReloader.h"
#include <memory>
#include "MLocator.h"

class COutputView;
class CCommandLogView;

class MBMatchServer : public MMatchServer
{
private :
	MBMatchServerConfigReloader m_ConfigReloader;

public:
#ifdef MFC
	COutputView*		m_pView;
	CCommandLogView*	m_pCmdLogView;
#endif

	MUID m_uidKeeper;
protected:
	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void OnPrepareCommand(MCommand* pCommand);
	virtual bool OnCommand(MCommand* pCommand);

	virtual void OnRun() override;

public:
	MBMatchServer(COutputView* pView=NULL);
	virtual void Shutdown();
	virtual void Log(unsigned int nLogLevel, const char* szLog);
	void OnViewServerStatus();

private :
	bool InitSubTaskSchedule();
	bool AddClanServerAnnounceSchedule();
	bool AddClanServerSwitchDownSchedule();
	bool AddClanServerSwitchUpSchedule();
	
	void OnScheduleAnnounce( const char* pszAnnounce );
	void OnScheduleClanServerSwitchDown();
	void OnScheduleClanServerSwitchUp();

	const MUID GetKeeperUID() const { return m_uidKeeper; }
	void SetKeeperUID( const MUID& uidKeeper ) { m_uidKeeper = uidKeeper; }

	void WriteServerInfoLog();

protected :
	bool IsKeeper( const MUID& uidKeeper );

	void OnResponseServerStatus( const MUID& uidSender );
	void OnRequestServerHearbeat( const MUID& uidSender );
	void OnResponseServerHeartbeat( const MUID& uidSender );
	void OnRequestConnectMatchServer( const MUID& uidSender );
	void OnResponseConnectMatchServer( const MUID& uidSender );
	void OnRequestKeeperAnnounce( const MUID& uidSender, const char* pszAnnounce );
	void OnRequestStopServerWithAnnounce( const MUID& uidSender );
	void OnResponseStopServerWithAnnounce( const MUID& uidSender );
	void OnRequestSchedule( const MUID& uidSender, 
							const int nType, 
							const int nYear, 
							const int nMonth, 
							const int nDay, 
							const int nHour, 
							const int nMin,
							const int nCount,
							const int nCommand,
							const char* pszAnnounce );
	void OnResponseSchedule( const MUID& uidSender, 
							 const int nType, 
							 const int nYear, 
							 const int nMonth, 
							 const int nDay, 
							 const int nHour, 
							 const int nMin,
							 const int nCount,
							 const int nCommand,
							 const char* pszAnnounce );
	void OnRequestKeeperStopServerSchedule( const MUID& uidSender, const char* pszAnnounce );
	void OnResponseKeeperStopServerSchedule( const MUID& uidSender, const char* pszAnnounce );
	void OnRequestDisconnectServerFromKeeper( const MUID& uidSender );
	void OnRequestReloadServerConfig( const MUID& uidSender, const string& strFileList );
	void OnResponseReloadServerConfig( const MUID& uidSender, const string& strFileList );
	void OnRequestAddHashMap( const MUID& uidSender, const string& strNewHashValue );
	void OnResponseAddHashMap( const MUID& uidSender, const string& strNewHashValue );

	// filter
	void OnLocalUpdateUseCountryFilter();
	void OnLocalGetDBIPtoCountry();
	void OnLocalGetDBBlockCountryCode();
	void OnLocalGetDBCustomIP();
	void OnLocalUpdateIPtoCountry();
	void OnLocalUpdateBlockCountryCode();
	void OnLocalUpdateCustomIP();
	void OnLocalUpdateAcceptInvaildIP();

public:
	// xtrap
	virtual void	XTrap_RandomKeyGenW(char* strKeyValue);
	virtual int		XTrap_XCrackCheckW(char* strSerialKey, char* strRandomValue, char* strHashValue);

	// HShield
	virtual ULONG	HShield_MakeGuidReqMsg(unsigned char *pbyGuidReqMsg, unsigned char *pbyGuidReqInfo);
	virtual ULONG	HShield_AnalyzeGuidAckMsg(unsigned char *pbyGuidAckMsg, unsigned char *pbyGuidReqInfo, unsigned long **ppCrcInfo);
	virtual ULONG   HShield_MakeReqMsg(unsigned long *pCrcInfo, unsigned char *pbyReqMsg, unsigned char *pbyReqInfo, unsigned long ulOption);
	virtual ULONG   HShield_AnalyzeAckMsg(unsigned long *pCrcInfo, unsigned char *pbyAckMsg, unsigned char *pbyReqInfo);

	char	m_strFileCrcDataPath[MAX_PATH];

	void InitLocator();

private:
	std::unique_ptr<MLocator> Locator;
};


#endif