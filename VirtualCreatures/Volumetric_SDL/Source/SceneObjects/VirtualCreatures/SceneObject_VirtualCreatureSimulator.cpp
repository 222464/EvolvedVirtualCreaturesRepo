#include <SceneObjects/VirtualCreatures/SceneObject_VirtualCreatureSimulator.h>

#include <Utilities/UtilFuncs.h>

#include <Scene/Scene.h>

SceneObject_VirtualCreatureSimulator::SceneObject_VirtualCreatureSimulator()
	: m_generationTimer(0.0f), m_generationTime(400.0f),
	m_setStartTimer(0.0f), m_setStartTime(160.0f), m_generation(0)
{
}

SceneObject_VirtualCreatureSimulator::~SceneObject_VirtualCreatureSimulator()
{
	ClearPhenotypes();
}

bool SceneObject_VirtualCreatureSimulator::Create(const std::string &configFileName)
{
	assert(GetScene() != NULL);

	if(!m_evolver.Create(configFileName))
		return false;

	CreatePhenotypes();

	return true;
}

void SceneObject_VirtualCreatureSimulator::OnAdd()
{
	// Get reference to world
	m_pWorld = static_cast<World*>(GetScene()->GetNamed_SceneObject("world"));

	assert(m_pWorld != NULL);
}

void SceneObject_VirtualCreatureSimulator::Logic()
{
	if(!m_pPhenotypes.empty())
	{
		if(m_generationTimer > m_generationTime)
		{
			m_generationTimer = 0.0f;
			m_setStartTimer = 0.0f;

			m_evolver.SetFitness(m_fitnesses);

			m_evolver.Generation();
	
			// Delete phenotypes, re-create
			ClearPhenotypes();
			CreatePhenotypes();

			m_generation++;

			std::cout << "Generation " << m_generation << " completed." << std::endl;
		}
		else
		{
			if(m_setStartTimer >= 0.0f)
			{
				if(m_setStartTimer > m_setStartTime)
				{
					m_setStartTimer = -1.0f;
					std::cout << "dfdsf";
					// Set start positions
					for(unsigned int i = 0, size = m_evolver.GetNumCreatures(); i < size; i++)
						m_startingPositions[i] = m_pPhenotypes[i].m_pCreature->GetPosition();
				}
				else
					m_setStartTimer += GetScene()->m_frameTimer.GetTimeMultiplier();
			}
			else
				CalulateFitness();

			m_generationTimer += GetScene()->m_frameTimer.GetTimeMultiplier();
		}
	}
}

void SceneObject_VirtualCreatureSimulator::ClearPhenotypes()
{
	assert(GetScene() != NULL);

	for(unsigned int i = 0, size = m_pPhenotypes.size(); i < size; i++)
	{
		if(m_pPhenotypes[i].m_tracker.ReferenceAlive())
			m_pPhenotypes[i].m_pCreature->Destroy();
	}

	m_pPhenotypes.clear();

	m_fitnesses.clear();
	m_startingPositions.clear();
}

void SceneObject_VirtualCreatureSimulator::CreatePhenotypes()
{
	assert(GetScene() != NULL);
	assert(m_pPhenotypes.empty());
	assert(m_startingPositions.empty());
	assert(m_fitnesses.empty());
	assert(m_evolver.GetNumCreatures() != 0);

	m_pPhenotypes.clear();

	m_pPhenotypes.reserve(m_evolver.GetNumCreatures());
	m_startingPositions.reserve(m_evolver.GetNumCreatures());

	m_fitnesses.assign(m_evolver.GetNumCreatures(), 0.0f);

	for(unsigned int i = 0, size = m_evolver.GetNumCreatures(); i < size; i++)
	{
		CreatureAndTracker creatureAndTracker;
		creatureAndTracker.m_pCreature = new SceneObject_Creature();
		creatureAndTracker.m_tracker.Set(creatureAndTracker.m_pCreature);

		m_pPhenotypes.push_back(creatureAndTracker);

		btTransform trans;
		trans.setIdentity();

		m_startingPositions.push_back(Vec3f(Randf() * 192.0f + 32.0f, 12.0f, Randf() * 192.0f + 32.0f));
		trans.setOrigin(bt(m_startingPositions[i]));

		GetScene()->Add(creatureAndTracker.m_pCreature, true);

		creatureAndTracker.m_pCreature->CreateFromGenes(*m_evolver.GetGenotype(i), trans);
	}
}

void SceneObject_VirtualCreatureSimulator::CalulateFitness()
{
	for(unsigned int i = 0, size = m_evolver.GetNumCreatures(); i < size; i++)
	{
		// Get positions
		float newFitness = (Vec3f(m_startingPositions[i].x, 0.0f, m_startingPositions[i].z) - Vec3f(m_pPhenotypes[i].m_pCreature->GetPosition().x, 0.0f, m_pPhenotypes[i].m_pCreature->GetPosition().z)).Magnitude();
		
		// Reinforcement learning with delta fitness based dopamine
		//m_pPhenotypes[i].m_pCreature->m_dopamine = (newFitness - m_fitnesses[i]) / 10.0f;

		m_fitnesses[i] = newFitness;
	}
}