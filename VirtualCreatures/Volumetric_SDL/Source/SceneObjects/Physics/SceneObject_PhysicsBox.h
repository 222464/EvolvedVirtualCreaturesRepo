#pragma once

#include <Scene/SceneObject.h>

#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>

#include <SceneObjects/SceneObject_Prop.h>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

class SceneObject_PhysicsBox :
	public SceneObject
{
private:
	// Rendering
	SceneObject_Prop* m_pModel;

	// Physics
	SceneObjectReferenceTracker m_physicsWorldTracker;
	SceneObject_PhysicsWorld* m_pPhysicsWorld;

	btCollisionShape* m_pCollisionShape;
	btDefaultMotionState* m_pMotionState;
	btRigidBody* m_pRigidBody;

	bool m_created;

public:
	SceneObject_PhysicsBox(const Vec3f &position);
	~SceneObject_PhysicsBox();

	// Inherited from SceneObject
	void OnAdd();
	void Logic();
	void Render();
};