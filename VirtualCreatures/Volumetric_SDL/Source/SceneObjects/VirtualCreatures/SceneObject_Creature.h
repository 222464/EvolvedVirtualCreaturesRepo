#pragma once

#include <Scene/SceneObject.h>
#include <SceneObjects/VirtualCreatures/Limb.h>

#include <Renderer/Model_OBJ.h>

struct CreatureGenes
{
	unsigned int m_mainBrainOutputs;
	std::vector<LimbGenes*> m_pLimbGenes;

	CreatureGenes();
	CreatureGenes(const CreatureGenes &other);
	~CreatureGenes();

	CreatureGenes &operator=(const CreatureGenes &other);
};

class SceneObject_Creature :
	public SceneObject
{
private:
	Limb* m_pRoot;
	std::vector<Limb*> m_pLimbs;

	// Physics
	SceneObjectReferenceTracker m_physicsWorldTracker;
	SceneObject_PhysicsWorld* m_pPhysicsWorld;

	// Graphics
	Model_OBJ* m_pLimbModel_Sensor;
	Model_OBJ* m_pLimbModel_Root;
	Model_OBJ* m_pLimbModel_Other;

	void UpdateAABB();

public:
	static const unsigned int s_numNeuralNetCycles_propagation = 20;
	static const unsigned int s_numNeuralNetCycles_outputAverage = 30;

	float m_dopamine;

	SceneObject_Creature();
	~SceneObject_Creature();

	// Inherited from SceneObject
	void OnAdd();
	void Logic();
	void Render();

	void CreateFromGenes(const CreatureGenes &genes, const btTransform &baseTransform);

	Vec3f GetPosition() const
	{
		assert(m_pRoot != NULL);
		return cons(m_pRoot->m_pRigidBody->getWorldTransform().getOrigin());
	}

	friend class Limb;
	friend class Neuron_Sensor_Contact;
};

