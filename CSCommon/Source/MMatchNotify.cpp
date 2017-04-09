#include "stdafx.h"
#include "MMatchNotify.h"
#include "MXml.h"
#include <crtdbg.h>
#include <map>
#include "MBaseStringResManager.h"
#include "MDebug.h"

#define MESSAGES_FILE_NAME	"system/notify.xml"

#define ZTOK_MESSAGE		"NOTIFY"
#define ZTOK_ID				"id"

using MNotifyMap = std::map<int, std::string>;

static MNotifyMap g_NotifyMap;

bool InitializeNotify(MZFileSystem *pfs)
{
	MZFile mzf;
	if (!mzf.Open(MESSAGES_FILE_NAME, pfs))
		return false;

	auto buffer = mzf.Release();

	MXmlDocument aXml;
	aXml.Create();
	if(!aXml.LoadFromMemory(buffer.get()))
	{
		MLog("InitializeNotify -- MXmlDocument::LoadFromMemory failed\n");
		return false;
	}

	auto aParent = aXml.GetDocumentElement();
	MXmlElement aChild;

	for (int i = 0, end = aParent.GetChildNodeCount(); i < end; i++)
	{
		aChild = aParent.GetChildNode(i);
		char TagName[256];
		aChild.GetTagName(TagName);
		if (_stricmp(TagName, ZTOK_MESSAGE) == 0)
		{
			int nID = 0;
			if (aChild.GetAttribute(&nID, ZTOK_ID))
			{
				_ASSERT(g_NotifyMap.find(nID) == g_NotifyMap.end());
				
				char szContents[256];
				aChild.GetContents(szContents);

				g_NotifyMap.emplace(nID, MGetStringResManager()->GetStringFromXml(szContents));
			}
		}
	}
	return true;
}

bool NotifyMessage(int nMsgID, std::string *out)
{
	auto i = g_NotifyMap.find(nMsgID);
	if (i == g_NotifyMap.end())
		return false;

	*out = i->second;

	return true;
}