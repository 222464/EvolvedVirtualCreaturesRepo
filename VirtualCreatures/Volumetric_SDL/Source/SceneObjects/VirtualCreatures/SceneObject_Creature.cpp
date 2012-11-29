#include <SceneObjects/VirtualCreatures/SceneObject_Creature.h>

#include <Scene/Scene.h>

CreatureGenes::CreatureGenes()
{
}

CreatureGenes::CreatureGenes(const CreatureGenes &other)
{
	m_mainBrainOutputs = other.m_mainBrainOutputs;

	for(unsigned int i = 0, size = other.m_pLimbGenes.size(); i < size; i++)
		m_pLimbGenes.push_back(new LimbGenes(*other.m_pLimbGenes[i]));
}

CreatureGenes::~CreatureGenes()
{
	for(unsigned int i = 0, size = m_pLimbGenes.size(); i < size; i++)
		delete m_pLimbGenes[i];
}

CreatureGenes &CreatureGenes::operator=(const CreatureGenes &other)
{
	// Self-assignment
	if(this == &other)
		return *this;

	m_mainBrainOutputs = other.m_mainBrainOutputs;

	for(unsigned int i = 0, size = m_pLimbGenes.size(); i < size; i++)
		delete m_pLimbGenes[i];

	m_pLimbGenes.clear();

	for(unsigned int i = 0, size = other.m_pLimbGenes.size(); i < size; i++)
		m_pLimbGenes.push_back(new LimbGenes(*other.m_pLimbGenes[i]));

	return *this;
}

SceneObject_Creature::SceneObject_Creature()
	: m_pRoot(NULL), m_pPhysicsWorld(NULL), m_dopamine(0.0f)
{
}

SceneObject_Creature::~SceneObject_Creature()
{
	for(unsigned int i = 0, size = m_pLimbs.size(); i < size; i++)
		delete m_pLimbs[i];
}

void SceneObject_Creature::UpdateAABB()
{
	m_aabb.m_lowerBound = m_aabb.m_upperBound = cons(m_pLimbs[0]->m_pRigidBody->getWorldTransform().getOrigin());

	for(unsigned int i = 1, size = m_pLimbs.size(); i < size; i++)
	{
		btVector3 min, max;
		m_pLimbs[i]->m_pRigidBody->getAabb(min, max);

		// Expand AABB
		if(min.getX() < m_aabb.m_lowerBound.x)
			m_aabb.m_lowerBound.x = min.getX();
		if(min.getY() < m_aabb.m_lowerBound.y)
			m_aabb.m_lowerBound.y = min.getY();
		if(min.getZ() < m_aabb.m_lowerBound.z)
			m_aabb.m_lowerBound.z = min.getZ();
		if(max.getX() > m_aabb.m_upperBound.x)
			m_aabb.m_upperBound.x = max.getX();
		if(max.getY() > m_aabb.m_upperBound.y)
			m_aabb.m_upperBound.y = max.getY();
		if(max.getZ() > m_aabb.m_upperBound.z)
			m_aabb.m_upperBound.z = max.getZ();
	}

	m_aabb.CalculateHalfDims();
	m_aabb.CalculateCenter();

	if(IsSPTManaged())
		TreeUpdate();
}

void SceneObject_Creature::OnAdd()
{
	// Get reference to physics world
	m_pPhysicsWorld = static_cast<SceneObject_PhysicsWorld*>(GetScene()->GetNamed_SceneObject("physWrld"));

	m_physicsWorldTracker.Set(m_pPhysicsWorld);

	assert(m_pPhysicsWorld != NULL);

	// Get references to models
	Asset* pAsset;

	AssetManager* pOBJManager = GetScene()->GetAssetManager_AutoCreate("obj", Model_OBJ::Asset_Factory);

	if(!pOBJManager->GetAsset("data/models/box2.obj", pAsset))
		abort();

	m_pLimbModel_Sensor = static_cast<Model_OBJ*>(pAsset);
	m_pLimbModel_Sensor->SetRenderer(GetScene());

	if(!pOBJManager->GetAsset("data/models/box2.obj", pAsset))
		abort();

	m_pLimbModel_Root = static_cast<Model_OBJ*>(pAsset);
	m_pLimbModel_Root->SetRenderer(GetScene());

	if(!pOBJManager->GetAsset("data/models/box2.obj", pAsset))
		abort();

	m_pLimbModel_Other = static_cast<Model_OBJ*>(pAsset);
	m_pLimbModel_Other->SetRenderer(GetScene());
}

void SceneObject_Creature::CreateFromGenes(const CreatureGenes &genes, const btTransform &baseTransform)
{
	assert(GetScene() != NULL);
	assert(m_pPhysicsWorld != NULL);
	assert(m_pRoot == NULL);
	assert(m_pLimbs.empty());

	/// TODO: HANDLE RECURSION AND SYMMETRY IN LIMBS!!!

	// Create all limbs
	const unsigned int numLimbs = genes.m_pLimbGenes.size();

	assert(numLimbs > 0);

	m_pLimbs.reserve(numLimbs);

	// Root is done manually
	m_pRoot = new Limb();

	m_pRoot->Create_Root(this, baseTransform, *genes.m_pLimbGenes[0], genes.m_mainBrainOutputs);

	m_pLimbs.push_back(m_pRoot);

	for(unsigned int i = 1; i < numLimbs; i++)
	{
		Limb* pLimb = new Limb();

		// Check parent bounds here
		// TODO: FIX IT FOR REAL (IN THE EVOLVER), NOT WITH THIS CHEAT
		if(genes.m_pLimbGenes[i]->m_parentIndexOffset > static_cast<signed>(i))
			genes.m_pLimbGenes[i]->m_parentIndexOffset = static_cast<signed>(i);

		int parentIndex = i - genes.m_pLimbGenes[i]->m_parentIndexOffset;
		
		assert(parentIndex >= 0 && parentIndex < static_cast<signed>(i));

		pLimb->Create(this, m_pLimbs[parentIndex], *genes.m_pLimbGenes[i]);

		// Add as child
		m_pLimbs[parentIndex]->m_children.push_back(pLimb);

		m_pLimbs.push_back(pLimb);

		// Additional recursive units (TODO: handle symmetry here as well)
		if(genes.m_pLimbGenes[i]->m_recursiveUnits != 0)
		{
			// Search for limbs that inherit recursion from this one
			std::vector<LimbGenes*> pInheritingLimbGenes;

			for(unsigned int j = i; j < numLimbs; j++)
			{
				if(!genes.m_pLimbGenes[j]->m_inheritsRecursion)
					continue;

				// Check parent bounds here
				// TODO: FIX IT FOR REAL (IN THE EVOLVER), NOT WITH THIS CHEAT
				if(genes.m_pLimbGenes[j]->m_parentIndexOffset > static_cast<signed>(j))
					genes.m_pLimbGenes[j]->m_parentIndexOffset = static_cast<signed>(j);

				int parentIndex = j - genes.m_pLimbGenes[j]->m_parentIndexOffset;
		
				if(parentIndex == i)
					pInheritingLimbGenes.push_back(genes.m_pLimbGenes[j]);
			}

			for(int j = 0; j < genes.m_pLimbGenes[i]->m_recursiveUnits; j++)
			{
				Limb* pRecursiveLimb = new Limb();

				// Previous limb is parent
				pRecursiveLimb->Create(this, m_pLimbs.back(), *genes.m_pLimbGenes[i]);

				// Add as child for previous limb
				m_pLimbs.back()->m_children.push_back(pRecursiveLimb);

				m_pLimbs.push_back(pRecursiveLimb);

				// Add inheriting limbs to this limb
				for(unsigned int k = 0, numInheritingLimbs = pInheritingLimbGenes.size(); k < numInheritingLimbs; k++)
				{
					Limb* pInheritingLimb = new Limb();

					// Previous limb is parent
					pInheritingLimb->Create(this, pRecursiveLimb, *genes.m_pLimbGenes[i]);

					// Add as child for previous limb
					pRecursiveLimb->m_children.push_back(pInheritingLimb);

					m_pLimbs.push_back(pInheritingLimb);
				}
			}
		}
	}

	// Create all input sets
	std::vector<NeuronInput*> empty;
	m_pLimbs[0]->SetInputs_Root(*genes.m_pLimbGenes[0], empty);

	for(unsigned int i = 1; i < numLimbs; i++)
		m_pLimbs[i]->SetInputs_SubNet(*genes.m_pLimbGenes[i]);
}

void SceneObject_Creature::Logic()
{
	const unsigned int numLimbs = m_pLimbs.size();

	for(unsigned int i = 0; i < numLimbs; i++)
		m_pLimbs[i]->PhysicsUpdate();

	// Update all neurons - propagation step
	for(unsigned int cycle = 0; cycle < s_numNeuralNetCycles_propagation; cycle++)
		for(unsigned int i = 0; i < numLimbs; i++)
			m_pLimbs[i]->NeuronUpdate(m_dopamine);

	// Update some more, but this time average some outputs
	for(unsigned int i = 0; i < numLimbs; i++)
		m_pLimbs[i]->StartMotorAveraging();

	float weighting = 1.0f / static_cast<float>(s_numNeuralNetCycles_outputAverage);

	for(unsigned int cycle = 0; cycle < s_numNeuralNetCycles_outputAverage; cycle++)
	{
		for(unsigned int i = 0; i < numLimbs; i++)
			m_pLimbs[i]->NeuronUpdate(m_dopamine);

		for(unsigned int i = 0; i < numLimbs; i++)
			m_pLimbs[i]->AddMotorAverages(weighting);
	}

	for(unsigned int i = 0; i < numLimbs; i++)
		m_pLimbs[i]->MotorUpdate();

	UpdateAABB();
}

void SceneObject_Creature::Render()
{
	const unsigned int numLimbs = m_pLimbs.size();

	for(unsigned int i = 0; i < numLimbs; i++)
		m_pLimbs[i]->Render();
}