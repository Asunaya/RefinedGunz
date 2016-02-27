#include <winsock2.h>
#include "MAgentClient.h"


void MAgentClient::AddPeerRoute(const MUID& uid)
{
	if (ExamPeerRoute(uid) == true)
		return;

	m_PeerRouteList.push_back(uid);
}

list<MUID>::iterator MAgentClient::RemovePeerRoute(const MUID& uid)
{
	list<MUID>::iterator i = find(GetPeerRouteBegin(), GetPeerRouteEnd(), uid);
	if (i != GetPeerRouteEnd()) {
		return m_PeerRouteList.erase(i);
	} else {
		return GetPeerRouteEnd();
	}
}

bool MAgentClient::ExamPeerRoute(const MUID& uid)
{
	if (find(GetPeerRouteBegin(), GetPeerRouteEnd(), uid) != GetPeerRouteEnd())
		return true;
	else
		return false;
}

void MAgentClient::Tick(unsigned long int nTime)
{
}


bool MAgentClient::CheckDestroy(int nTime)
{

	return false;
}