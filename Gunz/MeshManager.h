#pragma once
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "Tasks.h"

class MeshManager
{
public:
	MeshManager();
	RMeshNode *Get(const char *szMeshName, const char *szNodeName);
	template<typename T_this, typename T_callback>
	void GetAsync(const char *szMeshName, const char *szNodeName, T_this pThis, T_callback Callback);
	void Release(RMeshNode *pNode);

	void OnDestroyObject(void *pObj);

private:
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> PartsToEluMap;

	template<typename T>
	struct Allocation
	{
		T *pObj;
		int nReferences;

		void Inc() { nReferences++; }
		void Dec() { nReferences--; }
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

template<typename T_this, typename T_callback>
void MeshManager::GetAsync(const char *szMeshName, const char *szNodeName, T_this pThis, T_callback Callback)
{
	std::unique_lock<std::mutex> lock(ObjQueueMutex);

	QueuedObjs.push_back(pThis);

	lock.unlock();

	std::string PersistentMesh(szMeshName), PersistentNode(szNodeName);

	g_TaskManager.AddTask([this, PersistentMesh, PersistentNode, pThis, Callback](){
		auto ret = Get(PersistentMesh.c_str(), PersistentNode.c_str());

		g_RGMain.Invoke([=](){
			// We want to make sure the object hasn't been destroyed between the GetAsync call and the invokation.
			std::lock_guard<std::mutex> lock(ObjQueueMutex);

			bool bFound = RemoveObject(pThis);

			if(bFound)
				Callback(ret);
		});
	});
}

extern MeshManager *g_pMeshManager;