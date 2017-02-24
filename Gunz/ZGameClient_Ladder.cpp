#include "stdafx.h"

#include <winsock2.h>
#include "MErrorTable.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include "MSharedCommandTable.h"
#include "MCommandLogFrame.h"
#include "ZIDLResource.h"
#include "MBlobArray.h"
#include "ZInterface.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZPost.h"
#include "MComboBox.h"
#include "MTextArea.h"
#include "MDebug.h"
#include "ZMyInfo.h"
#include "ZNetRepository.h"
#include "ZCountDown.h"

#include "ZLanguageConf.h"

#define PROPOSAL_AGREEMENT_TIMEOUT_SEC			(30)		// 30�� Ÿ�Ӿƿ�

void ShowProposalWaitFrame(bool bVisible, const char* szTitle=NULL, const char* szMessage=NULL);



void OnAskReplierAgreement_OnExpire()
{
	ZChatOutput( ZMsg(MSG_CANCELED) );
	ZGetGameClient()->ReplyAgreement(false);
}


void ShowProposalWaitFrame_OnExpire()
{
	ZChatOutput( ZMsg(MSG_CANCELED) );
}

void ShowProposalWaitFrame(bool bVisible, const char* szTitle, const char* szMessage)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementWait");
	if(pWidget!=NULL)
	{
		if (bVisible)
		{
			pWidget->SetText(szTitle);

			MTextArea* pTextArea = (MTextArea*)pResource->FindWidget("ProposalAgreementWait_Textarea");
			if (pTextArea) 
			{
				pTextArea->SetText(szMessage);
			}



			static ZCOUNTDOWN countDown = {PROPOSAL_AGREEMENT_TIMEOUT_SEC,
				                           "ProposalAgreementWait_Remain",
                                           "ProposalAgreementWait",
										   ShowProposalWaitFrame_OnExpire};

			countDown.nSeconds=PROPOSAL_AGREEMENT_TIMEOUT_SEC;	// static �̹Ƿ� �缳��
			ZApplication::GetTimer()->SetTimerEvent(0, &OnTimer_CountDown, &countDown, true);

			pWidget->Show(true, true);
		}
		else
		{
            pWidget->Show(false);
		}
	}
}


/// ������ ��ٸ��� �ִ����� ���� - ui�� üũ�Ѵ�.
bool IsWaitingRepliersAgreement()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementWait");
	if(pWidget!=NULL)
	{
		return pWidget->IsVisible();
	}
	return false;
}




///////////////////////////////////////////////////////////////////////////////////

void ZGameClient::OnResponseProposal(const int nResult, const MMatchProposalMode nProposalMode, const int nRequestID)
{
	// �κ� �ƴϸ� �ȵȴ�.
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_LOBBY) 
	{
		m_AgreementBuilder.CancelProposal();
		return;
	}


	if (nResult == MOK)
	{
		char szTitle[256];
		char szDesc[512];
		strcpy_safe( szTitle, 
			ZMsg(MSG_LADDER_PROPOSAL_WAIT_TITLE) );

		strcpy_safe(szDesc, 
			ZMsg(MSG_LADDER_PROPOSAL_WAIT_DESC) );
		// �����ҵ��� ��ٸ���� �޼���â�� ����.
		
		ShowProposalWaitFrame(true, szTitle, szDesc);
	}
	else 
	{
		m_AgreementBuilder.CancelProposal();
		ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
	}
}

void ZGameClient::OnAskAgreement(const MUID& uidProposer, void* pMemberNamesBlob,
					const MMatchProposalMode nProposalMode, const int nRequestID)
{
	// ������ �� ���� ��Ȳ�̸� �ٷ� �����Ѵ�.
	if ((IsWaitingRepliersAgreement()) || (!ZGetGameInterface()->IsReadyToPropose()))
	{
		char szCharName[MATCHOBJECT_NAME_LENGTH];
		sprintf_safe(szCharName, ZGetMyInfo()->GetCharName());
		ZPostReplyAgreement(uidProposer, GetPlayerUID(), szCharName, nProposalMode, nRequestID, false);

		return;
	}

	char szMemberNames[MAX_REPLIER][256];		// 0 ��°�� ������

	int nMemberCount = MGetBlobArrayCount(pMemberNamesBlob);
	if (nMemberCount < 1) return;

	for (int i = 0; i < nMemberCount; i++)
	{
		MTD_ReplierNode* pReplierNode = (MTD_ReplierNode*)MGetBlobArrayElement(pMemberNamesBlob, i);
		strcpy_safe(szMemberNames[i], pReplierNode->szName);
	}


	m_nProposalMode = nProposalMode;
	m_nRequestID = nRequestID;
	m_uidRequestPlayer = uidProposer;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MTextArea* pTextEdit = (MTextArea*)pResource->FindWidget("ProposalAgreementConfirm_Textarea");
	if (pTextEdit)
	{
		char szTemp[256] = "";

		char szMembers[256] = " (";

		for (int i = 0; i < nMemberCount; i++)
		{
			strcat_safe(szMembers, szMemberNames[i]);
			if (i != nMemberCount-1) strcat_safe(szMembers, ", ");
		}
		strcat_safe(szMembers, ")");

		switch (nProposalMode)
		{
		case MPROPOSAL_LADDER_INVITE:
			{
				ZTransMsg(szTemp, MSG_LADDER_REPLIER_AGREEMENT_LABEL, 1, szMemberNames[0]);
				strcat_safe(szTemp, szMembers);
			}
			break;
		case MPROPOSAL_CLAN_INVITE:
			{
				ZTransMsg(szTemp, MSG_CLANBATTLE_REPLIER_AGREEMENT_LABEL, 1, szMemberNames[0]);
				strcat_safe(szTemp, szMembers);
			}
			break;
		};

		
		pTextEdit->SetText(szTemp);
	}

	MWidget* pWidget = pResource->FindWidget("ProposalAgreementConfirm");
	if(pWidget!=NULL)
	{
		static ZCOUNTDOWN countDown = {PROPOSAL_AGREEMENT_TIMEOUT_SEC,
										"ProposalAgreementConfirm_Remain",
										"ProposalAgreementConfirm",
										OnAskReplierAgreement_OnExpire};
		countDown.nSeconds=PROPOSAL_AGREEMENT_TIMEOUT_SEC;
		ZApplication::GetTimer()->SetTimerEvent(0, &OnTimer_CountDown, &countDown, true);

		pWidget->Show(true, true);
	}
}

void ZGameClient::OnReplyAgreement(const MUID& uidProposer, 
		                const MUID& uidChar, 
						const char* szReplierName, 
						const MMatchProposalMode nProposalMode,
					    const int nRequestID, 
						const bool bAgreement)
{
	if (!IsWaitingRepliersAgreement())
	{
		m_AgreementBuilder.CancelProposal();
		return;
	}

	ZNetAgreementBuilder::_BuildReplyResult nResult;
	nResult = m_AgreementBuilder.BuildReply(szReplierName, nProposalMode, nRequestID, bAgreement);

	if (nResult == ZNetAgreementBuilder::BRR_ALL_AGREED)
	{
		switch (nProposalMode)
		{
		case MPROPOSAL_LADDER_INVITE:
			{
				char szMember[MAX_LADDER_TEAM_MEMBER][MATCHOBJECT_NAME_LENGTH];
				char* ppMember[MAX_LADDER_TEAM_MEMBER];

				for(int i=0; i<MAX_LADDER_TEAM_MEMBER; i++) {
					ppMember[i] = szMember[i];
				}
				int nCount = m_AgreementBuilder.GetReplierNames(
					&ppMember[1],
					std::size(*szMember),
					std::size(ppMember) - 1);

				strcpy_safe(szMember[0], ZGetMyInfo()->GetCharName());
				nCount++;

				ZPostLadderRequestChallenge(ppMember, nCount, 0);
			}
			break;
		case MPROPOSAL_CLAN_INVITE:
			{
				char szMember[MAX_CLANBATTLE_TEAM_MEMBER][MATCHOBJECT_NAME_LENGTH];
				char* ppMember[MAX_CLANBATTLE_TEAM_MEMBER];

				for(int i=0; i<MAX_CLANBATTLE_TEAM_MEMBER; i++) {
					ppMember[i] = szMember[i];
				}
				int nCount = m_AgreementBuilder.GetReplierNames(
					&ppMember[1],
					std::size(*szMember),
					std::size(ppMember) - 1);

				// �ڽŵ� �ִ´�.
				strcpy_safe(szMember[0], ZGetMyInfo()->GetCharName());
				nCount++;

				// Balance �ɼ�
				int nBalancedMatching = 0;
				ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
				MButton* pButton = (MButton*)pResource->FindWidget("BalancedMatchingCheckBox");
				if ((pButton) && (pButton->GetCheck()))
				{
					nBalancedMatching = 1;
				}

				// ���⼭ ������ ��û
				ZPostLadderRequestChallenge(ppMember, nCount, nBalancedMatching);
			}
			break;
		};

		ShowProposalWaitFrame(false);
		return;
	}
	else if (nResult == ZNetAgreementBuilder::BRR_DISAGREED)	// �������� ���
	{
		ShowProposalWaitFrame(false);

		char temp[256] = "";
		char szRejecter[64] = "";

		// ������ ��� ��ȯ, ������ ����� ���� ���� �׳� �������� ����� ������� ���Ѵ�.
		if (!m_AgreementBuilder.GetRejecter(szRejecter)) strcpy_safe(szRejecter, szReplierName);

		ZTransMsg(temp, MSG_LADDER_REPLIER_AGREEMENT_REJECT, 1, szRejecter);
		ZApplication::GetGameInterface()->ShowMessage(temp, NULL, MSG_LADDER_REPLIER_AGREEMENT_REJECT);
	}
}


void ZGameClient::RequestProposal(const MMatchProposalMode nProposalMode, char** ppReplierCharNames, const int nReplierCount)
{
	// ������ �°� validate �ؾ��Ѵ�.
	switch (nProposalMode)
	{
		case MPROPOSAL_LADDER_INVITE:
			{

			}
			break;
		case MPROPOSAL_CLAN_INVITE:
			{

			}
			break;
	};


	for (int i = 0; i < nReplierCount; i++)
	{
		int len = (int)strlen(ppReplierCharNames[i]);
		if ((len < 2) || (len >= MATCHOBJECT_NAME_LENGTH)) return;
	}

	m_nRequestID++;
	ZPostRequestProposal(GetPlayerUID(), int(nProposalMode), m_nRequestID, ppReplierCharNames, nReplierCount);
	m_AgreementBuilder.Proposal(nProposalMode, m_nRequestID, ppReplierCharNames, nReplierCount);
}

void ZGameClient::ReplyAgreement(const MUID& uidProposer, const MMatchProposalMode nProposalMode, bool bAgreement)
{
	char szCharName[MATCHOBJECT_NAME_LENGTH];
	sprintf_safe(szCharName, ZGetMyInfo()->GetCharName());
	ZPostReplyAgreement(uidProposer, GetPlayerUID(), szCharName, nProposalMode, m_nRequestID, bAgreement);
}

void ZGameClient::ReplyAgreement(bool bAgreement)
{
	ReplyAgreement(m_uidRequestPlayer, m_nProposalMode, bAgreement);

}

void ZGameClient::OnLadderPrepare(const MUID& uidStage, const int nTeam)
{
	m_uidStage = uidStage;
}

void ZGameClient::OnLadderLaunch(const MUID& uidStage, const char* pszMapName)
{
	ZGetGameInterface()->OnArrangedTeamGameUI(false);

	m_bLadderGame = true;
 
	m_uidStage = uidStage;
	strcpy_safe(m_szStageName, "UnNamedStage");

	SetAllowTunneling(false);

	m_MatchStageSetting.SetMapName(const_cast<char*>(pszMapName));


	// ��ȣȭ Ű ����  - Ŭ������ OnStageJoin��� ���⼭ ��ȣȭŰ�� �����Ѵ�.
	unsigned int nStageNameChecksum = m_szStageName[0] + m_szStageName[1] + m_szStageName[2] + m_szStageName[3];
	InitPeerCrypt(uidStage, nStageNameChecksum);

	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME)
	{
		ZChangeGameState(GUNZ_GAME);		// thread safely
	}
}


void ZGameClient::OnLadderResponseChallenge(const int nResult)
{
	if (nResult != MOK)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
	}
}