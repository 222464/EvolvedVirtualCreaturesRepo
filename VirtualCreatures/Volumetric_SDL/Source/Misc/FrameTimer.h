#pragma once

#include <list>

class FrameTimer
{
private:
	std::list<unsigned int> m_times;

	unsigned int m_bufferSize;

	unsigned int m_elapsedTime;
	
	float m_normalFrameRate;

	unsigned int m_startTime;

public:
	float m_timeMultiplier;

	FrameTimer(unsigned int bufferSize, float normalFrameRate);

	void EndFrameUpdate();

	float GetTimeMultiplier();
	float GetFrameRate();
	float GetNormalFrameRate();
	unsigned int GetElapsedTime(); // Time since last frame in milliseconds
};

