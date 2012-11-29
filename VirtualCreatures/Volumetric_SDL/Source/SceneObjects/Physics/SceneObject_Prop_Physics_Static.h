#pragma once

#include <Renderer/Model_OBJ_Physics_Static.h>

#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>

#include <Scene/SceneObject.h>

#include <Constructs/Quaternion.h>

class SceneObject_Prop_Physics_Static :
	public SceneObject
{
private:
	bool m_created;

	Model_OBJ_Physics_Static* m_pModel;

	// Physics
	SceneObjectReferenceTracker m_physicsWorldTracker;
	SceneObject_PhysicsWorld* m_pPhysicsWorld;

	btDefaultMotionState* m_pMotionState;
	btRigidBody* m_pRigidBody;

public:
	SceneObject_Prop_Physics_Static();
	~SceneObject_Prop_Physics_Static();

	bool Create(const std::string &modelName, const Vec3f &position, const Quaternion &rotation, float restitution, float friction);

	void SetPosition(const Vec3f &position);
	void IncPosition(const Vec3f &increment);
	void SetRotation(const Quaternion &quat);

	// Call manually
	void RegenAABB();

	Vec3f GetPosition();
	Quaternion GetRotation();

	bool Created();

	// Inherited from SceneObject
	void Render();
};

