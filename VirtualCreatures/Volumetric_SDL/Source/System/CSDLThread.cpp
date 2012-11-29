#include <System/CSDLThread.h>

#include <iostream>

#include <assert.h>

int CSDLThreadFunc(void* data)
{
	CSDLThread* pCThread = static_cast<CSDLThread*>(data);

	pCThread->m_state = CSDLThread::e_running;

	pCThread->Run();

	pCThread->m_state = CSDLThread::e_stopped;

	return 0;
}

CSDLThread::CSDLThread()
	: m_pSDLThread(NULL), m_state(e_stopped)
{
}

CSDLThread::~CSDLThread()
{
	Wait();
}

void CSDLThread::Start()
{
	assert(m_pSDLThread == NULL);

	m_pSDLThread = SDL_CreateThread(CSDLThreadFunc, this);

	if(m_pSDLThread == NULL)
		std::cerr << "Error while trying to create SDL thread: " << SDL_GetError() << std::endl;
}

void CSDLThread::Wait()
{
	if(m_state != e_running)
		return;

	m_state = e_waiting;

	SDL_WaitThread(m_pSDLThread, NULL);

	m_state = e_stopped;
}

void CSDLThread::Kill()
{
	SDL_WaitThread(m_pSDLThread, NULL);

	m_state = e_stopped;
}

int CSDLThread::GetThreadID() const
{
	return SDL_GetThreadID(m_pSDLThread);
}