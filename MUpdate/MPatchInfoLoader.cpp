#include <winsock2.h>
#include "MPatchInfoLoader.h"
#include "MPatchNode.h"
#include "MXml.h"
#include "MVersion.h"


#define MPTOK_PATCHINFO		"PATCHINFO"
#define MPTOK_VERSION		"VERSION"
#define MPTOK_PATCHNODE		"PATCHNODE"
#define MPTOK_SIZE			"SIZE"
#define MPTOK_WRITETIMEHIGH	"WRITETIMEHIGH"
#define MPTOK_WRITETIMELOW	"WRITETIMELOW"
#define MPTOK_CHECKSUM		"CHECKSUM"
#define MPTOK_ATTR_FILE		"file"


bool MPatchInfoLoader::Load(const char* pszFileName, MPatchList* pList, unsigned long* pnVersion)
{
	unsigned long nVersion = 0;

	CoInitialize(0);

	MXmlDocument	xmlConfig;
	MXmlElement		rootElement, aPatchInfo;

	xmlConfig.Create();
	if (!xmlConfig.LoadFromFile(pszFileName)) 
	{
		xmlConfig.Destroy();
		return false;
	}

	rootElement = xmlConfig.GetDocumentElement();
	if (rootElement.IsEmpty()) {
		xmlConfig.Destroy();
		return false;
	}

	if (rootElement.FindChildNode(MPTOK_PATCHINFO, &aPatchInfo) == false) {
		xmlConfig.Destroy();
		return false;
	}

	int iCount = aPatchInfo.GetChildNodeCount();
	for (int i=0; i<iCount; i++) {
		MXmlElement aPatchNode = aPatchInfo.GetChildNode(i);
		
		char szBuf[256];
		aPatchNode.GetTagName(szBuf);
		if (szBuf[0] == '#') continue;

		if (!stricmp(szBuf, MPTOK_PATCHNODE)) {
			char szFileName[_MAX_DIR] = "";
			int nSize = 0;
			FILETIME tmWrite;
			int nChecksum = 0;

			if (aPatchNode.GetAttribute(szFileName, MPTOK_ATTR_FILE) == false)
				continue;
			if (aPatchNode.GetChildContents(&nSize, MPTOK_SIZE) == false)
				continue;
			if (aPatchNode.GetChildContents((int*)&tmWrite.dwHighDateTime, MPTOK_WRITETIMEHIGH) == false)
				continue;
			if (aPatchNode.GetChildContents((int*)&tmWrite.dwLowDateTime, MPTOK_WRITETIMELOW) == false)
				continue;
			if (aPatchNode.GetChildContents(&nChecksum, MPTOK_CHECKSUM) == false)
				continue;

			pList->push_back(new MPatchNode(szFileName, nSize, tmWrite, nChecksum));
		} else if (!stricmp(szBuf, MPTOK_VERSION)) {
			char szVersion[128] = "";
			aPatchNode.GetContents(szVersion);
			nVersion = MMakeVersion(szVersion);
		}
	}

	xmlConfig.Destroy();

	if (pnVersion)
		*pnVersion = nVersion;

	return true;
}
