#include "stdafx.h"
#include "Tasks.h"

TaskManager::TaskManager()
{
	//thr = std::thread([this](){ while (true) ThreadLoop(); });
}

void TaskManager::ThreadLoop()
{
	std::unique_lock<std::mutex> lock(QueueMutex);
	cv.wait(lock, [this](){ return Notified; });

	while (Tasks.size())
	{
		Tasks.front()();
		Tasks.pop();
	}

	Notified = false;
}