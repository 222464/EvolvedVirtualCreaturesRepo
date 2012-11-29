#include <SceneObjects/Physics/DynamicCharacterController.h>

class IgnoreBodyAndGhostCast :
	public btCollisionWorld::ClosestRayResultCallback
{
private:
	btRigidBody* m_pBody;
	btPairCachingGhostObject* m_pGhostObject;

public:
	IgnoreBodyAndGhostCast(btRigidBody* pBody, btPairCachingGhostObject* pGhostObject) 
		: btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0)),
		m_pBody(pBody), m_pGhostObject(pGhostObject)
	{
	}

	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		if(rayResult.m_collisionObject == m_pBody || rayResult.m_collisionObject == m_pGhostObject)
			return 1.0f;

		return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	}
};

DynamicCharacterController::DynamicCharacterController(Scene* pScene, SceneObject_PhysicsWorld* pPhysicsWorld, const Vec3f spawnPos, float radius, float height, float mass, float stepHeight)
	: m_pPhysicsWorld(pPhysicsWorld), m_pScene(pScene), m_bottomYOffset(height / 2.0f + radius), m_bottomRoundedRegionYOffset((height + radius) / 2.0f),
	m_deceleration(0.1f), m_maxSpeed(5.0f), m_jumpImpulse(600.0f),
	m_manualVelocity(0.0f, 0.0f, 0.0f), m_onGround(false), m_hittingWall(false),
	m_jumpRechargeTimer(0.0f), m_jumpRechargeTime(10.0f), m_stepHeight(stepHeight)
{
	m_pCollisionShape = new btCapsuleShape(radius, height);

	m_pMotionState = new btDefaultMotionState(btTransform(btQuaternion(1.0f, 0.0f, 0.0f, 0.0f).normalized(), bt(spawnPos)));

	btVector3 intertia;
	m_pCollisionShape->calculateLocalInertia(mass, intertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, m_pMotionState, m_pCollisionShape, intertia);

	// No friction, this is done manually
	rigidBodyCI.m_friction = 0.0f;
	//rigidBodyCI.m_additionalDamping = true;
	//rigidBodyCI.m_additionalLinearDampingThresholdSqr= 1.0f;
	//rigidBodyCI.m_additionalLinearDampingThresholdSqr = 0.5f;
	rigidBodyCI.m_restitution = 0.0f;

	rigidBodyCI.m_linearDamping = 0.0f;

	m_pRigidBody = new btRigidBody(rigidBodyCI);

	// Keep upright
	m_pRigidBody->setAngularFactor(0.0f);

	// No sleeping (or else setLinearVelocity won't work)
	m_pRigidBody->setActivationState(DISABLE_DEACTIVATION);

	m_pPhysicsWorld->m_pDynamicsWorld->addRigidBody(m_pRigidBody);

	// Ghost object that is synchronized with rigid body
	m_pGhostObject = new btPairCachingGhostObject();

	m_pGhostObject->setCollisionShape(m_pCollisionShape);
	m_pGhostObject->setUserPointer(this);
	m_pGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

	// Specify filters manually, otherwise ghost doesn't collide with statics for some reason
	m_pPhysicsWorld->m_pDynamicsWorld->addCollisionObject(m_pGhostObject, btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
}

DynamicCharacterController::~DynamicCharacterController()
{
	if(!m_pScene->IsClearing_SceneObjects())
	{
		m_pPhysicsWorld->m_pDynamicsWorld->removeRigidBody(m_pRigidBody);
		m_pPhysicsWorld->m_pDynamicsWorld->removeCollisionObject(m_pGhostObject);
	}

	delete m_pCollisionShape;
	delete m_pMotionState;
	delete m_pRigidBody;

	delete m_pGhostObject;
}

void DynamicCharacterController::Walk(const Vec2f dir)
{
	Vec2f velocityXZ(dir + Vec2f(m_manualVelocity.x, m_manualVelocity.z));

	// Prevent from going over maximum speed
	float speedXZ = velocityXZ.Magnitude();

	if(speedXZ > m_maxSpeed)
		velocityXZ = velocityXZ / speedXZ * m_maxSpeed;

	m_manualVelocity.x = velocityXZ.x;
	m_manualVelocity.z = velocityXZ.y;
}

void DynamicCharacterController::Walk(const Vec3f dir)
{
	Walk(Vec2f(dir.x, dir.z));
}

void DynamicCharacterController::Update()
{
	// Synch ghost with actually object
	m_pGhostObject->setWorldTransform(m_pRigidBody->getWorldTransform());
	//m_pGhostObject->getWorldTransform().getOrigin().setY(m_pGhostObject->getWorldTransform().getOrigin().getY() - 0.01f);

	// Update transform
	m_pMotionState->getWorldTransform(m_motionTransform);

	m_onGround = false;

	ParseGhostContacts();

	UpdatePosition();
	UpdateVelocity();

	// Update jump timer
	if(m_jumpRechargeTimer < m_jumpRechargeTime)
		m_jumpRechargeTimer += m_pPhysicsWorld->GetScene()->m_frameTimer.GetTimeMultiplier();
}

void DynamicCharacterController::ParseGhostContacts()
{
	btManifoldArray manifoldArray;
	btBroadphasePairArray &pairArray = m_pGhostObject->getOverlappingPairCache()->getOverlappingPairArray();
	int numPairs = pairArray.size();

	// Set false now, may be set true in test
	m_hittingWall = false;

	m_surfaceHitNormals.clear();

	for(int i = 0; i < numPairs; i++)
	{
		manifoldArray.clear();

		const btBroadphasePair &pair = pairArray[i];
         
		btBroadphasePair* collisionPair = m_pPhysicsWorld->m_pDynamicsWorld->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);
		
		if(collisionPair == NULL)
			continue;

		if(collisionPair->m_algorithm != NULL)
			collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

		for(int j = 0; j < manifoldArray.size(); j++)
		{
			btPersistentManifold* pManifold = manifoldArray[j];
			
			// Skip the rigid body the ghost monitors
			if(pManifold->getBody0() == m_pRigidBody)
				continue;

			for(int p = 0; p < pManifold->getNumContacts(); p++)
			{
				const btManifoldPoint &point = pManifold->getContactPoint(p);

				if(point.getDistance() < 0.0f)
				{
					//const btVector3 &ptA = point.getPositionWorldOnA();
					const btVector3 &ptB = point.getPositionWorldOnB();

					//const btVector3 &normalOnB = point.m_normalWorldOnB;

					// If point is in rounded bottom region of capsule shape, it is on the ground
					if(ptB.getY() < m_motionTransform.getOrigin().getY() - m_bottomRoundedRegionYOffset)
						m_onGround = true;
					else
					{
						m_hittingWall = true;

						m_surfaceHitNormals.push_back(cons(point.m_normalWorldOnB));
					}
				}
			}
		}
	}
}

void DynamicCharacterController::UpdateVelocity()
{
	// Adjust only xz velocity
	m_manualVelocity.y = m_pRigidBody->getLinearVelocity().getY();

	m_pRigidBody->setLinearVelocity(bt(m_manualVelocity));

	// Decelerate
	m_manualVelocity -= m_manualVelocity * m_deceleration * m_pPhysicsWorld->GetScene()->m_frameTimer.GetTimeMultiplier();

	if(m_hittingWall)
	{
		for(unsigned int i = 0, size = m_surfaceHitNormals.size(); i < size; i++)
		{
			// Cancel velocity across normal
			Vec3f velInNormalDir(m_manualVelocity.Project(m_surfaceHitNormals[i]));

			// Apply correction
			m_manualVelocity -= velInNormalDir * 1.05f;
		}

		// Do not adjust rigid body velocity manually (so bodies can still be pushed by character)
		return;
	}
}

void DynamicCharacterController::UpdatePosition()
{
	// Ray cast, ignore rigid body
	IgnoreBodyAndGhostCast rayCallBack_bottom(m_pRigidBody, m_pGhostObject);

	m_pPhysicsWorld->m_pDynamicsWorld->rayTest(m_pRigidBody->getWorldTransform().getOrigin(), m_pRigidBody->getWorldTransform().getOrigin() - btVector3(0.0f, m_bottomYOffset + m_stepHeight, 0.0f), rayCallBack_bottom);
	
	// Bump up if hit
	if(rayCallBack_bottom.hasHit())
	{
		float previousY = m_pRigidBody->getWorldTransform().getOrigin().getY();

		m_pRigidBody->getWorldTransform().getOrigin().setY(previousY + (m_bottomYOffset + m_stepHeight) * (1.0f - rayCallBack_bottom.m_closestHitFraction));

		btVector3 vel(m_pRigidBody->getLinearVelocity());

		vel.setY(0.0f);

		m_pRigidBody->setLinearVelocity(vel);

		m_onGround = true;
	}

	float testOffset = 0.07f;

	// Ray cast, ignore rigid body
	IgnoreBodyAndGhostCast rayCallBack_top(m_pRigidBody, m_pGhostObject);

	m_pPhysicsWorld->m_pDynamicsWorld->rayTest(m_pRigidBody->getWorldTransform().getOrigin(), m_pRigidBody->getWorldTransform().getOrigin() + btVector3(0.0f, m_bottomYOffset + testOffset, 0.0f), rayCallBack_top);
	
	// Bump up if hit
	if(rayCallBack_top.hasHit())
	{
		m_pRigidBody->getWorldTransform().setOrigin(m_previousPosition);

		btVector3 vel(m_pRigidBody->getLinearVelocity());

		vel.setY(0.0f);

		m_pRigidBody->setLinearVelocity(vel);
	}

	m_previousPosition = m_pRigidBody->getWorldTransform().getOrigin();
}

void DynamicCharacterController::Jump()
{
	if(m_onGround && m_jumpRechargeTimer >= m_jumpRechargeTime)
	{
		m_jumpRechargeTimer = 0.0f;
		m_pRigidBody->applyCentralImpulse(btVector3(0.0f, m_jumpImpulse, 0.0f));

		// Move upwards slightly so velocity isn't immediately canceled when it detects it as on ground next frame
		const float jumpYOffset = 0.01f;

		float previousY = m_pRigidBody->getWorldTransform().getOrigin().getY();

		m_pRigidBody->getWorldTransform().getOrigin().setY(previousY + jumpYOffset);
	}
}

Vec3f DynamicCharacterController::GetPosition() const
{
	return cons(m_motionTransform.getOrigin());
}

Vec3f DynamicCharacterController::GetVelocity() const
{
	return cons(m_pRigidBody->getLinearVelocity());
}

bool DynamicCharacterController::IsOnGround() const
{
	return m_onGround;
}