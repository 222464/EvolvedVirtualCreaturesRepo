#pragma once

#include <string>

struct JointDesc
{
	std::string m_name;
	int m_parentID;
	int m_flags;
	int m_startIndex;

	JointDesc();
	JointDesc(const std::string &name, int parentID, int flags, int startIndex);
};

