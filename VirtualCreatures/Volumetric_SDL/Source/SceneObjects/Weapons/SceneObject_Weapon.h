#pragma once

#include <Scene/SceneObject.h>

#include <SceneObjects/Weapons/Weapon.h>

class SceneObject_Weapon :
	public SceneObject
{
public:
	Vec3f m_position;
	Quaternion m_rotation;
	Vec3f m_scale;

	Weapon m_weapon;

	SceneObject_Weapon();

	// Inherited from SceneObject
	void Render();
};

