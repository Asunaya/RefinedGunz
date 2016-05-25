#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "Msg.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_GetLoginInfo.h"
#include "MAsyncDBJob_InsertConnLog.h"
#include "RTypes.h"
#include "MMatchUtil.h"
#include <winbase.h>
#include "MMatchPremiumIPCache.h"
#include "MCommandBuilder.h"
#include "MMatchStatus.h"
#include "MMatchLocale.h"

#define SODIUM_STATIC
#include "sodium.h"

bool MMatchServer::CheckOnLoginPre(const MUID& CommUID, int nCmdVersion, bool& outbFreeIP, string& strCountryCode3)
{
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj == NULL) return false;

	// 프로토콜 버전 체크
	if (nCmdVersion != MCOMMAND_VERSION)
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_COMMAND_INVALID_VERSION);
		Post(pCmd);	
		return false;
	}

	// free login ip를 검사하기전에 debug서버와 debug ip를 검사한다.
	// 서버가 debug타입인지 검사.
	if( MGetServerConfig()->IsDebugServer() && MGetServerConfig()->IsDebugLoginIPList(pCommObj->GetIPString()) )
	{
		outbFreeIP = true;
		return true;
	}

	// 최대인원 체크
	bool bFreeLoginIP = false;
	if (MGetServerConfig()->CheckFreeLoginIPList(pCommObj->GetIPString()) == true) {
		bFreeLoginIP = true;
		outbFreeIP = true;
		return true;
	} else {
		outbFreeIP = false;

		if ((int)m_Objects.size() >= MGetServerConfig()->GetMaxUser())
		{
			MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_FULL_PLAYERS);
			Post(pCmd);	
			return false;
		}
	}

	if( CheckIsValidIP(CommUID, pCommObj->GetIPString(), strCountryCode3, MGetServerConfig()->IsUseFilter()) )
		IncreaseNonBlockCount();
	else
	{
		IncreaseBlockCount();
		return false;
	}

	return true;
}

void MMatchServer::OnMatchLogin(MUID CommUID, const char* szUserID, const unsigned char *HashedPassword, int HashLength, int nCommandVersion, unsigned long nChecksumPack, int nVersion)
{
	if (HashLength != crypto_generichash_BYTES)
		return;

	int nMapID = 0;

	unsigned int nAID = 0;
	char szDBPassword[256] = "";
	string strCountryCode3;

	bool bFreeLoginIP = false;

	if (nVersion < RGUNZ_VERSION)
	{
		char buf[128];
		sprintf_safe(buf, "Your client is outdated (expected version %d, got %d)", RGUNZ_VERSION, nVersion);
		NotifyFailedLogin(CommUID, buf);
		return;
	}

	// 프로토콜, 최대인원 체크
	if (!CheckOnLoginPre(CommUID, nCommandVersion, bFreeLoginIP, strCountryCode3)) return;


	// 원래 계정은 넷마블에 있으므로 해당 계정이 없으면 새로 생성한다. 
	if (!m_MatchDBMgr.GetLoginInfo(szUserID, &nAID, szDBPassword))
	{
//#ifdef _DEBUG
//		m_MatchDBMgr.CreateAccount(szUserID, szPassword, 0, szUserID, 20, 1);
//		strcpy_safe(szDBPassword, szPassword);
//
//		m_MatchDBMgr.GetLoginInfo(szUserID, &nAID, szDBPassword);
//#endif

		/*MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_WRONG_PASSWORD);
		Post(pCmd);*/
		char buf[128];
		sprintf_safe(buf, "Couldn't find username %s", szUserID);
		NotifyFailedLogin(CommUID, buf);

		return;
	}


	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj)
	{
		// 디비에 최종 접속시간을 업데이트 한다.
		if (!m_MatchDBMgr.UpdateLastConnDate(szUserID, pCommObj->GetIPString()))
		{
			mlog("DB Query(OnMatchLogin > UpdateLastConnDate) Failed");
		}

	}


	// 패스워드가 틀렸을 경우 처리
	/*if (strcmp(szDBPassword, szPassword))
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_WRONG_PASSWORD);
		Post(pCmd);	

		return;
	}*/

	if (crypto_pwhash_scryptsalsa208sha256_str_verify
		(szDBPassword, (char *)HashedPassword, HashLength) != 0) {
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_WRONG_PASSWORD);
		Post(pCmd);
	}

	MMatchAccountInfo accountInfo;
	if (!m_MatchDBMgr.GetAccountInfo(nAID, &accountInfo))
	{
		NotifyFailedLogin(CommUID, "Failed to retrieve account information");
		// 접속 끊어버리자
		Disconnect(CommUID);
	}

#ifndef _DEBUG
	// 중복 로그인이면 이전에 있던 사람을 끊어버린다.
	MMatchObject* pCopyObj = GetPlayerByAID(accountInfo.m_nAID);
	if (pCopyObj != NULL) 
	{
		DisconnectObject(pCopyObj->GetUID());
	}
#endif

	// 사용정지 계정인지 확인한다.
	if ((accountInfo.m_nUGrade == MMUG_BLOCKED) || (accountInfo.m_nUGrade == MMUG_PENALTY))
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_MMUG_BLOCKED);
		Post(pCmd);	
		return;
	}

	// 로그인성공하여 오브젝트(MMatchObject) 생성
	AddObjectOnMatchLogin(CommUID, &accountInfo, bFreeLoginIP, strCountryCode3, nChecksumPack);
}

void MMatchServer::NotifyFailedLogin(const MUID& uidComm, const char *szReason)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_LOGIN_FAILED, uidComm);
	pCmd->AddParameter(new MCommandParameterString(szReason));
	Post(pCmd);
}

void MMatchServer::CreateAccount(const MUID &uidComm, const char *szUsername, const unsigned char *HashedPassword, int HashLength, const char *szEmail)
{
	if (szUsername[0] == 0)
	{
		CreateAccountResponse(uidComm, "Account creation failed: Empty username");
		return;
	}

	if (HashLength != crypto_generichash_BYTES)
	{
		CreateAccountResponse(uidComm, "Account creation failed: Invalid hash");
		return;
	}

	char szPasswordData[crypto_pwhash_scryptsalsa208sha256_STRBYTES];

	if (crypto_pwhash_scryptsalsa208sha256_str
		(szPasswordData, (char *)HashedPassword, HashLength,
		crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE,
		crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE) != 0) {
		CreateAccountResponse(uidComm, "Account creation failed: Server ran out of memory");
		return;
	}

	auto ret = m_MatchDBMgr.CreateAccountNew(szUsername, szPasswordData, szEmail);
	switch (ret)
	{
	case ACR_OK:
		CreateAccountResponse(uidComm, "Account created!");
		break;
	case ACR_USERNAME_ALREADY_EXISTS:
		CreateAccountResponse(uidComm, "Account creation failed: Username already exists");
		break;
	case ACR_DB_ERROR:
		CreateAccountResponse(uidComm, "Account creation failed: Unknown database error");
		break;
	default:
		CreateAccountResponse(uidComm, "Account creatIon failed: Unknown error");
		break;
	};
}

void MMatchServer::CreateAccountResponse(const MUID& uidComm, const char *szReason)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_CREATE_ACCOUNT, uidComm);
	pCmd->AddParameter(new MCommandParameterString(szReason));
	Post(pCmd);
}

void MMatchServer::OnMatchLoginFromNetmarble(const MUID& CommUID, const char* szCPCookie, const char* szSpareData, int nCmdVersion, unsigned long nChecksumPack)
{
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj == NULL) return;


	bool bFreeLoginIP = false;
	string strCountryCode3;

	// 프로토콜, 최대인원 체크
	if (!CheckOnLoginPre(CommUID, nCmdVersion, bFreeLoginIP, strCountryCode3)) return;


	MMatchAuthBuilder* pAuthBuilder = GetAuthBuilder();
	if (pAuthBuilder == NULL) {
		LOG(LOG_ALL, "Critical Error : MatchAuthBuilder is not assigned.");
		return;
	}
	MMatchAuthInfo* pAuthInfo = NULL;
	if (pAuthBuilder->ParseAuthInfo(szCPCookie, &pAuthInfo) == false) 
	{
		MGetServerStatusSingleton()->SetRunStatus(5);

		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_WRONG_PASSWORD);
		Post(pCmd);	

		LOG(LOG_ALL, "Netmarble Certification Failed");
		return;
	}

	const char* pUserID = pAuthInfo->GetUserID();
	const char* pUniqueID = pAuthInfo->GetUniqueID();
	const char* pCertificate = pAuthInfo->GetCertificate();
	const char* pName = pAuthInfo->GetName();
	int nAge = pAuthInfo->GetAge();
	int nSex = pAuthInfo->GetSex();
	bool bCheckPremiumIP = MGetServerConfig()->CheckPremiumIP();
	const char* szIP = pCommObj->GetIPString();
	DWORD dwIP = pCommObj->GetIP();

	// Async DB
	MAsyncDBJob_GetLoginInfo* pNewJob = new MAsyncDBJob_GetLoginInfo(CommUID);
	pNewJob->Input(new MMatchAccountInfo(), 
					pUserID, 
					pUniqueID, 
					pCertificate, 
					pName, 
					nAge, 
					nSex, 
					bFreeLoginIP, 
					nChecksumPack,
					bCheckPremiumIP,
					szIP,
					dwIP,
					strCountryCode3);
	PostAsyncJob(pNewJob);

	if (pAuthInfo)
	{
		delete pAuthInfo; pAuthInfo = NULL;
	}
}


void MMatchServer::OnMatchLoginFromNetmarbleJP(const MUID& CommUID, const char* szLoginID, const char* szLoginPW, int nCmdVersion, unsigned long nChecksumPack)
{
	bool bFreeLoginIP = false;
	string strCountryCode3;

	// 프로토콜, 최대인원 체크
	if (!CheckOnLoginPre(CommUID, nCmdVersion, bFreeLoginIP, strCountryCode3)) return;

	// DBAgent에 먼저 보내고 응답을 받으면 로그인 프로세스를 진행한다.
	if (!MGetLocale()->PostLoginInfoToDBAgent(CommUID, szLoginID, szLoginPW, bFreeLoginIP, nChecksumPack, GetClientCount()))
	{
		MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, MERR_CLIENT_FULL_PLAYERS);
		Post(pCmd);
		return;
	}
}

void MMatchServer::OnMatchLoginFromDBAgent(const MUID& CommUID, const char* szLoginID, const char* szName, int nSex, bool bFreeLoginIP, unsigned long nChecksumPack)
{
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(CommUID);
	if (pCommObj == NULL) return;

	string strCountryCode3;
	CheckIsValidIP( CommUID, pCommObj->GetIPString(), strCountryCode3, false );

	const char* pUserID = szLoginID;
	char szPassword[16] = "";			// 패스워드는 없다
	char szCertificate[16] = "";
	const char* pName = szName;
	int nAge = 20;

	bool bCheckPremiumIP = MGetServerConfig()->CheckPremiumIP();
	const char* szIP = pCommObj->GetIPString();
	DWORD dwIP = pCommObj->GetIP();

	// Async DB
	MAsyncDBJob_GetLoginInfo* pNewJob = new MAsyncDBJob_GetLoginInfo(CommUID);
	pNewJob->Input(new MMatchAccountInfo(), 
					pUserID, 
					szPassword, 
					szCertificate, 
					pName, 
					nAge, 
					nSex, 
					bFreeLoginIP, 
					nChecksumPack,
					bCheckPremiumIP,
					szIP,
					dwIP,
					strCountryCode3);
	PostAsyncJob(pNewJob);
}

void MMatchServer::OnMatchLoginFailedFromDBAgent(const MUID& CommUID, int nResult)
{
	// 프로토콜 버전 체크
	MCommand* pCmd = CreateCmdMatchResponseLoginFailed(CommUID, nResult);
	Post(pCmd);	
}

MCommand* MMatchServer::CreateCmdMatchResponseLoginOK(const MUID& uidComm, 
													  MUID& uidPlayer, 
													  const char* szUserID, 
													  MMatchUserGradeID nUGradeID, 
													  MMatchPremiumGradeID nPGradeID,
													  const char* szRandomValue,
													  const unsigned char* pbyGuidReqMsg)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_LOGIN, uidComm);
	pCmd->AddParameter(new MCommandParameterInt(MOK));
	pCmd->AddParameter(new MCommandParameterString(MGetServerConfig()->GetServerName()));
	pCmd->AddParameter(new MCommandParameterChar((char)MGetServerConfig()->GetServerMode()));
	pCmd->AddParameter(new MCommandParameterString(szUserID));
	pCmd->AddParameter(new MCommandParameterUChar((unsigned char)nUGradeID));
	pCmd->AddParameter(new MCommandParameterUChar((unsigned char)nPGradeID));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterString(szRandomValue));
	
	void* pBlob = MMakeBlobArray(sizeof(unsigned char), SIZEOF_GUIDREQMSG);
	unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
	CopyMemory(pCmdBlock, pbyGuidReqMsg, SIZEOF_GUIDREQMSG);

	pCmd->AddParameter(new MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));
	MEraseBlobArray(pBlob);

	return pCmd;
}

MCommand* MMatchServer::CreateCmdMatchResponseLoginFailed(const MUID& uidComm, const int nResult)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_LOGIN, uidComm);
	pCmd->AddParameter(new MCommandParameterInt(nResult));
	pCmd->AddParameter(new MCommandParameterString(MGetServerConfig()->GetServerName()));
	pCmd->AddParameter(new MCommandParameterChar((char)MGetServerConfig()->GetServerMode()));
	pCmd->AddParameter(new MCommandParameterString("Ana"));
	pCmd->AddParameter(new MCommandParameterUChar((unsigned char)MMUG_FREE));
	pCmd->AddParameter(new MCommandParameterUChar((unsigned char)MMPG_FREE));
	pCmd->AddParameter(new MCommandParameterUID(MUID(0,0)));
	pCmd->AddParameter(new MCommandParameterString("A"));
	
	unsigned char tmp = 0;
	void* pBlob = MMakeBlobArray(sizeof(unsigned char), sizeof(unsigned char));
	unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
	CopyMemory(pCmdBlock, &tmp, sizeof(unsigned char));

	pCmd->AddParameter(new MCommandParameterBlob(pBlob, MGetBlobArraySize(pBlob)));
	MEraseBlobArray(pBlob);

	return pCmd;
}


bool MMatchServer::AddObjectOnMatchLogin(const MUID& uidComm, 
 										 MMatchAccountInfo* pSrcAccountInfo,
 										 bool bFreeLoginIP,
										 string strCountryCode3,
										 unsigned long nChecksumPack)
{
	MCommObject* pCommObj = (MCommObject*)m_CommRefCache.GetRef(uidComm);
	if (pCommObj == NULL) return false;

	MUID AllocUID = uidComm;
	int nErrCode = ObjectAdd(uidComm);
	if(nErrCode!=MOK){
		LOG(LOG_DEBUG, MErrStr(nErrCode) );
	}

	MMatchObject* pObj = GetObject(AllocUID);
	if (pObj == NULL) 
	{
		Disconnect(uidComm);
		return false;
	}

	pObj->AddCommListener(uidComm);
	pObj->SetObjectType(MOT_PC);
	memcpy(pObj->GetAccountInfo(), pSrcAccountInfo, sizeof(MMatchAccountInfo));
	pObj->SetFreeLoginIP(bFreeLoginIP);
	pObj->SetCountryCode3( strCountryCode3 );
	pObj->UpdateTickLastPacketRecved();
	pObj->UpdateLastHShieldMsgRecved();

	if (pCommObj != NULL)
	{
		pObj->SetPeerAddr(pCommObj->GetIP(), pCommObj->GetIPString(), pCommObj->GetPort());
	}
	
	SetClientClockSynchronize(uidComm);

	// 프리미엄 IP를 체크한다.
	if (MGetServerConfig()->CheckPremiumIP())
	{
		if (pCommObj)
		{
			bool bIsPremiumIP = false;
			bool bExistPremiumIPCache = false;
			
			bExistPremiumIPCache = MPremiumIPCache()->CheckPremiumIP(pCommObj->GetIP(), bIsPremiumIP);

			// 만약 캐쉬에 없으면 직접 DB에서 찾도록 한다.
			if (!bExistPremiumIPCache)
			{
				if (m_MatchDBMgr.CheckPremiumIP(pCommObj->GetIPString(), bIsPremiumIP))
				{
					// 결과를 캐쉬에 저장
					MPremiumIPCache()->AddIP(pCommObj->GetIP(), bIsPremiumIP);
				}
				else
				{
					MPremiumIPCache()->OnDBFailed();
				}

			}

			if (bIsPremiumIP) pObj->GetAccountInfo()->m_nPGrade = MMPG_PREMIUM_IP;
		}		
	}

#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
	{
		ULONG dwRet = HShield_MakeGuidReqMsg(pObj->GetHShieldInfo()->m_pbyGuidReqMsg, pObj->GetHShieldInfo()->m_pbyGuidReqInfo);

		if(dwRet != ERROR_SUCCESS)
			LOG(LOG_FILE, "@MakeGuidReqMsg - %s Making Guid Req Msg Failed. (Error code = %x)", pObj->GetAccountName(), dwRet);
	}
#endif

#ifdef _XTRAP
	// XTrap Random Value 생성
	if( MGetServerConfig()->IsUseXTrap() )
	{
		if (!MGetLocale()->SkipCheckAntiHackCrack())
			XTrap_RandomKeyGenW(pObj->GetAntiHackInfo()->m_szRandomValue);
	}
#endif

	MCommand* pCmd = CreateCmdMatchResponseLoginOK(uidComm, 
												   AllocUID, 
												   pObj->GetAccountInfo()->m_szUserID,
												   pObj->GetAccountInfo()->m_nUGrade,
                                                   pObj->GetAccountInfo()->m_nPGrade,
												   pObj->GetAntiHackInfo()->m_szRandomValue,
												   pObj->GetHShieldInfo()->m_pbyGuidReqMsg);
	Post(pCmd);	

	// 접속 로그를 남긴다.
	//m_MatchDBMgr.InsertConnLog(pObj->GetAccountInfo()->m_nAID, pObj->GetIPString(), pObj->GetCountryCode3() );

	// 접속 로그
	MAsyncDBJob_InsertConnLog* pNewJob = new MAsyncDBJob_InsertConnLog();
	pNewJob->Input(pObj->GetAccountInfo()->m_nAID, pObj->GetIPString(), pObj->GetCountryCode3() );
	PostAsyncJob(pNewJob);

	// Client DataFile Checksum을 검사한다.
	// 2006.2.20 dubble. filelist checksum으로 변경
	//unsigned long nChecksum = nChecksumPack ^ uidComm.High ^ uidComm.Low;
	//if( MGetServerConfig()->IsUseFileCrc() && !MMatchAntiHack::CheckClientFileListCRC(nChecksum, pObj->GetUID()) && 
	//	!MGetServerConfig()->IsDebugLoginIPList(pObj->GetIPString()) )
	//{
	//	LOG(LOG_ALL, "Invalid filelist crc (%u) , UserID(%s) ", nChecksum, pObj->GetAccountInfo()->m_szUserID);
	//	pObj->SetBadFileCRCDisconnectWaitInfo();
	//	// Disconnect(uidComm);
	//	// return false;

	//	
	//}
	/*
	if (nChecksum != GetItemFileChecksum()) {
		LOG(LOG_ALL, "Invalid ZItemChecksum(%u) , UserID(%s) ", nChecksum, pObj->GetAccountInfo()->m_szUserID);
		Disconnect(uidComm);
		return false;
	}
	*/

	return true;
}
