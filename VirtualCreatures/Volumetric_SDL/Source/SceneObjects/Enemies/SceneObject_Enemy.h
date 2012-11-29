#pragma once

#include <Scene/SceneObject.h>

class SceneObject_Enemy :
	public SceneObject
{
protected:
	bool m_agroed;

public:
	float m_health;

	SceneObject_Enemy();

	virtual std::string GetTypeName() = 0;
	bool IsAgroed() const;

	virtual void SetUserData(void* pUserData);
};

