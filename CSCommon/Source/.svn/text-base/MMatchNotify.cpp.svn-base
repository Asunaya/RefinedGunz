#include "stdafx.h"
#include "MMatchNotify.h"
#include "MXml.h"
#include <crtdbg.h>
#include <map>


#define MESSAGES_FILE_NAME	"system/notify.xml"

#define ZTOK_MESSAGE		"NOTIFY"
#define ZTOK_ID				"id"

typedef map<int, string>	MNotifyMap;

MNotifyMap g_NotifyMap;

bool InitializeNotify(MZFileSystem *pfs)
{
	MZFile mzf;
	if(!mzf.Open(MESSAGES_FILE_NAME,pfs))
		return false;

	char *buffer;
	buffer=new char[mzf.GetLength()+1];
	mzf.Read(buffer,mzf.GetLength());
	buffer[mzf.GetLength()]=0;

	MXmlDocument aXml;
	aXml.Create();
	if(!aXml.LoadFromMemory(buffer))
	{
		delete buffer;
		return false;
	}
	delete buffer;

	int iCount, i;
	MXmlElement		aParent, aChild;
	aParent = aXml.GetDocumentElement();
	iCount = aParent.GetChildNodeCount();

	char szTagName[256];
	for (i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);
		aChild.GetTagName(szTagName);
		if(stricmp(szTagName,ZTOK_MESSAGE)==0)
		{
			int nID=0;
			if(aChild.GetAttribute(&nID,ZTOK_ID))
			{
				// 이미 등록되어있는게 없어야 한다. ( 메시지 중복 )
				_ASSERT(g_NotifyMap.find(nID)==g_NotifyMap.end());		
				
				char szContents[256];
				aChild.GetContents(szContents);

				g_NotifyMap.insert(MNotifyMap::value_type(nID,string(MGetStringResManager()->GetStringFromXml(szContents))));
			}
		}
	}
	return true;
}

bool NotifyMessage(int nMsgID, string *out)
{
	MNotifyMap::iterator i=g_NotifyMap.find(nMsgID);
	if(i==g_NotifyMap.end())
		return false;

	*out = i->second;

	return true;
}