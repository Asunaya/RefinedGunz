#pragma once
#include <unordered_map>
#include <unordered_set>
#include <mutex>

class MeshManager
{
public:
	// This kills the file.
	void LoadParts(std::vector<unsigned char>& File);
	RMeshNode *Get(const char *szMeshName, const char *szNodeName);
	void GetAsync(const char *szMeshName, const char *szNodeName, void* Obj, std::function<void(RMeshNode*)> Callback);
	void Release(RMeshNode *pNode);

	void OnDestroyObject(void *pObj);

private:
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> PartsToEluMap;

	template<typename T>
	struct Allocation
	{
		T *pObj;
		int nReferences;
	};

	std::unordered_map<std::string, Allocation<RMesh>> AllocatedMeshes;
	// Something mangles the filenames stored in RMeshes, so we need this to index AllocatedMeshes
	std::unordered_map<RMesh *, std::string> MeshFilenames;
	std::unordered_map<std::string, Allocation<RMeshNode>> AllocatedNodes;
	std::unordered_multimap<RMeshNode *, RMesh *> MeshNodeToMeshMap;

	std::mutex mutex;

	std::vector<void *> QueuedObjs;
	bool RemoveObject(void *);

	std::mutex ObjQueueMutex;
};