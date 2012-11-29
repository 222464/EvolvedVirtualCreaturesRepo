#include <SceneObjects/Physics/SceneObject_Prop_Physics_Dynamic.h>

#include <SceneObjects/Physics/DynamicConcaveShape.h>

#include <Scene/Scene.h>

#include <assert.h>

SceneObject_Prop_Physics_Dynamic::SceneObject_Prop_Physics_Dynamic()
	: m_created(false)
{
	m_unmanagedName = "dynProp";
}

SceneObject_Prop_Physics_Dynamic::~SceneObject_Prop_Physics_Dynamic()
{
	if(m_created)
	{
		if(m_physicsWorldTracker.ReferenceAlive())
			m_pPhysicsWorld->m_pDynamicsWorld->removeRigidBody(m_pRigidBody);

		delete m_pMotionState;
		delete m_pRigidBody;
	}
}

bool SceneObject_Prop_Physics_Dynamic::Create(const std::string &modelName, const std::string &physicsName,
		const Vec3f &position, const Quaternion &rotation,
		float mass, float restitution, float friction)
{
	assert(!m_created);
	assert(GetScene() != NULL);

	Asset* pAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("modelOBJ", Model_OBJ::Asset_Factory)->GetAsset(modelName, pAsset))
		return false;

	m_pModel = static_cast<Model_OBJ*>(pAsset);
	m_pModel->SetRenderer(GetScene());

	if(!GetScene()->GetAssetManager_AutoCreate("physShape", DynamicConcaveShape::Asset_Factory)->GetAsset(physicsName, pAsset))
		return false;

	DynamicConcaveShape* pConcaveShape = static_cast<DynamicConcaveShape*>(pAsset);

	// Get reference to physics world
	m_pPhysicsWorld = static_cast<SceneObject_PhysicsWorld*>(GetScene()->GetNamed_SceneObject("physWrld"));

	assert(m_pPhysicsWorld != NULL);

	m_physicsWorldTracker.Set(m_pPhysicsWorld);

	m_pMotionState = new btDefaultMotionState(btTransform(bt(rotation).normalized(), bt(position)));

	btVector3 intertia;
	pConcaveShape->GetShape()->calculateLocalInertia(mass, intertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, m_pMotionState, pConcaveShape->GetShape(), intertia);

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

	m_created = true;

	return true;
}

void SceneObject_Prop_Physics_Dynamic::RegenAABB()
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

void SceneObject_Prop_Physics_Dynamic::SetPosition(const Vec3f &position)
{
	assert(m_created);

	m_pRigidBody->getWorldTransform().setOrigin(bt(position));
}

void SceneObject_Prop_Physics_Dynamic::IncPosition(const Vec3f &increment)
{
	assert(m_created);

	m_pRigidBody->getWorldTransform().getOrigin() += bt(increment);
}

void SceneObject_Prop_Physics_Dynamic::SetRotation(const Quaternion &quat)
{
	assert(m_created);

	m_pRigidBody->getWorldTransform().setRotation(bt(quat));
}

Vec3f SceneObject_Prop_Physics_Dynamic::GetPosition()
{
	return cons(m_transform.getOrigin());
}

Quaternion SceneObject_Prop_Physics_Dynamic::GetRotation()
{
	return cons(m_transform.getRotation());
}

bool SceneObject_Prop_Physics_Dynamic::Created()
{
	return m_created;
}

void SceneObject_Prop_Physics_Dynamic::Logic()
{
	m_pMotionState->getWorldTransform(m_transform);

	RegenAABB();
}

void SceneObject_Prop_Physics_Dynamic::Render()
{
	assert(m_created);

	Matrix4x4f glTransform;
	m_transform.getOpenGLMatrix(glTransform.m_elements);

	m_pModel->Render(glTransform);
}

Matrix4x4f SceneObject_Prop_Physics_Dynamic::GetTransform() const
{
	Matrix4x4f mat;
	m_transform.getOpenGLMatrix(mat.m_elements);
	return mat;
}

Matrix4x4f SceneObject_Prop_Physics_Dynamic::GetInverseTransform() const
{
	btTransform inv(m_transform.inverse());
	return Matrix4x4f::TranslateMatrix(cons(inv.getOrigin())) * cons(m_transform.getRotation()).GetMatrix();
}