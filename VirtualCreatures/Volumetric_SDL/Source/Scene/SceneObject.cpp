#include <Scene/SceneObject.h>

#include <Scene/Scene.h>

SceneObject::SceneObject()
	: m_pScene(NULL), m_shouldDestroy(false),
	m_managedName(""), m_unmanagedName(""),
	m_occludable(false)
{
}

SceneObject::~SceneObject()
{
	if(IsSPTManaged())
		RemoveFromTree();

	RemoveReferences();
}

const std::string &SceneObject::GetManagedName()
{
	return m_managedName;
}

void SceneObject::OnAdd()
{
}

void SceneObject::Logic()
{
}

void SceneObject::Render()
{
}

void SceneObject::RemoveReferences()
{
	if(m_references.empty())
		return;

	for(std::unordered_set<SceneObjectReferenceTracker*>::iterator it = m_references.begin(); it != m_references.end(); it++)
		(*it)->m_pSceneObject = NULL;

	m_references.clear();
}