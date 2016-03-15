#pragma once
#include <unordered_map>

class MeshManager
{
public:
	MeshManager();
	RMeshNode *Get(const char *szMeshName);
	void Release(RMeshNode *pNode);

private:
	std::unordered_map<std::string, std::string> PartsToEluMap;

	struct Allocation
	{
		RMesh *pMesh;
		int ReferenceCount;
	};
	std::list<Allocation> Allocations;
	std::unordered_map<RMeshNode *, RMesh *> MeshNodeToMeshMap;
};

extern MeshManager *g_pMeshManager;