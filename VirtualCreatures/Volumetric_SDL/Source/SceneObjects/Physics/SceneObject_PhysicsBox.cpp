#include <SceneObjects/Physics/SceneObject_PhysicsBox.h>

#include <Scene/Scene.h>

SceneObject_PhysicsBox::SceneObject_PhysicsBox(const Vec3f &position)
	: m_created(false)
{
	m_aabb.SetCenter(position);
}

SceneObject_PhysicsBox::~SceneObject_PhysicsBox()
{
	if(m_created)
	{
		if(m_physicsWorldTracker.ReferenceAlive())
			m_pPhysicsWorld->m_pDynamicsWorld->removeRigidBody(m_pRigidBody);

		delete m_pCollisionShape;
		delete m_pMotionState;
		delete m_pRigidBody;
	}
}

void SceneObject_PhysicsBox::OnAdd()
{
	// Rendering
	m_pModel = new SceneObject_Prop();
	GetScene()->Add(m_pModel, true);
	m_pModel->Create("data/models/crate1.obj");

	// Get reference to physics world
	m_pPhysicsWorld = static_cast<SceneObject_PhysicsWorld*>(GetScene()->GetNamed_SceneObject("physWrld"));

	m_physicsWorldTracker.Set(m_pPhysicsWorld);

	assert(m_pPhysicsWorld != NULL);

	// Physics
	m_pCollisionShape = new btBoxShape(bt(m_pModel->GetAABB().GetHalfDims()));

	m_pMotionState = new btDefaultMotionState(btTransform(btQuaternion(1.0f, 0.0f, 0.0f, 0.0f).normalized(), bt(m_aabb.GetCenter())));

	const float mass = 1.0f;

	btVector3 intertia;
	m_pCollisionShape->calculateLocalInertia(mass, intertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, m_pMotionState, m_pCollisionShape, intertia);

	m_pRigidBody = new btRigidBody(rigidBodyCI);

	m_pPhysicsWorld->m_pDynamicsWorld->addRigidBody(m_pRigidBody);

	m_created = true;
}

void SceneObject_PhysicsBox::Logic()
{
	assert(m_created);

	btTransform transform;
	m_pMotionState->getWorldTransform(transform);
	
	// Update box model
	m_pModel->SetPosition(cons(transform.getOrigin()));

	// For some reason inverse quaternion is the graphics quaternion
	m_pModel->SetRotation(cons(transform.getRotation().inverse()));
}

void SceneObject_PhysicsBox::Render()
{
	//GetScene()->SetWorldMatrix(Matrix4x4f::IdentityMatrix());
}