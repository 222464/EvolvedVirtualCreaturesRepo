#include <SceneObjects/Physics/SceneObject_Prop_Physics_Static.h>

#include <Scene/Scene.h>

#include <assert.h>

SceneObject_Prop_Physics_Static::SceneObject_Prop_Physics_Static()
	: m_created(false)
{
	m_unmanagedName = "statProp";
}

SceneObject_Prop_Physics_Static::~SceneObject_Prop_Physics_Static()
{
	if(m_created)
	{
		if(m_physicsWorldTracker.ReferenceAlive())
			m_pPhysicsWorld->m_pDynamicsWorld->removeRigidBody(m_pRigidBody);

		delete m_pMotionState;
		delete m_pRigidBody;
	}
}

bool SceneObject_Prop_Physics_Static::Create(const std::string &modelName, const Vec3f &position, const Quaternion &rotation,
	float restitution, float friction)
{
	assert(!m_created);
	assert(GetScene() != NULL);

	Asset* pModelAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("modelOBJPhy", Model_OBJ_Physics_Static::Asset_Factory)->GetAsset(modelName, pModelAsset))
		return false;

	m_pModel = static_cast<Model_OBJ_Physics_Static*>(pModelAsset);
	m_pModel->SetRenderer(GetScene());

	// Get reference to physics world
	m_pPhysicsWorld = static_cast<SceneObject_PhysicsWorld*>(GetScene()->GetNamed_SceneObject("physWrld"));

	assert(m_pPhysicsWorld != NULL);

	m_physicsWorldTracker.Set(m_pPhysicsWorld);

	m_pMotionState = new btDefaultMotionState(btTransform(bt(rotation).normalized(), bt(position)));

	const float mass = 0.0f;

	btVector3 intertia;
	m_pModel->GetShape()->calculateLocalInertia(mass, intertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, m_pMotionState, m_pModel->GetShape(), intertia);

	rigidBodyCI.m_restitution = restitution;
	rigidBodyCI.m_friction = friction;

	m_pRigidBody = new btRigidBody(rigidBodyCI);

	m_pPhysicsWorld->m_pDynamicsWorld->addRigidBody(m_pRigidBody);

	m_pRigidBody->setUserPointer(this);

	// Default texture setting: nearest filtering
	/*for(unsigned int i = 0, size = m_pModel->GetNumMaterials(); i < size; i++)
	{
		Model_OBJ::Material* pMat = m_pModel->GetMaterial(i);
		
		pMat->m_pDiffuseMap->Bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if(pMat->m_pSpecularMap != NULL)
		{
			pMat->m_pSpecularMap->Bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		if(pMat->m_pNormalMap != NULL)
		{
			pMat->m_pNormalMap->Bind();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	}*/

	RegenAABB();

	m_created = true;

	return true;
}

void SceneObject_Prop_Physics_Static::RegenAABB()
{
	btVector3 min, max;

	m_pRigidBody->getAabb(min, max);

	m_aabb.m_lowerBound = cons(min);
	m_aabb.m_upperBound = cons(max);

	m_aabb.CalculateHalfDims();
	m_aabb.CalculateCenter();

	if(IsSPTManaged())
		TreeUpdate();
}

void SceneObject_Prop_Physics_Static::SetPosition(const Vec3f &position)
{
	assert(m_created);

	m_pRigidBody->getWorldTransform().setOrigin(bt(position));

	RegenAABB();
}

void SceneObject_Prop_Physics_Static::IncPosition(const Vec3f &increment)
{
	assert(m_created);

	m_pRigidBody->getWorldTransform().getOrigin() += bt(increment);

	RegenAABB();
}

void SceneObject_Prop_Physics_Static::SetRotation(const Quaternion &quat)
{
	assert(m_created);

	m_pRigidBody->getWorldTransform().setRotation(bt(quat));

	RegenAABB();
}

Vec3f SceneObject_Prop_Physics_Static::GetPosition()
{
	return cons(m_pRigidBody->getWorldTransform().getOrigin());
}

Quaternion SceneObject_Prop_Physics_Static::GetRotation()
{
	return cons(m_pRigidBody->getWorldTransform().getRotation());
}

bool SceneObject_Prop_Physics_Static::Created()
{
	return m_created;
}

void SceneObject_Prop_Physics_Static::Render()
{
	assert(m_created);

	Matrix4x4f glTransform;
	m_pRigidBody->getWorldTransform().getOpenGLMatrix(glTransform.m_elements);

	m_pModel->Render(glTransform);
}