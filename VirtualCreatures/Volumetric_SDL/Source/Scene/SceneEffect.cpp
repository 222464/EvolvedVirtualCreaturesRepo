#include <Scene/Scene.h>

#include <Scene/SceneEffect.h>

#include <Renderer/SDL_OpenGL.h>

SceneEffect::SceneEffect()
	: m_pScene(NULL), m_managedName("")
{
}

Scene* SceneEffect::GetScene()
{
	return m_pScene;
}

void SceneEffect::OnAdd()
{
}

void SceneEffect::Remove()
{
	GetScene()->RemoveEffect(GetLayer());
}

unsigned int SceneEffect::GetLayer()
{
	return m_layer;
}

const std::string &SceneEffect::GetManagedName()
{
	return m_managedName;
}