#pragma once

#include <SceneObjects/Physics/DynamicCharacterController.h>

#include <SceneObjects/Enemies/SceneObject_Enemy.h>

#include <Renderer/Model_MD5/Model_MD5.h>
#include <Renderer/Animation_MD5/Animation_MD5.h>

class SceneObject_Enemy_Spawner :
	public SceneObject_Enemy
{
private:
	Vec3f m_position;

	Model_MD5* m_pModel;
	Animation_MD5* m_pIdleAnimation;

	float m_animationTime;

	// Physics
	SceneObjectReferenceTracker m_physicsWorldTracker;
	SceneObject_PhysicsWorld* m_pPhysicsWorld;

	btCollisionShape* m_pCollisionShape;
	btDefaultMotionState* m_pMotionState;
	btRigidBody* m_pRigidBody;

public:
	SceneObject_Enemy_Spawner(const Vec3f &position);
	~SceneObject_Enemy_Spawner();

	// Inherited from SceneObject
	void OnAdd();
	void Logic();
	void Render();

	// Inherited from SceneObject_Enemy
	std::string GetTypeName();
};

