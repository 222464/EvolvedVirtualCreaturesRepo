#include <SceneObjects/Weapons/SceneObject_Weapon.h>

void SceneObject_Weapon::Render()
{
	m_weapon.Render(Matrix4x4f::TranslateMatrix(m_position) * m_rotation.GetMatrix() * Matrix4x4f::ScaleMatrix(m_scale));
}