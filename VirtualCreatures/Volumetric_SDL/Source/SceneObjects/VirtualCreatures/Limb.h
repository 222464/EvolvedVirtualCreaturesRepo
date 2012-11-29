#pragma once

#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>
#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/Neuron.h>
#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/Neuron_Sensor_Timer.h>
#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/Neuron_Sensor_Contact.h>

#include <Misc/PID_Controller.h>

#include <Renderer/Model_OBJ.h>

#include <System/Uncopyable.h>

// Uncomment the macro below in order to allow bodies at different levels in the morphology tree to collide
#define SELF_COLLISION

struct LimbGenes
{
	struct Sensor_Timer_Data
	{
		float m_initTime;
		float m_rate;
	};

	struct Neuron_Data
	{
		std::vector<float> m_weights;

		float m_threshold;
	};

	struct ChildAndOutputIndex
	{
		int m_childIndex;
		int m_outputIndex;
	};

	int m_parentIndexOffset;

	int m_recursiveUnits;
	float m_recursionDegrade; // Stuff shrinks, weakens, whatever according to this

	bool m_symmetrical;
	bool m_inheritsRecursion; // If parented to recursive limb, become part of repeated unit if true
	bool m_inheritsSymmetry; // If parented to symmetric limb, become part of reflected units if true

	int m_numBranches;

	Vec3f m_dims;

	// Normalized so can be crossed over
	Vec3f m_relativeAttachmentPositionOnParent;
	Vec3f m_relativeAttachmentPositionOnThis;
	Quaternion m_relativeRotation;

	// In radians
	float m_bendLimit;
	float m_twistLimit;

	// Density of limt
	float m_density;

	float m_friction;

	float m_strength;

	bool m_hasContactSensor;

	unsigned int m_numTimerSensors; // Separate from vector, so vector changes to match num sensors if not enough
	std::vector<Sensor_Timer_Data> m_timerSensorData;

	// Neuron data for all combinations of morphology generation
	std::vector<Neuron_Data> m_neuronData;
	std::vector<Neuron_Data> m_reflectedData;
	std::vector<std::vector<Neuron_Data>> m_recursiveData; // 2D due to repeating units
	std::vector<std::vector<Neuron_Data>> m_reflectedAndRecursiveData; // 2D due to repeating units

	// Indices
	std::vector<int> m_mainBrainInputNeurons;

	unsigned int m_numHiddenLayers;
	unsigned int m_numNeuronsPerHiddenLayer;

	std::vector<int> m_parentLimbInputNeurons;

	std::vector<ChildAndOutputIndex> m_childLimbInputNeurons;

	LimbGenes();
	~LimbGenes();
};

class Limb :
	public Uncopyable
{
public:
	static const unsigned int s_numMotorInputs = 3; // 1 for each axis
	static const unsigned int s_numMotorOutputs = 14; // 2 outputs for each direction of motor (4 per axis, 12 for all axes) + 2 for motor strength

	static float s_minInitTimerRate;
	static float s_maxInitTimerRate;
	static float s_minInitTimerTime;
	static float s_maxInitTimerTime;

private:
	Limb* m_pParent;
	class SceneObject_Creature* m_pCreature;

	std::vector<Limb*> m_children;

	// Physics
	btCollisionShape* m_pCollisionShape;
	btDefaultMotionState* m_pMotionState;
	btRigidBody* m_pRigidBody;

	btGeneric6DofConstraint* m_pJoint;

	// PID controllers for each axis
	PID_Controller m_PID_x, m_PID_y, m_PID_z;

	btTransform m_physicsTransform;

	Vec3f m_halfDims;

	float m_bendLimit;
	float m_twistLimit;

	float m_strength;

	// Neurons
	std::vector<Neuron> m_neurons;

	std::vector<Neuron_Sensor_Timer> m_timers;

	std::unique_ptr<Neuron_Sensor_Contact> m_pContactSensor;

	// Default sensors
	NeuronInput m_motorInputs[s_numMotorInputs];

	// Used to average neuron motor outputs over time
	float m_motorOutputs[s_numMotorOutputs];

	// Graphics
	Model_OBJ* m_pModel;

	unsigned int m_numOutputs;

	unsigned short m_limbLevel;

	bool m_reflected;

	unsigned int m_geneInnovationNumber; // Used just like in NEAT

public:
	Limb();
	~Limb();

	void Create(class SceneObject_Creature* pCreature, Limb* pParent, LimbGenes &genes, int flipAxis = -1);
	void Create_Root(class SceneObject_Creature* pCreature, const btTransform &baseTransform, LimbGenes &genes, unsigned int numOutputs);

	void CreateNet_SubNet_FeedForward(LimbGenes &genes); // Creates net using genes. Genes may be altered (random data added if not enough is given)

	//void CreateNet_SemiRecurrent(); // Previous outputs are also inputs

	void CreateNet_MainBrain_FeedForward(LimbGenes &genes, unsigned int numOutputs);

	void SetInputs_SubNet(LimbGenes &genes);
	void SetInputs_Root(LimbGenes &genes, const std::vector<NeuronInput*> &additionalInputs);

	void GetGenes(LimbGenes &genes);
	void PhysicsUpdate();
	void StartMotorAveraging();
	void NeuronUpdate(float dopamine); // Goes over all neurons once, called multiple times in a frame
	void AddMotorAverages(float weighting);
	void MotorUpdate();

	void Render();

	SceneObject_PhysicsWorld* GetPhysicsWorld() const;

	bool PhysicsWorldAlive() const;

	class SceneObject_Creature* GetCreature() const
	{
		return m_pCreature;
	}

	btCollisionShape* GetCollisionShape() const
	{
		return m_pCollisionShape;
	}

	btDefaultMotionState* GetMotionState() const
	{
		return m_pMotionState;
	}

	btRigidBody* GetRigidBody() const
	{
		return m_pRigidBody;
	}

	btGeneric6DofConstraint* GetJoint() const
	{
		return m_pJoint;
	}

	unsigned int GetNumNeurons() const
	{
		return m_neurons.size();
	}

	unsigned int GetNumTimerSensors() const
	{
		return m_timers.size();
	}

	friend class SceneObject_Creature;
};

