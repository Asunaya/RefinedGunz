#include "stdafx.h"
#include "GunGame.h"

GunGame::GunGame()
{
	m_GGSet.clear();
}

GunGame::~GunGame()
{
	for (auto itor = m_GGSet.begin(); itor != m_GGSet.end(); ++itor)
	{
		for (auto vec = (*itor).second.begin(); vec != (*itor).second.end(); ++itor)
		{
			delete *vec;
		}
	}
	m_GGSet.clear();
}

GunGame* GunGame::GetInstance()
{
	static GunGame Instance;

	return &Instance;
}

bool GunGame::ReadXML(const char* szFileName, MZFileSystem* pFileSystem)
{
	MXmlDocument	xmlIniData;
	xmlIniData.Create();

	char *buffer;
	MZFile mzf;

	if (pFileSystem)
	{
		if (!mzf.Open(szFileName, pFileSystem))
		{
			if (!mzf.Open(szFileName))
			{
				xmlIniData.Destroy();
				return false;
			}
		}
	}
	else
	{
		if (!mzf.Open(szFileName))
		{
			xmlIniData.Destroy();
			return false;
		}
	}

	buffer = new char[mzf.GetLength() + 1];
	buffer[mzf.GetLength()] = 0;
	mzf.Read(buffer, mzf.GetLength());

	if (!xmlIniData.LoadFromMemory(buffer))
	{
		xmlIniData.Destroy();
		delete[] buffer;
		return false;
	}
	delete[] buffer;
	mzf.Close();

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256], szChildName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int Count = rootElement.GetChildNodeCount();

	for (int i = 0; i < Count; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!stricmp(szTagName, "SET"))
		{
			int ID;
			chrElement.GetAttribute(&ID, "id", 0);
			std::vector<GGSet*> ItemSets;
			ItemSets.clear();
			int ChildCount = chrElement.GetChildNodeCount();
			for (int j = 0; j < ChildCount; ++j)
			{
				GGSet* Node = new GGSet();
				attrElement = chrElement.GetChildNode(j);
				attrElement.GetTagName(szChildName);

				if (!stricmp(szChildName, "ITEMSET"))
				{
					if (!ParseXML_ItemSet(attrElement, Node))
					{
						delete Node;
						mlog("Error parsing itemset\n");
						return false;
					}
					ItemSets.push_back(Node);
				}
			}
			m_GGSet[ID] = ItemSets;
		}
	}

	xmlIniData.Destroy();

	return true;
}

bool GunGame::ParseXML_ItemSet(MXmlElement& elem, GGSet* Node)
{
	int Melee, Primary, Secondary;
	if (!elem.GetAttribute(&Melee, "melee", 0)) {
		return false;
	}
	if (!elem.GetAttribute(&Primary, "primary", 0)) {
		return false;
	}
	if (!elem.GetAttribute(&Secondary, "secondary", 0)) {
		return false;
	}

	Node->WeaponSet[0] = Melee;
	Node->WeaponSet[1] = Primary;
	Node->WeaponSet[2] = Secondary;

	return true;
}