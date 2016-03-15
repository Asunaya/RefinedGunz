#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

class TaskManager
{
public:
	TaskManager();

	template<typename T>
	void AddTask(T Task)
	{
		{
			std::lock_guard<std::mutex> lock(QueueMutex);
			Tasks.push(Task);
			bNotified = true;
		}

		cv.notify_one();
	}

	void ThreadLoop();

private:
	std::mutex QueueMutex;
	std::queue<std::function<void()>> Tasks;
	std::condition_variable cv;
	bool bNotified = false;

	std::thread thr;
};

extern TaskManager g_TaskManager;