#pragma once

#include <System/Uncopyable.h>

#include <string>

class Scene;

class SceneEffect :
	public Uncopyable
{
private:
	Scene* m_pScene;

	unsigned int m_layer;

	std::string m_managedName;

public:
	SceneEffect();
	virtual ~SceneEffect() {};

	virtual void OnAdd();
	virtual void RunEffect() = 0;

	Scene* GetScene();

	void Remove();

	unsigned int GetLayer();

	const std::string &GetManagedName();

	friend Scene;
};

