#include "stdafx.h"
#include "MMatchFriendInfo.h"
#include "MMatchServer.h"
#include "MMatchChannel.h"


MMatchFriendInfo::MMatchFriendInfo()
{


}

MMatchFriendInfo::~MMatchFriendInfo()
{
	while (m_FriendList.size() > 0) {
		MMatchFriendList::iterator i = m_FriendList.begin();
		MMatchFriendNode* pNode = (MMatchFriendNode*)(*i);
		delete pNode;
		m_FriendList.pop_front();
	}
}

bool MMatchFriendInfo::Add(unsigned long nFriendCID, unsigned short nFavorite, const char* pszName)
{
	if (Find(nFriendCID) != NULL)
		return false;

	MMatchFriendNode* pNode = new MMatchFriendNode;
	pNode->nFriendCID = nFriendCID;
	pNode->nFavorite = nFavorite;
	strcpy(pNode->szName, pszName);
	strcpy(pNode->szDescription, "");
	m_FriendList.push_back(pNode);

	return true;
}

void MMatchFriendInfo::Remove(const char* pszName)
{
	for (MMatchFriendList::iterator i=m_FriendList.begin(); i!= m_FriendList.end(); i++) 
	{
		MMatchFriendNode* pNode = (*i);
		if (stricmp(pNode->szName, pszName)==0) {
			m_FriendList.erase(i);
			delete pNode;
			return;
		}
	}
}

MMatchFriendNode* MMatchFriendInfo::Find(unsigned long nFriendCID)
{
	for (MMatchFriendList::iterator i=m_FriendList.begin(); i!= m_FriendList.end(); i++) 
	{
		MMatchFriendNode* pNode = (*i);
		if (pNode->nFriendCID == nFriendCID)
			return pNode;
	}
	return NULL;
}

MMatchFriendNode* MMatchFriendInfo::Find(const char* pszName)
{
	for (MMatchFriendList::iterator i=m_FriendList.begin(); i!= m_FriendList.end(); i++) 
	{
		MMatchFriendNode* pNode = (*i);
		if (stricmp(pNode->szName, pszName)==0)
			return pNode;
	}
	return NULL;
}

void MMatchFriendInfo::UpdateDesc()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	for (MMatchFriendList::iterator i=m_FriendList.begin(); i!= m_FriendList.end(); i++) 
	{
		MMatchFriendNode* pNode = (*i);
		pNode->szDescription[0] = NULL;

		MMatchObject* pObj = pServer->GetPlayerByName(pNode->szName);
		if (pObj) {
			char szDesc[CHANNELNAME_LEN*2]="";

			pNode->nState = pObj->GetPlace();
			MMatchChannel* pChannel = pServer->FindChannel(pObj->GetChannelUID());
			if (pChannel) {
				sprintf(szDesc, "Channel '%s'", pChannel->GetName());
				strncpy(pNode->szDescription, szDesc, MATCH_SIMPLE_DESC_LENGTH);
				pNode->szDescription[MATCH_SIMPLE_DESC_LENGTH-1] = NULL;
			} else {
				strcpy(pNode->szDescription, "Unknown Channel");
			}
		} else {
			pNode->nState = MMP_OUTSIDE;
			strcpy(pNode->szDescription, "Not Logged on");
		}
	}
}
