#include <SceneObjects/VirtualCreatures/Limb.h>

#include <SceneObjects/VirtualCreatures/SceneObject_Creature.h>

#include <Utilities/UtilFuncs.h>

float Limb::s_minInitTimerRate = 0.05f;
float Limb::s_maxInitTimerRate = 0.6f;
float Limb::s_minInitTimerTime = 0.0f;
float Limb::s_maxInitTimerTime = 0.9f;

LimbGenes::LimbGenes()
{
}

LimbGenes::~LimbGenes()
{
}

Limb::Limb()
	: m_pCreature(NULL),
	m_pCollisionShape(NULL),
	m_pMotionState(NULL),
	m_pRigidBody(NULL),
	m_pJoint(NULL),
	m_pModel(NULL),
	m_PID_x(0.7f, 0.1f, 0.5f), m_PID_y(0.7f, 0.1f, 0.5f), m_PID_z(0.7f, 0.1f, 0.5f)
{
	for(unsigned int i = 0; i < s_numMotorOutputs; i++)
		m_motorOutputs[i] = 0.0f;
}

Limb::~Limb()
{
	if(m_pCreature != NULL)
	{
		if(PhysicsWorldAlive())
		{
			if(m_pJoint != NULL)
				GetPhysicsWorld()->m_pDynamicsWorld->removeConstraint(m_pJoint);

			GetPhysicsWorld()->m_pDynamicsWorld->removeRigidBody(m_pRigidBody);
		}

		if(m_pJoint != NULL)
			delete m_pJoint;

		delete m_pCollisionShape;
		delete m_pMotionState;
		delete m_pRigidBody;
	}
}

void Limb::Create(class SceneObject_Creature* pCreature, Limb* pParent, LimbGenes &genes, int flipAxis) // flipAxis = -1 makes it not flip it
{
	assert(m_pCreature == NULL); // Not already created

	// ----------------------------------------- Copy gene data -----------------------------------------

	// Physics related data
	m_pCreature = pCreature;
	m_pParent = pParent;

	assert(m_pParent != NULL);

	m_limbLevel = m_pParent->m_limbLevel + 1;

	m_bendLimit = genes.m_bendLimit;
	m_twistLimit = genes.m_twistLimit;

	m_strength = genes.m_strength;

	m_halfDims = genes.m_dims / 2.0f;

	// ------------------------------------------ Physics ------------------------------------------

	m_pCollisionShape = new btBoxShape(bt(m_halfDims));

	Vec3f attachmentPositionOnThis(genes.m_relativeAttachmentPositionOnThis * m_halfDims);
	Vec3f attachmentPositionOnParent(genes.m_relativeAttachmentPositionOnParent * m_pParent->m_halfDims);

	Quaternion rot(cons(m_pParent->m_pRigidBody->getWorldTransform().getRotation()) * genes.m_relativeRotation);

	m_reflected = flipAxis != -1;

	switch(flipAxis)
	{
	case 0: // x
		attachmentPositionOnThis.x *= -1;
		attachmentPositionOnParent.x *= -1;
		rot.y *= -1;
		rot.z *= -1;
		break;
	case 1: // y
		attachmentPositionOnThis.y *= -1;
		attachmentPositionOnParent.y *= -1;
		rot.x *= -1;
		rot.z *= -1;
		break;
	case 2: // z
		attachmentPositionOnThis.z *= -1;
		attachmentPositionOnParent.z *= -1;
		rot.x *= -1;
		rot.y *= -1;
		break;
	}

	rot.NormalizeThis();

	assert(m_pParent->m_pRigidBody != NULL);

	Vec3f boxCenterPos(cons(m_pParent->m_pRigidBody->getWorldTransform().getOrigin()) + cons(m_pParent->m_pRigidBody->getWorldTransform().getRotation()) * attachmentPositionOnParent + rot * (Vec3f(m_halfDims.x, 0.0f, 0.0f) + attachmentPositionOnThis));

	m_pMotionState = new btDefaultMotionState(btTransform(bt(genes.m_relativeRotation).normalized(), bt(boxCenterPos)));

	const float mass = genes.m_density * genes.m_dims.Magnitude();

	btVector3 intertia;
	m_pCollisionShape->calculateLocalInertia(mass, intertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, m_pMotionState, m_pCollisionShape, intertia);

	rigidBodyCI.m_friction = genes.m_friction;

	assert(m_pRigidBody == NULL);
	m_pRigidBody = new btRigidBody(rigidBodyCI);

#ifdef SELF_COLLISION
	unsigned short group = Pow2(m_limbLevel);
	unsigned short mask = 0xffff ^ (Pow2(m_pParent->m_limbLevel) | group | Pow2(m_limbLevel + 1)); // Mask out parent and child
#else
	unsigned short group = 0x8000;
	unsigned short mask = 0x7fff;
#endif

	GetPhysicsWorld()->m_pDynamicsWorld->addRigidBody(m_pRigidBody, group, mask);

	// If it has a parent, create a joint
	btTransform parentFrame;
	parentFrame.setIdentity();
	parentFrame.setOrigin(bt(attachmentPositionOnParent));
	parentFrame.setRotation(bt(genes.m_relativeRotation));

	btTransform thisFrame;
	thisFrame.setIdentity();
	thisFrame.setOrigin(bt(genes.m_relativeAttachmentPositionOnThis * m_halfDims));

	// Create joint
	m_pJoint = new btGeneric6DofConstraint(*m_pParent->m_pRigidBody, *m_pRigidBody, parentFrame, thisFrame, true);

	// Limit angles
	//m_pJoint->setAngularLowerLimit(btVector3(-genes.m_bendLimit, -genes.m_bendLimit, -genes.m_twistLimit));
	//m_pJoint->setAngularUpperLimit(btVector3(genes.m_bendLimit, genes.m_bendLimit, genes.m_twistLimit));
	m_pJoint->setLimit(3, -genes.m_bendLimit, genes.m_bendLimit);
	m_pJoint->setLimit(4, -genes.m_bendLimit, genes.m_bendLimit);
	m_pJoint->setLimit(5, -genes.m_twistLimit, genes.m_twistLimit);

	const float maxLimitForce = genes.m_strength * 2.0f; // Larger number than limb strength
	const float limitSoftness = 0.1f;
	const float damping = 0.5f;

	// Limit motor angles too (not really necessary?)
	m_pJoint->getRotationalLimitMotor(0)->m_enableMotor = true;
	m_pJoint->getRotationalLimitMotor(0)->m_loLimit = -genes.m_bendLimit;
	m_pJoint->getRotationalLimitMotor(0)->m_hiLimit = genes.m_bendLimit;
	m_pJoint->getRotationalLimitMotor(0)->m_maxLimitForce = maxLimitForce;
	m_pJoint->getRotationalLimitMotor(0)->m_limitSoftness = limitSoftness;
	m_pJoint->getRotationalLimitMotor(0)->m_damping = damping;

	m_pJoint->getRotationalLimitMotor(1)->m_enableMotor = true;
	m_pJoint->getRotationalLimitMotor(1)->m_loLimit = -genes.m_bendLimit;
	m_pJoint->getRotationalLimitMotor(1)->m_hiLimit = genes.m_bendLimit;
	m_pJoint->getRotationalLimitMotor(1)->m_maxLimitForce = maxLimitForce;
	m_pJoint->getRotationalLimitMotor(1)->m_limitSoftness = limitSoftness;
	m_pJoint->getRotationalLimitMotor(1)->m_damping = damping;

	m_pJoint->getRotationalLimitMotor(2)->m_enableMotor = true;
	m_pJoint->getRotationalLimitMotor(2)->m_loLimit = -genes.m_twistLimit;
	m_pJoint->getRotationalLimitMotor(2)->m_hiLimit = genes.m_twistLimit;
	m_pJoint->getRotationalLimitMotor(2)->m_maxLimitForce = maxLimitForce;
	m_pJoint->getRotationalLimitMotor(2)->m_limitSoftness = limitSoftness;
	m_pJoint->getRotationalLimitMotor(2)->m_damping = damping;

	GetPhysicsWorld()->m_pDynamicsWorld->addConstraint(m_pJoint);

	// ------------------------------------------ Neural Network ------------------------------------------

	CreateNet_SubNet_FeedForward(genes);

	// --------------------------------------------- Graphics ---------------------------------------------

	// Choose model based on parameters
	if(m_pContactSensor.get() != NULL)
		m_pModel = m_pCreature->m_pLimbModel_Sensor;
	else
		m_pModel = m_pCreature->m_pLimbModel_Other;
}

void Limb::Create_Root(class SceneObject_Creature* pCreature, const btTransform &baseTransform, LimbGenes &genes, unsigned int numOutputs)
{
	assert(m_pCreature == NULL); // Not already created

	// ----------------------------------------- Copy gene data -----------------------------------------

	// Physics related data
	m_pCreature = pCreature;
	m_pParent = NULL;

	m_limbLevel = 2;

	m_bendLimit = genes.m_bendLimit;
	m_twistLimit = genes.m_twistLimit;

	m_strength = genes.m_strength;

	m_halfDims = genes.m_dims / 2.0f;

	// ------------------------------------------ Physics ------------------------------------------

	m_pCollisionShape = new btBoxShape(bt(m_halfDims));

	Vec3f attachmentPositionOnThis(genes.m_relativeAttachmentPositionOnThis * m_halfDims);
	Vec3f attachmentPositionOnParent(genes.m_relativeAttachmentPositionOnParent * m_halfDims); // Randomly init these for now

	m_pMotionState = new btDefaultMotionState(btTransform(bt(genes.m_relativeRotation).normalized(), baseTransform.getOrigin()));

	const float mass = genes.m_density * genes.m_dims.Magnitude();

	btVector3 intertia;
	m_pCollisionShape->calculateLocalInertia(mass, intertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, m_pMotionState, m_pCollisionShape, intertia);

	rigidBodyCI.m_friction = genes.m_friction;

	assert(m_pRigidBody == NULL);
	m_pRigidBody = new btRigidBody(rigidBodyCI);

#ifdef SELF_COLLISION
	unsigned short group = 1;
	unsigned short mask = 0xffff ^ (Pow2(m_limbLevel + 1) | group);
#else
	unsigned short group = 0x8000;
	unsigned short mask = 0x7fff;
#endif
	
	GetPhysicsWorld()->m_pDynamicsWorld->addRigidBody(m_pRigidBody, group, mask);

	// ------------------------------------------ Neural Network ------------------------------------------

	CreateNet_MainBrain_FeedForward(genes, numOutputs);

	// --------------------------------------------- Graphics ---------------------------------------------

	// Choose model based on parameters
	if(m_pContactSensor != NULL)
		m_pModel = m_pCreature->m_pLimbModel_Sensor;
	else
		m_pModel = m_pCreature->m_pLimbModel_Other;
}

void Limb::CreateNet_SubNet_FeedForward(LimbGenes &genes)
{
	assert(m_pParent != NULL);

	m_numOutputs = s_numMotorOutputs;

	m_neurons.resize(genes.m_numHiddenLayers * genes.m_numNeuronsPerHiddenLayer + m_numOutputs);

	// Skip first hidden layer for now
	unsigned int ni = genes.m_numNeuronsPerHiddenLayer;

	for(unsigned int l = 1; l < genes.m_numHiddenLayers; l++)
	{
		const unsigned int startNi = ni - 1;

		for(unsigned int i = 0; i < genes.m_numNeuronsPerHiddenLayer; i++, ni++)
		{
			// Create neuron if it does not exist
			if(ni >= genes.m_neuronData.size())
			{
				while(ni >= genes.m_neuronData.size())
				{
					LimbGenes::Neuron_Data data;

					// Random assignment
					data.m_threshold = Randf();

					for(unsigned int j = 0; j < genes.m_numNeuronsPerHiddenLayer; j++)
						data.m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));

					genes.m_neuronData.push_back(data);
				}
			}
			else if(genes.m_neuronData[ni].m_weights.size() != genes.m_numNeuronsPerHiddenLayer) // Remove/Add Random inputs if size mismatch
			{
				const unsigned int numWeights =  genes.m_neuronData[ni].m_weights.size();

				// Add random weights if there are not enough
				if(numWeights < genes.m_numNeuronsPerHiddenLayer)
				{
					const unsigned int difference = genes.m_numNeuronsPerHiddenLayer - numWeights;

					for(unsigned int i = 0; i < difference; i++)
						genes.m_neuronData[ni].m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));
				}
				else
					genes.m_neuronData[ni].m_weights.resize(genes.m_numNeuronsPerHiddenLayer);
			}

			// Create weight array for neuron to take over
			const unsigned int numWeights = genes.m_neuronData[ni].m_weights.size();

			Neuron::NeuronInputAndWeight* pNeuronInputs = new Neuron::NeuronInputAndWeight[numWeights];

			for(unsigned int j = 0; j < numWeights; j++)
			{
				pNeuronInputs[j].m_pInput = &m_neurons[startNi - j];
				pNeuronInputs[j].m_weight = genes.m_neuronData[ni].m_weights[j];
			}

			m_neurons[ni].SetInputs(numWeights, pNeuronInputs);
		}
	}

	// Outputs
	unsigned int startNi = ni - 1;

	for(unsigned int i = 0; i < s_numMotorOutputs; i++, ni++)
	{
		// Create neuron if it does not exist
		if(ni >= genes.m_neuronData.size())
		{
			while(ni >= genes.m_neuronData.size())
			{
				LimbGenes::Neuron_Data data;

				// Random assignment
				data.m_threshold = Randf();

				for(unsigned int j = 0; j < genes.m_numNeuronsPerHiddenLayer; j++)
					data.m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));

				genes.m_neuronData.push_back(data);
			}
		}
		else if(genes.m_neuronData[ni].m_weights.size() != genes.m_numNeuronsPerHiddenLayer) // Remove/Add Random inputs if size mismatch
		{
			const unsigned int numWeights =  genes.m_neuronData[ni].m_weights.size();

			// Add random weights if there are not enough
			if(numWeights < genes.m_numNeuronsPerHiddenLayer)
			{
				const unsigned int difference = genes.m_numNeuronsPerHiddenLayer - numWeights;

				for(unsigned int j = 0; j < difference; j++)
					genes.m_neuronData[ni].m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));
			}
			else
				genes.m_neuronData[ni].m_weights.resize(genes.m_numNeuronsPerHiddenLayer);
		}

		// Create weight array for neuron to take over
		const unsigned int numWeights = genes.m_neuronData[ni].m_weights.size();

		Neuron::NeuronInputAndWeight* pNeuronInputs = new Neuron::NeuronInputAndWeight[numWeights];

		for(unsigned int j = 0; j < numWeights; j++)
		{
			pNeuronInputs[j].m_pInput = &m_neurons[startNi - j];
			pNeuronInputs[j].m_weight = genes.m_neuronData[ni].m_weights[j];
		}

		m_neurons[ni].SetInputs(numWeights, pNeuronInputs);
	}

	// Trim away unused neurons
	if(ni < genes.m_neuronData.size() - 1)
		genes.m_neuronData.resize(ni + 1);
}

void Limb::CreateNet_MainBrain_FeedForward(LimbGenes &genes, unsigned int numOutputs)
{
	assert(m_pParent == NULL);

	m_numOutputs = numOutputs;

	m_neurons.resize(genes.m_numHiddenLayers * genes.m_numNeuronsPerHiddenLayer + m_numOutputs);

	// Skip first hidden layer for now
	unsigned int ni = genes.m_numNeuronsPerHiddenLayer;

	// Other hidden layers
	for(unsigned int l = 1; l < genes.m_numHiddenLayers; l++)
	{
		const unsigned int startNi = ni - 1;

		for(unsigned int i = 0; i < genes.m_numNeuronsPerHiddenLayer; i++, ni++)
		{
			// Create neuron if it does not exist
			if(ni >= genes.m_neuronData.size())
			{
				while(ni >= genes.m_neuronData.size())
				{
					LimbGenes::Neuron_Data data;

					// Random assignment
					data.m_threshold = Randf();

					for(unsigned int j = 0; j < genes.m_numNeuronsPerHiddenLayer; j++)
						data.m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));

					genes.m_neuronData.push_back(data);
				}
			}
			else if(genes.m_neuronData[ni].m_weights.size() != genes.m_numNeuronsPerHiddenLayer) // Remove/Add Random inputs if size mismatch
			{
				const unsigned int numWeights =  genes.m_neuronData[ni].m_weights.size();

				// Add random weights if there are not enough
				if(numWeights < genes.m_numNeuronsPerHiddenLayer)
				{
					const unsigned int difference = genes.m_numNeuronsPerHiddenLayer - numWeights;

					for(unsigned int j = 0; j < difference; j++)
						genes.m_neuronData[ni].m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));
				}
				else
					genes.m_neuronData[ni].m_weights.resize(genes.m_numNeuronsPerHiddenLayer);
			}

			// Create weight array for neuron to take over
			const unsigned int numWeights = genes.m_neuronData[ni].m_weights.size();

			Neuron::NeuronInputAndWeight* pNeuronInputs = new Neuron::NeuronInputAndWeight[numWeights];

			for(unsigned int j = 0; j < numWeights; j++)
			{
				pNeuronInputs[j].m_pInput = &m_neurons[startNi - j];
				pNeuronInputs[j].m_weight = genes.m_neuronData[ni].m_weights[j];
			}

			m_neurons[ni].SetInputs(numWeights, pNeuronInputs);
		}
	}

	// Outputs
	unsigned int startNi = ni - 1;

	for(unsigned int i = 0; i < m_numOutputs; i++, ni++)
	{
		// Create neuron if it does not exist
		if(ni >= genes.m_neuronData.size())
		{
			while(ni >= genes.m_neuronData.size())
			{
				LimbGenes::Neuron_Data data;

				// Random assignment
				data.m_threshold = Randf();

				for(unsigned int j = 0; j < genes.m_numNeuronsPerHiddenLayer; j++)
					data.m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));

				genes.m_neuronData.push_back(data);
			}
		}
		else if(genes.m_neuronData[ni].m_weights.size() != genes.m_numNeuronsPerHiddenLayer) // Remove/Add Random inputs if size mismatch
		{
			const unsigned int numWeights =  genes.m_neuronData[ni].m_weights.size();

			// Add random weights if there are not enough
			if(numWeights < genes.m_numNeuronsPerHiddenLayer)
			{
				const unsigned int difference = genes.m_numNeuronsPerHiddenLayer - numWeights;

				for(unsigned int j = 0; j < difference; j++)
					genes.m_neuronData[ni].m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));
			}
			else
				genes.m_neuronData[ni].m_weights.resize(genes.m_numNeuronsPerHiddenLayer);
		}

		// Create weight array for neuron to take over
		const unsigned int numWeights = genes.m_neuronData[ni].m_weights.size();

		Neuron::NeuronInputAndWeight* pNeuronInputs = new Neuron::NeuronInputAndWeight[numWeights];

		for(unsigned int j = 0; j < numWeights; j++)
		{
			pNeuronInputs[j].m_pInput = &m_neurons[startNi - j];
			pNeuronInputs[j].m_weight = genes.m_neuronData[ni].m_weights[j];
		}

		m_neurons[ni].SetInputs(numWeights, pNeuronInputs);
	}

	// Trim away unused neurons
	if(ni < genes.m_neuronData.size() - 1)
		genes.m_neuronData.resize(ni + 1);
}

void Limb::SetInputs_SubNet(LimbGenes &genes)
{
	// All inputs/sensors list
	std::vector<NeuronInput*> inputs;

	// Create/Append inputs/sensors to list
	for(unsigned int i = 0; i < s_numMotorInputs; i++)
		inputs.push_back(&m_motorInputs[i]);

	if(genes.m_hasContactSensor)
	{
		m_pContactSensor.reset(new Neuron_Sensor_Contact());
		m_pContactSensor->m_pLimb = this;
		m_pContactSensor->Create();

		inputs.push_back(m_pContactSensor.get());
	}

	m_timers.reserve(genes.m_numTimerSensors);

	for(unsigned int i = 0; i < genes.m_numTimerSensors; i++)
	{
		if(m_timers.size() <= i)
		{
			LimbGenes::Sensor_Timer_Data newTimer;
			newTimer.m_initTime = Randf(s_minInitTimerTime, s_maxInitTimerTime);
			newTimer.m_rate = Randf(s_minInitTimerRate, s_maxInitTimerRate);
			genes.m_timerSensorData.push_back(newTimer);
			m_timers.push_back(Neuron_Sensor_Timer());
		}

		m_timers[i].m_timer = genes.m_timerSensorData[i].m_initTime;
		m_timers[i].m_rate = genes.m_timerSensorData[i].m_rate;
		m_timers[i].m_pLimb = this;
		m_timers[i].Create();

		inputs.push_back(&m_timers[i]);
	}

	// Trim away unused timer data
	if(genes.m_numTimerSensors < genes.m_timerSensorData.size())
		genes.m_timerSensorData.resize(genes.m_numTimerSensors);

	// Add previous limb inputs
	for(unsigned int i = 0, size = genes.m_parentLimbInputNeurons.size(); i < size; i++)
	{
		if(genes.m_parentLimbInputNeurons[i] >= static_cast<signed>(m_pParent->m_numOutputs))
			genes.m_parentLimbInputNeurons[i] = rand() % m_pParent->m_numOutputs;

		inputs.push_back(&m_pParent->m_neurons[m_pParent->m_numOutputs - genes.m_parentLimbInputNeurons[i] - 1]);
	}

	// Add child limb inputs
	if(!m_children.empty())
		for(unsigned int i = 0, size = genes.m_childLimbInputNeurons.size(); i < size; i++)
		{
			if(genes.m_childLimbInputNeurons[i].m_childIndex >= static_cast<signed>(m_children.size()))
				genes.m_childLimbInputNeurons[i].m_childIndex = rand() % m_children.size();

			Limb* pChild = m_children[genes.m_childLimbInputNeurons[i].m_childIndex];

			if(genes.m_childLimbInputNeurons[i].m_outputIndex >= static_cast<signed>(pChild->m_numOutputs))
				genes.m_childLimbInputNeurons[i].m_outputIndex = rand() % pChild->m_numOutputs;

			inputs.push_back(&pChild->m_neurons[pChild->GetNumNeurons() - genes.m_childLimbInputNeurons[i].m_outputIndex - 1]);
		}

	// Add main brain inputs neurons
	for(unsigned int i = 0, size = genes.m_mainBrainInputNeurons.size(); i < size; i++)
	{
		if(genes.m_mainBrainInputNeurons[i] >= static_cast<signed>(m_pCreature->m_pRoot->m_numOutputs))
			genes.m_mainBrainInputNeurons[i] = rand() % m_pCreature->m_pRoot->m_numOutputs;

		inputs.push_back(&m_pCreature->m_pRoot->m_neurons[m_pCreature->m_pRoot->m_numOutputs - genes.m_mainBrainInputNeurons[i] - 1]);
	}

	const unsigned int numInputs = inputs.size();

	// First hidden layer
	for(unsigned int ni = 0; ni < genes.m_numNeuronsPerHiddenLayer; ni++)
	{
		// Create neuron if it does not exist
		if(ni >= genes.m_neuronData.size())
		{
			while(ni >= genes.m_neuronData.size())
			{
				LimbGenes::Neuron_Data data;

				// Random assignment
				data.m_threshold = Randf();

				for(unsigned int j = 0; j < numInputs; j++)
					data.m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));

				genes.m_neuronData.push_back(data);
			}
		}
		else if(genes.m_neuronData[ni].m_weights.size() != numInputs) // Remove/Add Random inputs if size mismatch
		{
			const unsigned int numWeights =  genes.m_neuronData[ni].m_weights.size();

			// Add random weights if there are not enough
			if(numWeights < numInputs)
			{
				const unsigned int difference = numInputs - numWeights;

				for(unsigned int j = 0; j < difference; j++)
					genes.m_neuronData[ni].m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));
			}
			else
				genes.m_neuronData[ni].m_weights.resize(numInputs);
		}

		// Create weight array for neuron to take over
		const unsigned int numWeights = genes.m_neuronData[ni].m_weights.size();

		Neuron::NeuronInputAndWeight* pNeuronInputs = new Neuron::NeuronInputAndWeight[numWeights];

		for(unsigned int j = 0; j < numWeights; j++)
		{
			pNeuronInputs[j].m_pInput = inputs[j];
			pNeuronInputs[j].m_weight = genes.m_neuronData[ni].m_weights[j];
		}

		m_neurons[ni].SetInputs(numWeights, pNeuronInputs);
	}
}

void Limb::SetInputs_Root(LimbGenes &genes, const std::vector<NeuronInput*> &additionalInputs)
{
	std::vector<NeuronInput*> inputs(additionalInputs);

	// Create/Append inputs/sensors to list
	if(genes.m_hasContactSensor)
	{
		m_pContactSensor.reset(new Neuron_Sensor_Contact());
		m_pContactSensor->m_pLimb = this;
		m_pContactSensor->Create();

		inputs.push_back(m_pContactSensor.get());
	}

	m_timers.reserve(genes.m_numTimerSensors);

	for(unsigned int i = 0; i < genes.m_numTimerSensors; i++)
	{
		if(m_timers.size() <= i)
		{
			LimbGenes::Sensor_Timer_Data newTimer;
			newTimer.m_initTime = Randf(s_minInitTimerTime, s_maxInitTimerTime);
			newTimer.m_rate = Randf(s_minInitTimerRate, s_maxInitTimerRate);
			genes.m_timerSensorData.push_back(newTimer);
			m_timers.push_back(Neuron_Sensor_Timer());
		}

		m_timers[i].m_timer = genes.m_timerSensorData[i].m_initTime;
		m_timers[i].m_rate = genes.m_timerSensorData[i].m_rate;
		m_timers[i].m_pLimb = this;
		m_timers[i].Create();

		inputs.push_back(&m_timers[i]);
	}

	// Trim away unused timer data
	if(genes.m_numTimerSensors < genes.m_timerSensorData.size())
		genes.m_timerSensorData.resize(genes.m_numTimerSensors);

	// Add child limb inputs
	if(!m_children.empty())
		for(unsigned int i = 0, size = genes.m_childLimbInputNeurons.size(); i < size; i++)
		{
			if(genes.m_childLimbInputNeurons[i].m_childIndex >= static_cast<signed>(m_children.size()))
				genes.m_childLimbInputNeurons[i].m_childIndex = rand() % m_children.size();

			Limb* pChild = m_children[genes.m_childLimbInputNeurons[i].m_childIndex];

			if(genes.m_childLimbInputNeurons[i].m_outputIndex >= static_cast<signed>(pChild->m_numOutputs))
				genes.m_childLimbInputNeurons[i].m_outputIndex = rand() % pChild->m_numOutputs;

			inputs.push_back(&pChild->m_neurons[pChild->GetNumNeurons() - genes.m_childLimbInputNeurons[i].m_outputIndex - 1]);
		}

	// Add outputs as recurrent
	for(unsigned int i = 0; i < m_numOutputs; i++)
		inputs.push_back(&m_neurons[GetNumNeurons() - i - 1]);

	const unsigned int numInputs = inputs.size();

	// First hidden layer
	for(unsigned int ni = 0; ni < genes.m_numNeuronsPerHiddenLayer; ni++)
	{
		// Create neuron if it does not exist
		if(ni >= genes.m_neuronData.size())
		{
			while(ni >= genes.m_neuronData.size())
			{
				LimbGenes::Neuron_Data data;

				// Random assignment
				data.m_threshold = Randf();

				for(unsigned int j = 0; j < numInputs; j++)
					data.m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));

				genes.m_neuronData.push_back(data);
			}
		}
		else if(genes.m_neuronData[ni].m_weights.size() != numInputs) // Remove/Add Random inputs if size mismatch
		{
			const unsigned int numWeights = genes.m_neuronData[ni].m_weights.size();

			// Add random weights if there are not enough
			if(numWeights < numInputs)
			{
				const unsigned int difference = numInputs - numWeights;

				for(unsigned int j = 0; j < difference; j++)
					genes.m_neuronData[ni].m_weights.push_back(Randf(Neuron::s_minRandomWeight, Neuron::s_maxRandomWeight));
			}
			else
				genes.m_neuronData[ni].m_weights.resize(numInputs);
		}

		// Create weight array for neuron to take over
		const unsigned int numWeights = genes.m_neuronData[ni].m_weights.size();

		Neuron::NeuronInputAndWeight* pNeuronInputs = new Neuron::NeuronInputAndWeight[numWeights];

		for(unsigned int j = 0; j < numWeights; j++)
		{
			pNeuronInputs[j].m_pInput = inputs[j];
			pNeuronInputs[j].m_weight = genes.m_neuronData[ni].m_weights[j];
		}

		m_neurons[ni].SetInputs(numWeights, pNeuronInputs);
	}
}

void Limb::PhysicsUpdate()
{
	m_pMotionState->getWorldTransform(m_physicsTransform);

	for(unsigned int i = 0, numTimers = m_timers.size(); i < numTimers; i++)
		m_timers[i].PhysicsUpdate();

	if(m_pContactSensor.get() != NULL)
		m_pContactSensor->PhysicsUpdate();

	if(m_pJoint != NULL)
	{
		if(m_reflected)
		{
			m_motorInputs[0].m_output = -m_pJoint->getAngle(0) / pif_times_2;
			m_motorInputs[1].m_output = -m_pJoint->getAngle(1) / pif_times_2;
			m_motorInputs[2].m_output = -m_pJoint->getAngle(2) / pif_times_2;
		}
		else
		{
			m_motorInputs[0].m_output = m_pJoint->getAngle(0) / pif_times_2;
			m_motorInputs[1].m_output = m_pJoint->getAngle(1) / pif_times_2;
			m_motorInputs[2].m_output = m_pJoint->getAngle(2) / pif_times_2;
		}
		/*m_motorInputs[0].m_output = m_pJoint->getRotationalLimitMotor(0)->m_currentPosition / pif_times_2;
		m_motorInputs[1].m_output = m_pJoint->getRotationalLimitMotor(1)->m_currentPosition / pif_times_2;
		m_motorInputs[2].m_output = m_pJoint->getRotationalLimitMotor(2)->m_currentPosition / pif_times_2;*/
		// Get joint angle inputs
		/*btTransform parentLocal(m_pParent->m_pRigidBody->getWorldTransform().inverse() * m_pRigidBody->getWorldTransform());
		Vec3f euler(cons(parentLocal.getRotation()).GetEulerAngles());
		m_motorInputs[0].m_output = euler.x / pif_times_2;
		m_motorInputs[1].m_output = euler.y / pif_times_2;
		m_motorInputs[2].m_output = euler.z / pif_times_2;*/
	}
}

void Limb::StartMotorAveraging()
{
	// Init to 0
	for(unsigned int i = 0; i < s_numMotorOutputs; i++)
		m_motorOutputs[i] = 0.0f;
}

void Limb::NeuronUpdate(float dopamine)
{
	if(dopamine == 0.0f)
	{
		for(unsigned int i = 0, numNeurons = m_neurons.size(); i < numNeurons; i++)
			m_neurons[i].Update();
	}
	else
	{
		for(unsigned int i = 0, numNeurons = m_neurons.size(); i < numNeurons; i++)
			m_neurons[i].Update_Reinforce(dopamine);
	}
}

void Limb::AddMotorAverages(float weighting)
{
	// Add outputs to averages
	const unsigned int numNeurons = m_neurons.size();

	assert(numNeurons >= s_numMotorOutputs);

	for(unsigned int i = 0; i < s_numMotorOutputs; i++)
		m_motorOutputs[i] += NeuronInput::GetFireRateValue(m_neurons[numNeurons - i - 1].GetTimeSinceLastFire()) * weighting;
}

void Limb::MotorUpdate()
{
	if(m_pJoint == NULL)
		return;

	const unsigned int numNeurons = GetNumNeurons();

	assert(numNeurons >= s_numMotorOutputs);
	assert(s_numMotorOutputs == 14);

	const float maxVelocity = 0.0f; // MAXIMUM

	// Last s_numMotorOutputs neurons are outputs
	// Get desired angles
	float dax, day, daz;

	if(m_reflected)
	{
		dax = -((m_motorOutputs[0] + m_motorOutputs[1] - (m_motorOutputs[2] + m_motorOutputs[3])) / 2.0f) * m_bendLimit;
		day = -((m_motorOutputs[4] + m_motorOutputs[5] - (m_motorOutputs[6] + m_motorOutputs[7])) / 2.0f) * m_bendLimit;
		daz = -((m_motorOutputs[8] + m_motorOutputs[9] - (m_motorOutputs[10] + m_motorOutputs[11])) / 2.0f) * m_twistLimit;
	}
	else
	{
		dax = ((m_motorOutputs[0] + m_motorOutputs[1] - (m_motorOutputs[2] + m_motorOutputs[3])) / 2.0f) * m_bendLimit;
		day = ((m_motorOutputs[4] + m_motorOutputs[5] - (m_motorOutputs[6] + m_motorOutputs[7])) / 2.0f) * m_bendLimit;
		daz = ((m_motorOutputs[8] + m_motorOutputs[9] - (m_motorOutputs[10] + m_motorOutputs[11])) / 2.0f) * m_twistLimit;
	}

	float maxStrength = m_strength * (m_motorOutputs[12] + m_motorOutputs[13]) / 2.0f; // Max strength (force)

	// Update PID's
	m_PID_x.m_desiredValue = 0.0f;//dax;
	m_PID_y.m_desiredValue = 0.0f;//day;
	m_PID_z.m_desiredValue = 0.0f;//daz;

	m_PID_x.Update(m_pJoint->getAngle(0) / m_bendLimit, m_pCreature->GetScene()->m_frameTimer.GetTimeMultiplier());
	m_PID_y.Update(m_pJoint->getAngle(1) / m_bendLimit, m_pCreature->GetScene()->m_frameTimer.GetTimeMultiplier());
	m_PID_z.Update(m_pJoint->getAngle(2) / m_twistLimit, m_pCreature->GetScene()->m_frameTimer.GetTimeMultiplier());

	// X
	if(m_PID_x.m_output < 0.0f)
	{
		m_pJoint->getRotationalLimitMotor(0)->m_targetVelocity = -maxVelocity;
		m_pJoint->getRotationalLimitMotor(0)->m_maxMotorForce = Clamp(absf(m_PID_x.m_output), 0.0f, maxStrength);
	}
	else
	{
		m_pJoint->getRotationalLimitMotor(0)->m_targetVelocity = maxVelocity;
		m_pJoint->getRotationalLimitMotor(0)->m_maxMotorForce = Clamp(absf(m_PID_x.m_output), 0.0f, maxStrength);
	}

	// Y
	if(m_PID_y.m_output < 0.0f)
	{
		m_pJoint->getRotationalLimitMotor(1)->m_targetVelocity = -maxVelocity;
		m_pJoint->getRotationalLimitMotor(1)->m_maxMotorForce = Clamp(absf(m_PID_y.m_output), 0.0f, maxStrength);
	}
	else
	{
		m_pJoint->getRotationalLimitMotor(1)->m_targetVelocity = maxVelocity;
		m_pJoint->getRotationalLimitMotor(1)->m_maxMotorForce = Clamp(absf(m_PID_y.m_output), 0.0f, maxStrength);
	}

	// Z
	if(m_PID_z.m_output < 0.0f)
	{
		m_pJoint->getRotationalLimitMotor(2)->m_targetVelocity = -maxVelocity;
		m_pJoint->getRotationalLimitMotor(2)->m_maxMotorForce = Clamp(absf(m_PID_z.m_output), 0.0f, maxStrength);
	}
	else
	{
		m_pJoint->getRotationalLimitMotor(2)->m_targetVelocity = maxVelocity;
		m_pJoint->getRotationalLimitMotor(2)->m_maxMotorForce = Clamp(absf(m_PID_z.m_output), 0.0f, maxStrength);
	}

	//std::cout << x << " " << y << " " << z << " " << strength << std::endl;
}

void Limb::Render()
{
	assert(m_pModel != NULL);

	m_pCreature->GetScene()->SetWorldMatrix(Matrix4x4f::IdentityMatrix());

	Matrix4x4f glTransform;
	m_physicsTransform.getOpenGLMatrix(glTransform.m_elements);

	glTransform *= Matrix4x4f::ScaleMatrix(m_halfDims);

	m_pModel->Render(glTransform);
}

bool Limb::PhysicsWorldAlive() const
{
	return m_pCreature->m_physicsWorldTracker.ReferenceAlive();
}

SceneObject_PhysicsWorld* Limb::GetPhysicsWorld() const
{
	assert(m_pCreature != NULL);
	return m_pCreature->m_pPhysicsWorld;
}