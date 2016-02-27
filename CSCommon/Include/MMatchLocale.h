#ifndef _MMATCHLOCALE_H
#define _MMATCHLOCALE_H


#include "MBaseLocale.h"
#include "MNJ_DBAgentClient.h"
#include "MUID.h"

class MMatchLocale : public MBaseLocale
{
protected:
	virtual bool			OnInit();
	MNJ_DBAgentClient*		m_pDBAgentClient;		///< 일본넷마블에서만 사용하는 클라이언트 소켓
	bool					m_bCheckAntiHackCrack;
public:
							MMatchLocale();
	virtual					~MMatchLocale();
	static MMatchLocale*	GetInstance();

	bool					ConnectToDBAgent();
	bool					PostLoginInfoToDBAgent(const MUID&		uidComm, 
												   const char*		szCN, 
												   const char*		szPW, 
												   bool				bFreeLoginIP, 
												   unsigned long	nChecksumPack, 
												   int				nTotalUserCount);
	bool					SkipCheckAntiHackCrack();	///< XTrap Crack을 체크할지 여부
};

inline MMatchLocale* MGetLocale()
{
	return MMatchLocale::GetInstance();
}



#endif