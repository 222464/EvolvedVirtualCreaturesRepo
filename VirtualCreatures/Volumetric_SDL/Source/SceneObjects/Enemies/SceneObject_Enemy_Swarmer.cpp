#include <SceneObjects/Enemies/SceneObject_Enemy_Swarmer.h>

#include <Utilities/UtilFuncs.h>

#include <SceneObjects/Particles/SceneObject_ParticleEmitter_Sprite_Point.h>

#include <Scene/Scene.h>

int SceneObject_Enemy_Swarmer::s_numSwarmers = 0;

SceneObject_Enemy_Swarmer::SceneObject_Enemy_Swarmer(const Vec3f &position)
	: m_animationTime(0.0f), m_faceAngleXZ(0.0f), m_position(position),
	m_agroRange(20.0f), m_pRigidBody(NULL), m_splitTimer(0.0f), m_splitTime(GetSplitTime())
{
	m_health = 30.0f;

	m_aabb.SetHalfDims(Vec3f(0.2f, 0.2f, 0.2f));
	m_aabb.SetCenter(m_position);

	s_numSwarmers++;
}

SceneObject_Enemy_Swarmer::~SceneObject_Enemy_Swarmer()
{
	if(m_pRigidBody != NULL)
	{
		if(m_physicsWorldTracker.ReferenceAlive())
			m_pPhysicsWorld->m_pDynamicsWorld->removeRigidBody(m_pRigidBody);

		delete m_pCollisionShape;
		delete m_pMotionState;
		delete m_pRigidBody;
	}

	s_numSwarmers--;

	if(m_particleEffectTracker.ReferenceAlive())
		m_particleEffectTracker->Destroy();
}

void SceneObject_Enemy_Swarmer::OnAdd()
{
	// Load models
	Asset* pAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("md5model", Model_MD5::Asset_Factory)->GetAsset("data/models/enemies/swarmer.md5mesh", pAsset))
		abort();

	m_pModel = static_cast<Model_MD5*>(pAsset);

	if(!GetScene()->GetAssetManager_AutoCreate("md5anim", Animation_MD5::Asset_Factory)->GetAsset("data/models/enemies/swarmer.md5anim", pAsset))
		abort();

	m_pAnimation = static_cast<Animation_MD5*>(pAsset);

	// Set the animation
	m_pModel->m_pAnimation = m_pAnimation;

#ifdef DEBUG
	// Check it
	m_pModel->CheckAnimation();
#endif

	m_pModel->SetRenderer(GetScene());

	// Get reference to player
	m_pPlayer = static_cast<SceneObject_Player*>(GetScene()->GetNamed_SceneObject("player"));

	assert(m_pPlayer != NULL);

	// Get reference to physics world
	m_pPhysicsWorld = static_cast<SceneObject_PhysicsWorld*>(GetScene()->GetNamed_SceneObject("physWrld"));

	assert(m_pPhysicsWorld != NULL);

	m_physicsWorldTracker.Set(m_pPhysicsWorld);

	// ----------------------------------- Physics -----------------------------------

	m_pCollisionShape = new btBoxShape(bt(m_aabb.GetHalfDims()));

	m_pMotionState = new btDefaultMotionState(btTransform(btQuaternion(1.0f, 0.0f, 0.0f, 0.0f).normalized(), bt(m_position)));

	const float mass = 1.0f;

	btVector3 intertia;
	m_pCollisionShape->calculateLocalInertia(mass, intertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, m_pMotionState, m_pCollisionShape, intertia);

	rigidBodyCI.m_linearDamping = 0.6f;
	rigidBodyCI.m_angularDamping = 0.6f;

	m_pRigidBody = new btRigidBody(rigidBodyCI);

	// Keep upright
	m_pRigidBody->setAngularFactor(btVector3(0.0f, 1.0f, 0.0f));

	m_pPhysicsWorld->m_pDynamicsWorld->addRigidBody(m_pRigidBody);

	// No gravity for swarmer
	m_pRigidBody->setGravity(btVector3(0.0f, 0.0f, 0.0f));

	m_pRigidBody->setUserPointer(this);

	// ------------------------------ Particle Effects ------------------------------

	m_pParticleEffect = new SceneObject_ParticleEmitter_Sprite_Point();

	GetScene()->Add(m_pParticleEffect, true);

	m_pParticleEffect->m_emit = true;
	m_pParticleEffect->m_autoDestruct = false;
	m_pParticleEffect->m_minSpawnTime = 3.0f;
	m_pParticleEffect->m_maxSpawnTime = 5.0f;
	m_pParticleEffect->m_minDespawnTime = 20.0f;
	m_pParticleEffect->m_maxDespawnTime = 30.0f;
	m_pParticleEffect->m_minSpeed = 0.0f;
	m_pParticleEffect->m_maxSpeed = 0.05f;

	if(!GetScene()->GetAssetManager_AutoCreate("tex", Asset_Texture::Asset_Factory)->GetAsset("data/particles/gas1.png", pAsset))
		abort();

	Asset_Texture* pParticleTexture = static_cast<Asset_Texture*>(pAsset);

	assert(pParticleTexture != NULL);

	Sprite s;

	s.Create(pParticleTexture, 0.15f, 1, 1, 1);

	m_pParticleEffect->AddSprite(s);

	m_particleEffectTracker.Set(m_pParticleEffect);
}

void SceneObject_Enemy_Swarmer::Logic()
{
	Scene* pScene = GetScene();

	btTransform transform;
	m_pMotionState->getWorldTransform(transform);

	m_rotation = cons(transform.getRotation());
	m_position = cons(transform.getOrigin());

	// Death
	if(m_health <= 0.0f)
	{
		Die();
		return;
	}

	if(m_agroed)
	{
		// Follow player
		Vec3f accel = m_pPlayer->GetPosition() - m_position;

		float dist = accel.Magnitude();

		accel = accel / dist * 25.0f;

		float speed = m_pRigidBody->getLinearVelocity().length();

		if(dist > 2.0f && speed < 5.0f)
		{
			m_pRigidBody->applyCentralForce(bt(accel));
			m_pRigidBody->activate();
		}

		// Get facing angle from accel
		float newFaceAngleXZ = atan2f(accel.z, accel.x);

		m_faceAngleXZ += (newFaceAngleXZ - m_faceAngleXZ) / 10.0f * pScene->m_frameTimer.GetTimeMultiplier();

		btQuaternion newRotation;

		newRotation.setEuler(0.0f, newFaceAngleXZ, 0.0f);

		m_pRigidBody->getWorldTransform().setRotation(newRotation);
	}
	else // If not agro, check for agro (player and fellow enemies)
	{
		// Query the agro range for players or for other enemies
		AABB agroAABB;
		
		agroAABB.SetCenter(m_position);
		agroAABB.SetHalfDims(Vec3f(m_agroRange, m_agroRange, m_agroRange));

		std::vector<OctreeOccupant*> result;

		GetScene()->m_spt.Query_Region(agroAABB, result);

		for(unsigned int i = 0, size = result.size(); i < size; i++)
		{
			SceneObject* pSceneObject = static_cast<SceneObject*>(result[i]);

			if(pSceneObject->GetManagedName() == "player")
			{
				m_agroed = true;

				break;
			}
			else if(pSceneObject->m_unmanagedName == "enemy")
			{
				SceneObject_Enemy* pEnemy = static_cast<SceneObject_Enemy*>(pSceneObject);

				if(pEnemy->IsAgroed())
				{
					m_agroed = true;

					break;
				}
			}
		}
	}

	// Splitting
	if(s_numSwarmers < s_maxNumSwarmers)
	{
		m_splitTimer += GetScene()->m_frameTimer.GetTimeMultiplier();

		if(m_splitTimer >= m_splitTime)
			Split();
	}

	// Animation
	m_animationTime = Wrap(m_animationTime + GetScene()->m_frameTimer.GetTimeMultiplier() / 50.0f, m_pAnimation->GetAnimationDuration());
	m_pModel->Update(m_animationTime);

	m_aabb.SetCenter(m_position);

	m_pParticleEffect->m_position = m_position;

	if(IsSPTManaged())
		TreeUpdate();
}

void SceneObject_Enemy_Swarmer::Render()
{
	m_pModel->Render(m_animationTime, Matrix4x4f::TranslateMatrix(m_position) * Matrix4x4f::RotateMatrix_Y(m_faceAngleXZ + pif_over_2) * Matrix4x4f::RotateMatrix_X(pif_over_2) * Matrix4x4f::ScaleMatrix(Vec3f(0.2f, 0.2f, 0.2f)));
}

std::string SceneObject_Enemy_Swarmer::GetTypeName()
{
	return "swarmer";
}

void SceneObject_Enemy_Swarmer::Die()
{
	// Spawn blood particles
	SceneObject_ParticleEmitter_Sprite_Point* pParticleSystem = new SceneObject_ParticleEmitter_Sprite_Point();

	Asset* pAsset;

	if(!GetScene()->GetAssetManager_AutoCreate("tex", Asset_Texture::Asset_Factory)->GetAsset("data/particles/blood1.png", pAsset))
		abort();

	pParticleSystem->m_position = m_position;

	pParticleSystem->m_minDespawnTime = 5.0f;
	pParticleSystem->m_maxDespawnTime = 12.0f;

	pParticleSystem->m_minSpeed = 0.13f;
	pParticleSystem->m_maxSpeed = 0.25f;

	Asset_Texture* pParticleTexture = static_cast<Asset_Texture*>(pAsset);

	assert(pParticleTexture != NULL);

	// Texture settings
	pParticleTexture->Bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

	Sprite s;

	s.Create(pParticleTexture, 0.15f, 1, 1, 1);

	pParticleSystem->AddSprite(s);

	pParticleSystem->AddAffector(Affector_Gravity);

	pParticleSystem->m_autoDestruct = true;
	pParticleSystem->m_emit = false;

	GetScene()->Add(pParticleSystem, true);

	pParticleSystem->SpawnBurst(rand() % 20 + 30);

	// Remove this entity
	Destroy();
}

void SceneObject_Enemy_Swarmer::Split()
{
	m_splitTimer = 0.0f;
	m_splitTime = GetSplitTime();

	SceneObject_Enemy_Swarmer* pNewSwarmer = new SceneObject_Enemy_Swarmer(m_position);

	GetScene()->Add(pNewSwarmer, true);

	pNewSwarmer->m_faceAngleXZ = m_faceAngleXZ;
	pNewSwarmer->m_pRigidBody->getWorldTransform().getRotation().setY(m_pRigidBody->getWorldTransform().getRotation().getY());

	if(m_agroed)
		pNewSwarmer->m_agroed = true;
}