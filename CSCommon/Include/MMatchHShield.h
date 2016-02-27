#pragma once

#include "../../MatchServer/HSHIELD/AntiCPSvrFunc.h"

struct MMatchObjectHShieldInfo
{
	unsigned char m_pbyGuidReqMsg[SIZEOF_GUIDREQMSG];
	unsigned char m_pbyGuidReqInfo[SIZEOF_GUIDREQINFO];
	unsigned char m_pbyReqMsg[SIZEOF_REQMSG];
	unsigned char m_pbyReqInfo[SIZEOF_REQINFO];

	unsigned long* m_pCRCInfo;

	bool		m_bGuidAckPass;

	MMatchObjectHShieldInfo()	{	Clear();	};
	void Clear()				{	memset(m_pbyGuidReqMsg, 0, sizeof(m_pbyGuidReqMsg));
									memset(m_pbyGuidReqInfo, 0, sizeof(m_pbyGuidReqInfo));	
									memset(m_pbyReqMsg, 0, sizeof(m_pbyReqMsg));
									memset(m_pbyReqInfo, 0, sizeof(m_pbyReqInfo));
									m_pCRCInfo = NULL;
									m_bGuidAckPass = false;
								};
};

class MMatchHShield
{
public:
	MMatchHShield(void);
	virtual ~MMatchHShield(void);
};