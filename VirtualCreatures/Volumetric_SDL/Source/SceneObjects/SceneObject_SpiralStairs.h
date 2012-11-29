#pragma once

#include <Renderer/Model_OBJ.h>
#include <Scene/SceneObject.h>

#include <Utilities/UtilFuncs.h>

#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>

class SceneObject_SpiralStairs :
	public SceneObject
{
private:
	Model_OBJ* m_pStepModel;
	Model_OBJ* m_pBeamModel;

	bool m_created;

	// Stair info
	bool m_turnCW;
	int m_height;
	float m_startAngle;

	Vec3f m_basePos;

	// Physics
	SceneObjectReferenceTracker m_physicsWorldTracker;
	SceneObject_PhysicsWorld* m_pPhysicsWorld;

	btCylinderShape* m_pCylinderShape;
	btDefaultMotionState* m_pMotionState;

	btCompoundShape m_compoundShape;
	btRigidBody* m_pRigidBody;

public:
	static const float s_angleSegment;

	SceneObject_SpiralStairs();
	~SceneObject_SpiralStairs();

	bool Create(const std::string &stepModelName, const std::string &beamModelName, const std::string &physicsName, bool turnCW, int height, float startAngle, const Vec3f &basePos);
	bool Created();

	// Inherited from SceneObject
	void Render();
};

