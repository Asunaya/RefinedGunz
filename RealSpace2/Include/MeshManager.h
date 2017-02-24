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
	bool LoadParts(std::vector<unsigned char>& File);
	void Destroy();

	RMeshNode *Get(const char *szMeshName, const char *szNodeName);
	void GetAsync(const char *szMeshName, const char *szNodeName, void* Obj,
		std::function<void(RMeshNode*)> Callback);

	void Release(RMeshNode *pNode);

	bool RemoveObject(void* Obj, bool All = true);

private:
	template <typename T>
	struct Allocation
	{
		T Obj;
		int References;
	};

	struct RMeshDeleter {
		void operator()(RMesh* ptr) const {
			// Not sure why the RMesh destructor doesn't do this
			ptr->ClearMtrl();
			delete ptr;
		}
	};

	using RMeshPtr = std::unique_ptr<RMesh, RMeshDeleter>;

	// Using a new type instead of a typedef so that the actual type name is shorter,
	// since using a typedef causes name truncation warnings.
	struct StringMap : std::unordered_map<std::string, std::string> {
		using std::unordered_map<std::string, std::string>::unordered_map;
	};

	using PartsToEluMapType = std::unordered_map<std::string, StringMap>;
	using AllocatedMeshesType = std::unordered_map<std::string, Allocation<RMeshPtr>>;
	using AllocatedNodesType = std::unordered_map<std::string, Allocation<RMeshNode*>>;

	// Decrements the reference count of an allocation, performing the appropriate deletions if it has hit zero
	void DecrementRefCount(AllocatedMeshesType::iterator);
	void DecrementRefCount(AllocatedNodesType::iterator);

	// Maps a mesh name (like "heroman1") to a map
	// The inner map maps parts names to elus for that mesh, e.g. "eq_chest_005" -> "Model/woman/woman-parts11.elu"
	PartsToEluMapType PartsToEluMap;
	// Maps mesh filenames to a mesh allocation struct, containing the RMesh pointer and the reference count
	AllocatedMeshesType AllocatedMeshes;
	// Maps parts names to a mesh node allocation struct, containing the RMeshNode pointer and the reference count
	AllocatedNodesType AllocatedNodes;

	// Mutex for accessing any part of the class, except QueuedObjs
	std::mutex mutex;

	std::vector<void*> QueuedObjs;
	// Mutex only for accessing QueuedObjs
	std::mutex ObjQueueMutex;
};

MeshManager* GetMeshManager();

struct MeshNodeDeleter {
	void operator()(RMeshNode* ptr) const {
		if (ptr)
			GetMeshManager()->Release(ptr);
	}
};

using RMeshNodePtr = std::unique_ptr<RMeshNode, MeshNodeDeleter>;

// TODO: Move this awkward thing or something
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
