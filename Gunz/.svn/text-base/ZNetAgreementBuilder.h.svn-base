#ifndef _ZNETAGREEMENTBUILDER_H
#define _ZNETAGREEMENTBUILDER_H

#include "MUID.h"
#include "MMatchGlobal.h"
#include <list>
using namespace std;

class ZNetAgreementBuilder
{
private:
	int						m_nRequestID;
	MMatchProposalMode		m_nProposalMode;
	bool					m_bProposingNow;

	struct ZReplier
	{
		char	szCharName[MATCHOBJECT_NAME_LENGTH];
		bool	bAnswered;
		bool	bAgreement;
	};
	list<ZReplier*>			m_Replies;

	void ClearReplies();
	void Clear();
public:
	ZNetAgreementBuilder();
	virtual ~ZNetAgreementBuilder();
	
	bool Proposal(MMatchProposalMode nProposalMode, int nRequestID, char** ppReplierNames, int nReplierCount);
	void CancelProposal();

	enum _BuildReplyResult
	{
		BRR_WRONG_REPLY		= 0,		// 잘못온 답변
		BRR_NOT_REPLIED_ALL	= 1,		// 아직 모두 대답하지 않았다.
		BRR_ALL_AGREED		= 2,		// 모두 동의함.
		BRR_DISAGREED		= 3			// 거절한 사람이 있다.
	};

	ZNetAgreementBuilder::_BuildReplyResult BuildReply(const char* szReplierName, const MMatchProposalMode nProposalMode, 
													   int nRequestID, bool bAgreement);	// 모든 동의가 다 모아지면 return true
	MMatchProposalMode GetProposalMode()	{ return m_nProposalMode; }
	int GetReplierNames(char** ppReplierNames, int nMaxCount);	// Return값이 실제 Replier 갯수
	bool GetRejecter(char* out);
};



#endif