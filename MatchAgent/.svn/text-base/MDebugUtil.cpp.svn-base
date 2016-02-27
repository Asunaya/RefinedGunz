#include "MDebugUtil.h"
#include "MMatchAgent.h"


void MDebugUtil_PrintStagePeerList()
{
	MMatchAgent* pServer = (MMatchAgent*)MMatchAgent::GetInstance();
	pServer->LOG(MMatchAgent::LOG_PROG, "== Stage/Peer DebugList =========");

	int nStageCount = 0;
	for(MStageAgents::iterator iStage=pServer->m_Stages.begin(); iStage!=pServer->m_Stages.end();iStage++)
	{
		MStageAgent* pStage = (*iStage).second;
		pServer->LOG(MMatchAgent::LOG_PROG, "Stage(%u%u) %d/%d ClientCnt(%d)", 
					pStage->GetUID().High,pStage->GetUID().Low, 
					++nStageCount, pServer->m_Stages.size(),
					pStage->GetObjCount());
	
		for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) 
		{
			MAgentClient* pClient = pServer->GetClient((*i).first);
			pServer->LOG(MMatchAgent::LOG_PROG, "	Obj(%u%u) PeerCnt(%d)", 
						pClient->GetUID().High, pClient->GetUID().Low, pClient->GetPeerRouteCount());
			if (pClient->GetPeerRouteCount() > 16) 
			{
				pServer->LOG(MMatchAgent::LOG_PROG, "	WARNING!! : PeerCnt(%d)", pClient->GetPeerRouteCount());

				for (list<MUID>::iterator i=pClient->GetPeerRouteBegin(); i!=pClient->GetPeerRouteEnd(); i++)
				{
					MUID uidPeer = (*i);
					pServer->LOG(MMatchAgent::LOG_PROG, "		Route(%u%u)", uidPeer.High, uidPeer.Low);
				}
			}
		}
	}

	pServer->LOG(MMatchAgent::LOG_PROG, "=========================");
}
