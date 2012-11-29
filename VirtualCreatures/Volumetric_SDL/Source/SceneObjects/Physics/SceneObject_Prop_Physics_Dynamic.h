#pragma once

#include <Renderer/Model_OBJ.h>

#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>

#include <Scene/SceneObject.h>

#include <Constructs/Quaternion.h>

class SceneObject_Prop_Physics_Dynamic :
	public SceneObject
{
private:
	bool m_created;

	Model_OBJ* m_pModel;

	void RegenAABB();

	// Physics
	SceneObjectReferenceTracker m_physicsWorldTracker;
	SceneObject_PhysicsWorld* m_pPhysicsWorld;

	btDefaultMotionState* m_pMotionState;
	btRigidBody* m_pRigidBody;

	btTransform m_transform;

public:
	SceneObject_Prop_Physics_Dynamic();
	~SceneObject_Prop_Physics_Dynamic();

	bool Create(const std::string &modelName, const std::string &physicsName,
		const Vec3f &position, const Quaternion &rotation,
		float mass, float restitution, float friction);

	void SetPosition(const Vec3f &position);
	void IncPosition(const Vec3f &increment);
	void SetRotation(const Quaternion &quat);

	Vec3f GetPosition();
	Quaternion GetRotation();

	Matrix4x4f GetTransform() const;
	Matrix4x4f GetInverseTransform() const;

	bool Created();

	// Inherited from SceneObject
	void Logic();
	void Render();
};

