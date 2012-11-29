#pragma once

#include <System/CSDLThread.h>

#include <queue>

// A task for a thread
struct Task
{
	bool m_taskDone;

	Task()
		: m_taskDone(false)
	{
	}

	virtual void Run() = 0;
};

// Task thread waits until it receives a new task (using condition)
// and keeps working on those until signalled to quit
class CSDLTaskThread :
	public CSDLThread
{
private:
	class CSDLThreadPool* m_pThreadPool;

	// Used to pause thread until it has a task
	SDL_sem* m_hasTaskSemaphore;

	Task* m_pCurrentTask;

	// Inherited from CSDLThread
	void Run();

	// If should continue waiting for tasks (used to close thread)
	bool m_continue;

public:
	CSDLTaskThread(class CSDLThreadPool* pThreadPool);
	~CSDLTaskThread();

	friend class CSDLThreadPool;
};

class CSDLThreadPool :
	public Uncopyable
{
private:
	std::queue<Task*> m_pTaskQueue;

	unsigned int m_numThreads;

	std::vector<CSDLTaskThread*> m_pAllThreads;

	std::queue<CSDLTaskThread*> m_pOpenThreads;

	void OpenUpThread(CSDLTaskThread* pTaskThread);

	SDL_mutex* m_poolUpdateMutex;

public:
	CSDLThreadPool(unsigned int numThreads);
	~CSDLThreadPool();

	// Pointer to boolean is used to determine when task was completed
	void AddTask(Task* pTask);

	bool HasOpenThreads() const;
	bool HasPendingTasks() const;

	friend CSDLTaskThread;
};

