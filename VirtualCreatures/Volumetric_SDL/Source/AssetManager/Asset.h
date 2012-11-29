#pragma once

#include <string>

#include <System/Uncopyable.h>

class Asset :
	public Uncopyable
{
public:
	virtual ~Asset() {}
	virtual bool LoadAsset(const std::string &name);
};

