#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/Neuron_Sensor_Contact.h>
#include <SceneObjects/VirtualCreatures/Limb.h>
#include <Scene/Scene.h>

Neuron_Sensor_Contact::Neuron_Sensor_Contact()
	: m_pGhostObject(NULL)
{
}

Neuron_Sensor_Contact::~Neuron_Sensor_Contact()
{
	if(m_pGhostObject != NULL)
	{
		if(m_pLimb->PhysicsWorldAlive())
			m_pLimb->GetPhysicsWorld()->m_pDynamicsWorld->removeCollisionObject(m_pGhostObject);

		delete m_pGhostObject;
	}
}

void Neuron_Sensor_Contact::Create()
{
	assert(m_pLimb != NULL);

	m_pGhostObject = new btPairCachingGhostObject();

	m_pGhostObject->setCollisionShape(m_pLimb->GetCollisionShape());
	m_pGhostObject->setUserPointer(this);
	m_pGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

	// Specify filters manually, otherwise ghost doesn't collide with statics for some reason
	m_pLimb->GetPhysicsWorld()->m_pDynamicsWorld->addCollisionObject(m_pGhostObject, btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
}

bool Neuron_Sensor_Contact::Hitting()
{
	// Synch ghost with object
	m_pGhostObject->setWorldTransform(m_pLimb->GetRigidBody()->getWorldTransform());
	
	// Check sensor
	btManifoldArray manifoldArray;
	btBroadphasePairArray &pairArray = m_pGhostObject->getOverlappingPairCache()->getOverlappingPairArray();
	int numPairs = pairArray.size();

	// Set false now, may be set true in test
	bool hit = false;

	for(int i = 0; i < numPairs; i++)
	{
		manifoldArray.clear();

		const btBroadphasePair &pair = pairArray[i];
         
		btBroadphasePair* collisionPair = m_pLimb->GetPhysicsWorld()->m_pDynamicsWorld->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);
		
		if(collisionPair == NULL)
			continue;

		if(collisionPair->m_algorithm != NULL)
			collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

		for(int j = 0; j < manifoldArray.size(); j++)
		{
			btPersistentManifold* pManifold = manifoldArray[j];
			
			// Skip the rigid body the ghost monitors
			if(pManifold->getBody0() == m_pLimb->GetRigidBody())
				continue;

			for(int p = 0; p < pManifold->getNumContacts(); p++)
			{
				const btManifoldPoint &point = pManifold->getContactPoint(p);

				if(point.getDistance() < 0.0f)
					return true;
			}
		}
	}

	return false;
}

void Neuron_Sensor_Contact::PhysicsUpdate()
{
	assert(m_pLimb != NULL);
	
	if(Hitting())
	{
		m_output = 1.0f;

		m_timeSinceLastFire = 0.0f;
	}
	else
	{
		m_output = 0.0f;

		m_timeSinceLastFire++;
	}
}