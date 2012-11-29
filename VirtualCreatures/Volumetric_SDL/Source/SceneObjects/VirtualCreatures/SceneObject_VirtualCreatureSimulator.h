#pragma once

#include <Scene/SceneObject.h>
#include <SceneObjects/VirtualCreatures/CreatureEvolver.h>

#include <World/World.h>

class SceneObject_VirtualCreatureSimulator :
	public SceneObject
{
private:
	struct CreatureAndTracker
	{
		SceneObjectReferenceTracker m_tracker;
		SceneObject_Creature* m_pCreature;
	};

	CreatureEvolver m_evolver;

	std::vector<CreatureAndTracker> m_pPhenotypes;
	std::vector<Vec3f> m_startingPositions;
	std::vector<float> m_fitnesses;

	float m_generationTimer;

	float m_setStartTimer;

	World* m_pWorld;

	void CalulateFitness();

	unsigned int m_generation;

public:
	float m_generationTime;
	float m_setStartTime;

	SceneObject_VirtualCreatureSimulator();
	~SceneObject_VirtualCreatureSimulator();

	bool Create(const std::string &configFileName);

	// Inherited from SceneObject
	void OnAdd();
	void Logic();

	void ClearPhenotypes();
	void CreatePhenotypes();
};

