#pragma once

#include <SDL.h>

#include <System/Uncopyable.h>

class CSDLThread;

int CSDLThreadFunc(void* data);

class CSDLThread :
	private Uncopyable
{
private:
	SDL_Thread* m_pSDLThread;

public:
	enum State
	{
		e_stopped, e_running, e_waiting
	} m_state;

	CSDLThread();
	virtual ~CSDLThread();

	void Start();

	virtual void Run() = 0;

	SDL_Thread* GetSDLThread() const
	{
		return m_pSDLThread;
	}

	// Waits for the thread to finish
	void Wait();

	// Terminates thread immediately. Ugly.
	void Kill();

	int GetThreadID() const;
};

