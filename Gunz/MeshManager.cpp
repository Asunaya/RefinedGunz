#include "stdafx.h"
#include "MeshManager.h"
#undef pi
#include <boost/property_tree/detail/rapidxml.hpp>
#include <fstream>
#include "RGMain.h"

using namespace boost::property_tree::detail;

MeshManager *g_pMeshManager;

MeshManager::MeshManager()
{
	MZFile File;

	if (!File.Open("system/parts_index.xml", g_pFileSystem))
	{
		MLog("failed to open parts_index.xml\n");
		return;
	}

	int FileLength = File.GetLength();

	if (FileLength <= 0)
	{
		MLog("Invalid parts_index.xml length\n");
		return;
	}

	std::string InflatedFile;
	InflatedFile.resize(FileLength);

	File.Read(&InflatedFile[0], FileLength);

	std::list<std::string> FileList;

	rapidxml::xml_document<> parts;

	try
	{
		parts.parse<rapidxml::parse_declaration_node | rapidxml::parse_no_data_nodes>(&InflatedFile[0]);
	}
	catch (rapidxml::parse_error &e)
	{
		MLog("RapidXML threw parse_error (%s) on parts_index.xml at %s\n", e.what(), e.where<char>());
		return;
	}

	for (auto listnode = parts.first_node("partslisting"); listnode; listnode = listnode->next_sibling("partslisting"))
	{
		auto meshattr = listnode->first_attribute("mesh");

		if (!meshattr)
			continue;

		const char *Mesh = meshattr->value();
		auto &map = PartsToEluMap.insert({ Mesh, std::unordered_map<std::string, std::string>() }).first->second;

		for (auto node = listnode->first_node("parts"); node; node = node->next_sibling())
		{
			auto fileattr = node->first_attribute("file");

			if (!fileattr)
				continue;

			const char *szFile = fileattr->value();
			FileList.push_back(szFile);

			auto attr = node->first_attribute("part");

			while (attr)
			{
				const char *parts = attr->value();

				map.insert({ parts, szFile });

				attr = attr->next_attribute("part");
			}
		}
	}
}

RMeshNode *MeshManager::Get(const char *szMeshName, const char *szNodeName)
{
	/*LARGE_INTEGER start;
	QueryPerformanceCounter(&start);*/

	std::lock_guard<std::mutex> lock(mutex);
	//MLog("Get mesh: %s, node: %s\n", szMeshName, szNodeName);

	auto allocnode = AllocatedNodes.find(szNodeName);

	if (allocnode != AllocatedNodes.end())
	{
		allocnode->second.nReferences++;

		auto meshit = MeshNodeToMeshMap.find(allocnode->second.pObj);

		if (meshit != MeshNodeToMeshMap.end())
		{
			auto allocmesh = AllocatedMeshes.find(meshit->second->GetName());

			if (allocmesh != AllocatedMeshes.end())
			{
				allocmesh->second.nReferences++;
			}
		}

		//MLog("Found already allocated node");

		return allocnode->second.pObj;
	}

	auto mapsit = PartsToEluMap.find(szMeshName);
	
	if (mapsit == PartsToEluMap.end())
	{
		//MLog("Couldn't find %s\n", szMeshName);

		return nullptr;
	}

	auto nodeit = mapsit->second.find(szNodeName);

	if (nodeit == mapsit->second.end())
	{ 
		//MLog("Couldn't find %s\n", szNodeName);

		return nullptr;
	}

	RMesh *pMesh = nullptr;

	auto allocmesh = AllocatedMeshes.find(nodeit->second);

	if (allocmesh != AllocatedMeshes.end())
	{
		pMesh = allocmesh->second.pObj;
		allocmesh->second.nReferences++;
		//MLog("Found already allocated mesh %s, ref count %d\n", pMesh->GetFileName(), allocmesh->second.nReferences);
	}
	else
	{
		pMesh = new RMesh;
		if (!pMesh->ReadElu(nodeit->second.c_str()))
		{
			MLog("Couldn't load elu %s\n", nodeit->second.c_str());

			return nullptr;
		}

		//MLog("Loaded mesh %s, %s\n", nodeit->second.c_str(), pMesh->GetFileName());

		AllocatedMeshes.insert({ nodeit->second.c_str(), { pMesh, 1 } });
		MeshFilenames.insert({ pMesh, nodeit->second.c_str() });
	}

	auto node = pMesh->GetMeshData(szNodeName);

	if (!node)
	{
		//MLog("Failed to find node %s\n", szNodeName);

		return nullptr;
	}

	MeshNodeToMeshMap.insert({ node, pMesh });

	/*LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	LARGE_INTEGER tps;
	QueryPerformanceFrequency(&tps);

	MLog("Get time: %f\n", double(end.QuadPart - start.QuadPart) / tps.QuadPart);*/

	return node;
}

void MeshManager::Release(RMeshNode *pNode)
{
	std::lock_guard<std::mutex> lock(mutex);

	//MLog("Release %s\n", pNode->GetName());

	auto it = MeshNodeToMeshMap.find(pNode);

	if (it == MeshNodeToMeshMap.end())
	{
		//MLog("MeshManager::Release(): Couldn't find mesh\n");
		return;
	}

	auto allocnode = AllocatedNodes.find(pNode->GetName());

	if (allocnode != AllocatedNodes.end())
	{
		allocnode->second.nReferences--;

		if (allocnode->second.nReferences <= 0)
		{
			AllocatedNodes.erase(allocnode);
			MeshNodeToMeshMap.erase(it);
		}
	}

	//MLog("Filename length: %d\n", it->second->m_FileName.length());
	//auto allocmesh = AllocatedMeshes.find(it->second->GetFileName());
	auto meshname = MeshFilenames.find(it->second);

	if (meshname == MeshFilenames.end())
	{
		//MLog("Couldn't find mesh\n");
		return;
	}

	auto allocmesh = AllocatedMeshes.find(meshname->second);

	if (allocmesh != AllocatedMeshes.end())
	{
		allocmesh->second.nReferences--;

		//MLog("Release mesh %s: ref count %d\n", meshname->second.c_str(), allocmesh->second.nReferences);

		if (allocmesh->second.nReferences <= 0)
		{
			//MLog("Releasing mesh %s\n", allocmesh->first.c_str());//it->second->GetFileName());

			allocmesh->second.pObj->ClearMtrl();
			delete allocmesh->second.pObj;

			AllocatedMeshes.erase(allocmesh);
			MeshFilenames.erase(meshname);

			//MLog("Released\n");
		}
	}
	else
	{
		//MLog("Couldn't find mesh %s to release\n", meshname->second.c_str());
	}
}

void MeshManager::OnDestroyObject(void *pObj)
{
	std::lock_guard<std::mutex> lock(ObjQueueMutex);

	RemoveObject(pObj);
}

bool MeshManager::RemoveObject(void *pObj)
{
	for (auto it = QueuedObjs.begin(); it != QueuedObjs.end(); it++)
	{
		if (*it == pObj)
		{
			QueuedObjs.erase(it);
			return true;
		}
	}

	return false;
}