#include <SceneObjects/Enemies/SceneObject_Enemy.h>

SceneObject_Enemy::SceneObject_Enemy()
	: m_agroed(false)
{
	m_unmanagedName = "enemy";
}

bool SceneObject_Enemy::IsAgroed() const
{
	return m_agroed;
}

void SceneObject_Enemy::SetUserData(void* pUserData)
{
}