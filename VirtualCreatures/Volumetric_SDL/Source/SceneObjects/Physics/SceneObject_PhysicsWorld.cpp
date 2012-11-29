#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>

#include <Scene/Scene.h>

#include <assert.h>

SceneObject_PhysicsWorld::SceneObject_PhysicsWorld()
{
	m_unmanagedName = "physWrld";

	m_pBroadphase = new btDbvtBroadphase();
	m_pCollisionConfiguration = new btDefaultCollisionConfiguration();
	m_pDispatcher = new btCollisionDispatcher(m_pCollisionConfiguration);

	m_pSolver = new btSequentialImpulseConstraintSolver();

	m_pDynamicsWorld = new btDiscreteDynamicsWorld(m_pDispatcher, m_pBroadphase, m_pSolver, m_pCollisionConfiguration);

	m_pGhostPairCallBack = new btGhostPairCallback();
	
	m_pDynamicsWorld->setGravity(btVector3(0.0f, -9.81f, 0.0f));

	m_pDynamicsWorld->getPairCache()->setInternalGhostPairCallback(m_pGhostPairCallBack);
}

SceneObject_PhysicsWorld::~SceneObject_PhysicsWorld()
{
	delete m_pDynamicsWorld;

	delete m_pSolver;

	delete m_pBroadphase;
	delete m_pCollisionConfiguration;
	delete m_pDispatcher;

	delete m_pGhostPairCallBack;
}

void SceneObject_PhysicsWorld::Logic()
{
	// Advance the simulation
	m_pDynamicsWorld->stepSimulation(btScalar(GetScene()->m_frameTimer.GetElapsedTime()) / 1000.0f, 6);
}