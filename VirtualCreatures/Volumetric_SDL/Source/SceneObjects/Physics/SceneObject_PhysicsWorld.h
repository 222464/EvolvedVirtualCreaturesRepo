#pragma once

#include <Scene/SceneObject.h>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include <Constructs/BulletConversions.h>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

class SceneObject_PhysicsWorld :
	public SceneObject
{
public:
	btBroadphaseInterface* m_pBroadphase;
	btDefaultCollisionConfiguration* m_pCollisionConfiguration;
    btCollisionDispatcher* m_pDispatcher;

	btSequentialImpulseConstraintSolver* m_pSolver;
 
    btDiscreteDynamicsWorld* m_pDynamicsWorld;

	btGhostPairCallback* m_pGhostPairCallBack;

	SceneObject_PhysicsWorld();
	~SceneObject_PhysicsWorld();

	// Inherited from SceneObject
	void Logic();
};

