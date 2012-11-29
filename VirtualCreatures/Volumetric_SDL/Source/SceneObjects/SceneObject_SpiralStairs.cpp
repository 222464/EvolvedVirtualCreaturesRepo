#include <SceneObjects/SceneObject_SpiralStairs.h>

#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>

#include <SceneObjects/Physics/DynamicConcaveShape.h>

#include <Scene/Scene.h>

#include <assert.h>

const float SceneObject_SpiralStairs::s_angleSegment = pif / 4.0f;

SceneObject_SpiralStairs::SceneObject_SpiralStairs()
	: m_created(false)
{
}

SceneObject_SpiralStairs::~SceneObject_SpiralStairs()
{
	if(m_created)
	{
		if(m_physicsWorldTracker.ReferenceAlive())
			m_pPhysicsWorld->m_pDynamicsWorld->removeRigidBody(m_pRigidBody);

		delete m_pCylinderShape;
		delete m_pMotionState;
		delete m_pRigidBody;
	}
}

bool SceneObject_SpiralStairs::Create(const std::string &stepModelName, const std::string &beamModelName, const std::string &physicsName, bool turnCW, int height, float startAngle, const Vec3f &basePos)
{
	assert(!m_created);

	Scene* pScene = GetScene();

	assert(pScene != NULL);

	Asset* pAsset;

	if(!pScene->GetAssetManager_AutoCreate("modelOBJ", Model_OBJ::Asset_Factory)->GetAsset(stepModelName, pAsset))
		return false;

	m_pStepModel = static_cast<Model_OBJ*>(pAsset);

	m_pStepModel->SetRenderer(GetScene());

	DynamicConcaveShape* pStepShape;

	if(!GetScene()->GetAssetManager_AutoCreate("physShape", DynamicConcaveShape::Asset_Factory)->GetAsset(physicsName, pAsset))
		return false;

	pStepShape = static_cast<DynamicConcaveShape*>(pAsset);

	// Texture filtering
	m_pStepModel->GetMaterial(0)->m_pDiffuseMap->Bind();

	Asset* pBeamModelAsset;

	if(!pScene->GetAssetManager("modelOBJ")->GetAsset(beamModelName, pBeamModelAsset))
		return false;

	m_pBeamModel = static_cast<Model_OBJ*>(pBeamModelAsset);

	m_pBeamModel->SetRenderer(GetScene());

	m_turnCW = turnCW;
	m_height = height;

	if(m_height <= 0)
	{
		std::cerr << "Invalid stair height \"" << m_height << "\"!" << std::endl;

		return false;
	}

	m_startAngle = startAngle;

	m_basePos = basePos;

	// Create AABB
	m_aabb.SetDims(Vec3f(6.0f, static_cast<float>(m_height), 6.0f));
	m_aabb.SetCenter(basePos + Vec3f(0.0f, m_height / 2.0f, 0.0f));

	if(IsSPTManaged())
		TreeUpdate();

	// -------------------------------- Physics --------------------------------

	// Get reference to physics world
	m_pPhysicsWorld = static_cast<SceneObject_PhysicsWorld*>(pScene->GetNamed_SceneObject("physWrld"));

	assert(m_pPhysicsWorld != NULL);

	m_physicsWorldTracker.Set(m_pPhysicsWorld);

	m_pCylinderShape = new btCylinderShape(btVector3(0.12f, m_height / 2.0f, 0.12f));

	int numSegments = m_height * 2 - 1; // One less since last "step" is the next floor

	if(m_turnCW)
	{
		for(int segment = 1; segment < numSegments; segment++) // Start at 1 because the first "step" is the floor
		{
			Matrix4x4f transform(Matrix4x4f::TranslateMatrix(m_basePos + Vec3f(0.0f, segment / 2.0f, 0.0f)) * Matrix4x4f::RotateMatrix_Y(m_startAngle - segment * s_angleSegment));
			
			btTransform trans;
			trans.setFromOpenGLMatrix(transform.m_elements);

			// Add step
			m_compoundShape.addChildShape(trans, pStepShape->GetShape());
		}
	}
	else
	{
		for(int segment = 0; segment < numSegments; segment++)
		{
			Matrix4x4f transform(Matrix4x4f::TranslateMatrix(m_basePos + Vec3f(0.0f, segment / 2.0f, 0.0f)) * Matrix4x4f::RotateMatrix_Y(m_startAngle + segment * s_angleSegment));

			btTransform trans;
			trans.setFromOpenGLMatrix(transform.m_elements);

			// Add step
			m_compoundShape.addChildShape(trans, pStepShape->GetShape());
		}
	}

	btTransform cylinderTransform;
	cylinderTransform.setIdentity();
	cylinderTransform.setOrigin(bt(m_basePos + Vec3f(0.0f, m_height / 2.0f, 0.0f)));

	m_compoundShape.addChildShape(cylinderTransform, m_pCylinderShape);

	m_pMotionState = new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, 0.0f, 0.0f)));

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0.0f, m_pMotionState, &m_compoundShape, btVector3(0.0f, 0.0f, 0.0f));

	rigidBodyCI.m_restitution = 0.0f;
	rigidBodyCI.m_friction = 0.3f;

	m_pRigidBody = new btRigidBody(rigidBodyCI);

	m_pPhysicsWorld->m_pDynamicsWorld->addRigidBody(m_pRigidBody);

	m_pRigidBody->setUserPointer(this);

	m_created = true;

	return true;
}

bool SceneObject_SpiralStairs::Created()
{
	return m_created;
}

void SceneObject_SpiralStairs::Render()
{
	assert(m_created);

	Scene* pScene = GetScene();

	// Render steps
	int numSegments = m_height * 2 - 1; // One less since last "step" is the next floor

	if(m_turnCW)
	{
		for(int segment = 1; segment < numSegments; segment++) // Start at 1 because the first "step" is the floor
			m_pStepModel->Render(Matrix4x4f::TranslateMatrix(m_basePos + Vec3f(0.0f, segment / 2.0f, 0.0f)) * Matrix4x4f::RotateMatrix_Y(m_startAngle - segment * s_angleSegment));
	}
	else
	{
		for(int segment = 0; segment < numSegments; segment++)
			m_pStepModel->Render(Matrix4x4f::TranslateMatrix(m_basePos + Vec3f(0.0f, segment / 2.0f, 0.0f)) * Matrix4x4f::RotateMatrix_Y(m_startAngle + segment * s_angleSegment));
	}

	// Render beams
	for(int h = 0; h < m_height; h++)
		m_pBeamModel->Render(Matrix4x4f::TranslateMatrix(m_basePos + Vec3f(0.0f, static_cast<float>(h), 0.0f)));
}