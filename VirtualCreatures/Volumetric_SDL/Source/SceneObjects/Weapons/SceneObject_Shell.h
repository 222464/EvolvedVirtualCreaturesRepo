#pragma once

#include <Scene/SceneObject.h>

#include <Renderer/Model_OBJ.h>

#include <Constructs/Quaternion.h>

class SceneObject_Shell :
	public SceneObject
{
private:
	Model_OBJ* m_pShellModel;

	Vec3f m_position;
	Vec3f m_velocity;

	Vec3f m_rotationAxis;

	Quaternion m_spawnRotation;
	float m_rotateAngle;
	float m_rotateSpeed;

	float m_age;

public:
	static const float s_shellMaxAge;
	static const float s_gravityAccel;
	static const float s_randomVelInfluence;
	static const float s_randomRotateSpeedMax;

	SceneObject_Shell(Model_OBJ* pShellModel, const Vec3f &spawnPosition, const Vec3f &spawnVelocity, const Quaternion &spawnRotation);

	// Inherited from SceneObject
	void Logic();
	void Render();
};

