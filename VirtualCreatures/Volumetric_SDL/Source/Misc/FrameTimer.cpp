#include <Misc/FrameTimer.h>

#include <SDL.h>

FrameTimer::FrameTimer(unsigned int bufferSize, float normalFrameRate)
	: m_bufferSize(bufferSize), m_normalFrameRate(normalFrameRate),
	m_timeMultiplier(1.0f), m_startTime(0), m_elapsedTime(0)
{
	unsigned int normalFrameRateElapsedTime = static_cast<unsigned int>((1.0f / static_cast<float>(m_normalFrameRate)) * 1000.0f);

	// Fill buffer with initial values
	for(unsigned int i = 0; i < m_bufferSize; i++)
		m_times.push_back(normalFrameRateElapsedTime);
}

void FrameTimer::EndFrameUpdate()
{
	unsigned int currentTime = SDL_GetTicks();
	m_elapsedTime = currentTime - m_startTime;
	m_startTime = currentTime;

	// Add the new time, remove oldest
	m_times.push_back(m_elapsedTime);
	m_times.pop_front();

	unsigned int sum = 0;

	// Get average of times
	for(std::list<unsigned int>::iterator it = m_times.begin(); it != m_times.end(); it++)
		sum += *it;

	float averageElapsedTime = static_cast<float>(sum) / m_bufferSize;

	// Get time multiplier
	m_timeMultiplier = (averageElapsedTime / 1000.0f) * m_normalFrameRate;
}

float FrameTimer::GetTimeMultiplier()
{
	return m_timeMultiplier;
}

float FrameTimer::GetFrameRate()
{
	if(m_timeMultiplier == 0.0f)
		return m_normalFrameRate;

	return m_normalFrameRate / m_timeMultiplier;
}

float FrameTimer::GetNormalFrameRate()
{
	return m_normalFrameRate;
}

unsigned int FrameTimer::GetElapsedTime()
{
	return m_elapsedTime;
}