#include "MCharacterDesc.h"

MCharacterDescManager	g_CharacterDescManager;		///< 캐릭터 Desc 리스트


MCharacterDesc::MCharacterDesc(int id, char* name)
{
	if (strlen(name) > 63)
	{
		strcpy(m_szName, name);
	}
	m_nDescID = id;
}

MCharacterDesc::~MCharacterDesc()
{

}


MCharacterDescManager::MCharacterDescManager()
{

}
MCharacterDescManager::~MCharacterDescManager()
{

}

bool MCharacterDescManager::Read(const char* szFileName)
{
	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(szFileName))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement		rootElement, chrElement, attrElement;
	char szTemp[256];
	char szName[256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTemp);
		if (szTemp[0] == '#') continue;

		if (!strcmp(szTemp, "CHARACTER"))
		{
			int id = -1;
			memset(szName, 0, 256);

			chrElement.GetAttribute(&id, "id");
			chrElement.GetAttribute(szName, "name");

			MCharacterDesc*		pCharacterDesc = new MCharacterDesc(id, szName);
			MCharacterBasicAttr	basic_attr;

			if (chrElement.FindChildNode("BasicAttr", &attrElement))
			{
				attrElement.GetAttribute(&basic_attr.nSTR, "str");
				attrElement.GetAttribute(&basic_attr.nCON, "con");
				attrElement.GetAttribute(&basic_attr.nDEX, "dex");
				attrElement.GetAttribute(&basic_attr.nAGI, "agi");
				attrElement.GetAttribute(&basic_attr.nINT, "int");
				attrElement.GetAttribute(&basic_attr.nCHA, "cha");
			}

			pCharacterDesc->SetDesc(&basic_attr);

			insert(value_type(id, pCharacterDesc));
		}
	}

	xmlIniData.Destroy();
	return true;
}

MCharacterDesc* MCharacterDescManager::Find(int nDescID)
{
	iterator itor = find(nDescID);
	if (itor != end())
	{
		return (*itor).second;
	}
	return NULL;

}

void MCharacterDescManager::Clear()
{
	if (!empty())
	{
		MCharacterDesc* pCharacterDesc = (*begin()).second;
		delete pCharacterDesc; pCharacterDesc = NULL;
		erase(begin());
	}

}