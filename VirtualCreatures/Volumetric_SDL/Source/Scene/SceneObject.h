#pragma once

#include <System/Uncopyable.h>

#include <Renderer/Octree/OctreeOccupant.h>

#include <string>
#include <list>
#include <vector>
#include <unordered_set>
#include <assert.h>

class Scene;
class SceneObjectReferenceTracker;

class SceneObject :
	public OctreeOccupant, Uncopyable
{
private:
	Scene* m_pScene;

	std::string m_managedName;

	bool m_shouldDestroy;

	std::unordered_set<SceneObjectReferenceTracker*> m_references;

public:
	std::string m_unmanagedName;

	bool m_occludable;

	SceneObject();
	virtual ~SceneObject();

	// Inline
	Scene* GetScene()
	{
		return m_pScene;
	}

	bool IsSPTManaged()
	{
		return GetTree() != NULL; 
	}

	const std::string &GetManagedName();

	virtual void OnAdd();
	virtual void Logic();
	virtual void Render();

	void Destroy()
	{
		m_shouldDestroy = true;
	}

	bool ShouldDestroy()
	{
		return m_shouldDestroy;
	}

	void RemoveReferences();

	friend Scene;
	friend SceneObjectReferenceTracker;
};

class SceneObjectReferenceTracker
{
private:
	SceneObject* m_pSceneObject;

public:
	SceneObjectReferenceTracker()
		: m_pSceneObject(NULL)
	{
	}

	~SceneObjectReferenceTracker()
	{
		if(m_pSceneObject != NULL)
			m_pSceneObject->m_references.erase(this);
	}

	void Set(SceneObject* pSceneObject)
	{
		if(m_pSceneObject != NULL)
			m_pSceneObject->m_references.erase(this);

		m_pSceneObject = pSceneObject;

		m_pSceneObject->m_references.insert(this);
	}

	void RemoveReference()
	{
		if(m_pSceneObject != NULL)
			m_pSceneObject->m_references.erase(this);

		m_pSceneObject = NULL;
	}

	SceneObject* Get() const
	{
		return m_pSceneObject;
	}

	SceneObject* operator->() const
	{
		assert(m_pSceneObject != NULL);

		return m_pSceneObject;
	}

	bool ReferenceAlive() const
	{
		return m_pSceneObject != NULL;
	}

	friend SceneObject;
};
