#pragma once
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <memory>
#include <queue>
#include <thread>
#include <condition_variable>

class MeshManager
{
public:
	// This kills the file.
	void LoadParts(std::vector<unsigned char>& File);
	RMeshNode *Get(const char *szMeshName, const char *szNodeName);
	void GetAsync(const char *szMeshName, const char *szNodeName, void* Obj,
		std::function<void(RMeshNode*)> Callback);
	void Release(RMeshNode *pNode);

	void OnDestroyObject(void *pObj);

	friend MeshManager* GetMeshManager()
	{
		static MeshManager Instance;

		return &Instance;
	}

private:
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> PartsToEluMap;

	template <typename T>
	struct Allocation
	{
		T* Obj;
		int References;
	};

	std::unordered_map<std::string, Allocation<RMesh>> AllocatedMeshes;
	// Something mangles the filenames stored in RMeshes, so we need this to index AllocatedMeshes
	std::unordered_map<RMesh *, std::string> MeshFilenames;
	std::unordered_map<std::string, Allocation<RMeshNode>> AllocatedNodes;
	std::unordered_multimap<RMeshNode *, RMesh *> MeshNodeToMeshMap;

	std::mutex mutex;

	std::vector<void*> QueuedObjs;
	bool RemoveObject(void*);

	std::mutex ObjQueueMutex;
};

MeshManager* GetMeshManager();

class TaskManager
{
public:
	TaskManager();

	template<typename T>
	void AddTask(T&& Task)
	{
		{
			std::lock_guard<std::mutex> lock(QueueMutex[0]);
			Tasks.push(Task);
			Notified = true;
		}

		cv.notify_one();
	}

	template<typename T>
	void Invoke(T&& Task)
	{
		std::lock_guard<std::mutex> lock(QueueMutex[1]);
		Invokations.push(Task);
	}

	void Update(float Elapsed);
	void ThreadLoop();

	static TaskManager& GetInstance()
	{
		static TaskManager Instance;

		return Instance;
	}

private:
	std::mutex QueueMutex[2];
	std::queue<std::function<void()>> Tasks;
	std::queue<std::function<void()>> Invokations;
	std::condition_variable cv;
	bool Notified = false;

	std::thread thr;
};