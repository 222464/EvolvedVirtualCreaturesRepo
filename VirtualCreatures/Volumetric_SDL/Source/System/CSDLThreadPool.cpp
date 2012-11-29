#include <System/CSDLThreadPool.h>

#include <iostream>

#include <assert.h>

CSDLTaskThread::CSDLTaskThread(CSDLThreadPool* pThreadPool)
	: m_continue(true), m_pThreadPool(pThreadPool), m_pCurrentTask(NULL)
{
	// Initialize semaphore to block
	m_hasTaskSemaphore = SDL_CreateSemaphore(0);
}

CSDLTaskThread::~CSDLTaskThread()
{
	SDL_DestroySemaphore(m_hasTaskSemaphore);
}

void CSDLTaskThread::Run()
{
	while(m_continue)
	{
		// Wait until the pool sets the semaphore value to 0
		if(SDL_SemWait(m_hasTaskSemaphore) == -1)
		{
			std::cerr << "Semaphore wait failed in thread pool! Exiting..." << std::endl;

			return;
		}

		if(!m_continue)
			return;

		// Run the task
		m_pCurrentTask->Run();
		
		m_pCurrentTask->m_taskDone = true;

		// Lock in order to interact with the pool without conflicting with other threads that want to do the same
		// This is unlocked again when the new task signal has been received
		SDL_mutexP(m_pThreadPool->m_poolUpdateMutex);

		// Let the pool know the thread is available again
		m_pThreadPool->OpenUpThread(this);

		SDL_mutexV(m_pThreadPool->m_poolUpdateMutex);
	}
}

CSDLThreadPool::CSDLThreadPool(unsigned int numThreads)
{
	m_poolUpdateMutex = SDL_CreateMutex();

	// Add all threads to the open thread queue
	for(unsigned int i = 0; i < numThreads; i++)
	{
		CSDLTaskThread* pThread = new CSDLTaskThread(this);

		m_pAllThreads.push_back(pThread);

		m_pOpenThreads.push(pThread);

		pThread->Start();
	}
}

CSDLThreadPool::~CSDLThreadPool()
{
	for(unsigned int i = 0, size = m_pAllThreads.size(); i < size; i++)
	{
		m_pAllThreads[i]->m_continue = false;

		// Unlock all semaphores to allow threads to finish cleanly
		SDL_SemPost(m_pAllThreads[i]->m_hasTaskSemaphore);

		m_pAllThreads[i]->Wait();

		delete m_pAllThreads[i];
	}

	SDL_DestroyMutex(m_poolUpdateMutex);
}

void CSDLThreadPool::AddTask(Task* pTask)
{
	assert(pTask != NULL);

	// If there are no open threads, add to queue
	if(m_pOpenThreads.empty())
		m_pTaskQueue.push(pTask);
	else  // No threads currently available
	{
		// Should be no tasks in queue if threads are open
		assert(m_pTaskQueue.empty());

		// Assign task to thread
		CSDLTaskThread* pOpenThread = m_pOpenThreads.front();

		m_pOpenThreads.pop();

		pOpenThread->m_pCurrentTask = pTask;

		// Unlock semaphore, allowing thread to proceed
		SDL_SemPost(pOpenThread->m_hasTaskSemaphore);
	}
}

void CSDLThreadPool::OpenUpThread(CSDLTaskThread* pTaskThread)
{
	// If more tasks are waiting, assign one immediately
	if(!m_pTaskQueue.empty())
	{
		Task* pTask = m_pTaskQueue.front();

		m_pTaskQueue.pop();

		pTaskThread->m_pCurrentTask = pTask;

		// Unlock semaphore, allowing thread to proceed
		SDL_SemPost(pTaskThread->m_hasTaskSemaphore);
	}
	else // Add to open thread queue
		m_pOpenThreads.push(pTaskThread);
}

bool CSDLThreadPool::HasOpenThreads() const
{
	return !m_pOpenThreads.empty();
}

bool CSDLThreadPool::HasPendingTasks() const
{
	return !m_pTaskQueue.empty();
}