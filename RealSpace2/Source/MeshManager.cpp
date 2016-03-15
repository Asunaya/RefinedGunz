#include "stdafx.h"
#include "MeshManager.h"
#include <rapidxml.hpp>

MeshManager *g_pMeshManager;

MeshManager::MeshManager()
{
	MZFile file;
	file.Open("model/parts_index.xml", g_pFileSystem);
	int len = file.GetLength();
	MLog("parts_index length: %d", len);
	char *InflatedFile = new char[len];
	file.Read(InflatedFile, len);

	std::list<std::string> FileList;

	rapidxml::xml_document<> parts;
	parts.parse<0>(InflatedFile);

	rapidxml::xml_node<> *xml = parts.first_node("xml");

	for (rapidxml::xml_node<> *node = xml->first_node("parts"); node; node = node->next_sibling())
	{
		const char *szFile = node->first_attribute("file")->value();
		FileList.push_back(szFile);

		const char *parts[5];
		parts[0] = node->first_attribute("face")->value();
		parts[1] = node->first_attribute("chest")->value();
		parts[2] = node->first_attribute("hands")->value();
		parts[3] = node->first_attribute("legs")->value();
		parts[4] = node->first_attribute("feet")->value();

		for (int i = 0; i < 5; i++)
		{
			if (!parts[i])
				continue;

			PartsToEluMap.insert({ parts[i], szFile });
		}

		MLog("Loaded file %s\n", szFile);
	}
}

RMeshNode *MeshManager::Get(const char *szMeshName)
{
	auto it = PartsToEluMap.find(szMeshName);
	
	if (it == PartsToEluMap.end())
		return nullptr;

	RMesh *pMesh = new RMesh;

	pMesh->ReadElu(it->second.c_str());

	return pMesh->GetPartsNode(szMeshName);
}

void MeshManager::Release(RMeshNode *pNode)
{
	auto it = MeshNodeToMeshMap.find(pNode);

	if (it == MeshNodeToMeshMap.end())
		return;

	for (auto ait = Allocations.begin(); ait != Allocations.end(); ait++)
	{
		if (ait->pMesh == it->second)
		{
			ait->ReferenceCount--;
			if (ait->ReferenceCount <= 0)
			{
				delete ait->pMesh;
				Allocations.erase(ait);
			}
			return;
		}
	}
}