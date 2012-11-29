#pragma once

#include <SceneObjects/VirtualCreatures/SceneObject_Creature.h>

#include <list>
#include <vector>

// Uncomment below in order to mutate elites
#define MUTATE_ELITES

class CreatureEvolver :
	public Uncopyable
{
private:
	struct FitnessInfo
	{
		float m_fitness;
		unsigned int m_creatureIndex;
	};

	std::vector<CreatureGenes*> m_genotypePopulation;
	std::list<FitnessInfo> m_fitnesses;

	// GA Parameters (loaded from file)
	unsigned int m_populationSize;
	float m_crossoverRate;

	float m_contactSensorMutationRate;
	float m_initContactSensorChance;

	float m_neuralStructureMutationRate_numLayers;
	float m_neuralStructureMutationRate_numNeuronsPerHiddenLayer;
	unsigned int m_minHiddenLayers;
	unsigned int m_maxHiddenLayers;
	unsigned int m_minNeuronsPerHiddenLayer;
	unsigned int m_maxNeuronsPerHiddenLayer;
	float m_maxNeuronsPerHiddenLayerPerturbation;
	float m_mainBrainInputNeuronsMutationRate;
	unsigned int m_maxNumMainBrainInputNeurons;
	float m_mainBrainOutputsMutationRate;
	unsigned int m_minMainBrainOutputs;
	unsigned int m_maxMainBrainOutputs;
	float m_parentLimbInputNeuronsMutationRate;
	unsigned int m_maxNumParentLimbInputNeurons;
	float m_childLimbInputNeuronsMutationRate;
	unsigned int m_maxNumChildLimbInputNeurons;
	float m_neuralWeightMutationRate;
	float m_neuralWeightPerturbation;
	float m_minPossibleNeuronWeight;
	float m_maxPossibleNeuronWeight;

	float m_minInitTimerCount;
	float m_maxInitTimerCount;
	float m_minInitTimerRate;
	float m_maxInitTimerRate;
	unsigned int m_minTimerCount;
	unsigned int m_maxTimerCount;
	float m_minTimerRate;
	float m_maxTimerRate;
	float m_timerCountMutationRate;
	float m_timerRateMutationRate;
	float m_timerTimeMutationRate;
	float m_minPossibleTimerRate;
	float m_maxPossibleTimerRate;
	float m_maxTimerRatePerturbation;
	float m_maxTimerTimePerturbation;

	float m_densityMutationRate;
	float m_maxDensityPerturbation;
	float m_minDensity;
	float m_maxDensity;
	float m_frictionMutationRate;
	float m_maxFrictionPerturbation;
	float m_minFriction;
	float m_maxFriction;

	float m_dimensionalMutationRate;
	float m_maxLimbDimensionPerturbation;
	float m_minLimbDimension;
	float m_maxLimbDimension;

	float m_relativeOffsetRatioMutationRate;
	float m_maxRelativeOffsetRatioPerturbation;

	float m_limbBendAndTwistMutationRate;
	float m_maxLimbBendAndTwistPerturbation;
	float m_minLimbTwist;
	float m_minLimbBend;
	float m_maxLimbTwist;
	float m_maxLimbBend;
	
	float m_limbAngleMutationRate;
	float m_maxLimbAnglePerturbation;
	float m_maxLimbAngle;
	
	float m_limbStrengthMutationRate;
	float m_maxLimbStrengthPerturbation;
	float m_minLimbStrength;
	float m_maxLimbStrength;

	float m_limbCountMutationRate;
	float m_maxLimbPerturbation; // Cast to an int
	int m_minNumLimbs;
	int m_maxNumLimbs;

	float m_limbParentMutationRate;

	float m_recursiveLimbMutationRate_whenRecursive;
	float m_recursiveLimbMutationRate_whenNotRecursive;
	float m_symmetricalLimbMutationRate_whenSymmetrical;
	float m_symmetricalLimbMutationRate_whenNotSymmetrical;
	float m_recursiveLimbUnitCountChangeRate;
	float m_initRecursiveLimbChance;
	float m_initSymetricalLimbChance;
	float m_inheritRecursionMutationRate;
	float m_inheritSymmetryMutationRate;
	float m_initInheritRecursionChance;
	float m_initInheritSymmetryChance;
	int m_minInitRecursiveUnits;
	int m_maxInitRecursiveUnits;
	int m_maxRecursiveUnits;
	float m_numBranchesMutationRate;
	int m_maxNumBranches;

	unsigned int m_numElites;

	// Evolution-time
	float m_totalFitness;

	// Helpers
	void InitGenes(CreatureGenes* pGenes);
	void InitGenes(LimbGenes* pGenes, unsigned int index);
	void Crossover(CreatureGenes* pGenes1, CreatureGenes* pGenes2);
	void Mutate(CreatureGenes* pGenes);
	void AttachmentPositionInit_Parent(Vec3f &position); // Returns "normal" quaternion
	void AttachmentPositionInit_This(Vec3f &position);
	void AttachmentPositionMutate_Parent(Vec3f &position); // Returns "normal" quaternion
	Quaternion NormalQuaternionFromPosition(const Vec3f &position);
	Vec3f NormalFromPosition(const Vec3f &position);
	void AttachmentPositionMutate_This(Vec3f &position);
	void GetTotalFitness();
	unsigned int RouletteSelection();

	void FindElites(std::vector<CreatureGenes> &genes);
	void RemoveChumps(std::vector<unsigned int> &chumpIndices);

public:
	bool m_checkPositiveFitness;

	CreatureEvolver();
	~CreatureEvolver();

	bool Create(const std::string &configFileName);

	void SetFitness(const std::vector<float> &fitnesses);
	void Generation();

	CreatureGenes* GetGenotype(unsigned int index)
	{
		return m_genotypePopulation[index];
	}

	unsigned int GetNumCreatures() const
	{
		return m_populationSize;
	}
};

template<class T>
void CrossOver_VaryingSize(std::vector<T> &g1, std::vector<T> &g2)
{
	float crossOverRatio = Randf();

	const int geneSize1 = static_cast<signed>(g1.size());
	const int geneSize2 = static_cast<signed>(g2.size());

	int crossOverPoint1 = static_cast<int>(crossOverRatio * geneSize1);
	int crossOverPoint2 = static_cast<int>(crossOverRatio * geneSize2);

	// Check bounds and adapt
	if(crossOverPoint1 == 0)
	{
		crossOverPoint1++;

		// No crossover
		if(crossOverPoint1 > geneSize1)
			return;
	}

	if(crossOverPoint2 == 0)
	{
		crossOverPoint2++;

		// No crossover
		if(crossOverPoint2 > geneSize2)
			return;
	}

	const int dSize1 = crossOverPoint2 - crossOverPoint1;

	std::vector<T> newGenes1;
	newGenes1.reserve(geneSize1 + dSize1);
	std::vector<T> newGenes2;
	newGenes2.reserve(geneSize2 - dSize1);

	for(int j = 0; j < crossOverPoint2; j++)
		newGenes1.push_back(g2[j]);

	for(int j = crossOverPoint1; j < geneSize1; j++)
		newGenes1.push_back(g1[j]);

	for(int j = 0; j < crossOverPoint1; j++)
		newGenes2.push_back(g1[j]);

	for(int j = crossOverPoint2; j < geneSize2; j++)
		newGenes2.push_back(g2[j]);

	assert(!newGenes1.empty());
	assert(!newGenes2.empty());

	// Copy back
	g1 = newGenes1;
	g2 = newGenes2;
}