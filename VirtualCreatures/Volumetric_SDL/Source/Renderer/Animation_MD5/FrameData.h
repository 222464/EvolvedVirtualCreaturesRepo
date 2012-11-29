#pragma once

#include <vector>

struct FrameData
{
	int m_frameID;
	std::vector<float> m_frameData;

	FrameData();
	FrameData(int frameID);
};

