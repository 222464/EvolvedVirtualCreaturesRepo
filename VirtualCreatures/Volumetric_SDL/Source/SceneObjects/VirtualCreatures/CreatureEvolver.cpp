#include <SceneObjects/VirtualCreatures/CreatureEvolver.h>

#include <Utilities/UtilFuncs.h>
#include <iostream>
#include <assert.h>
#include <fstream>

CreatureEvolver::CreatureEvolver()
	: m_checkPositiveFitness(true)
{
}

CreatureEvolver::~CreatureEvolver()
{
	// Delete the population
	for(unsigned int i = 0, size = m_genotypePopulation.size(); i < size; i++)
		delete m_genotypePopulation[i];
}

bool CreatureEvolver::Create(const std::string &configFileName)
{
	std::fstream fromFile(configFileName);

	if(!fromFile.is_open())
	{
		std::cerr << "Could not open file " << configFileName << "!" << std::endl;
		return false;
	}

	std::string param;

	fromFile >> param >> m_populationSize;
	fromFile >> param >> m_crossoverRate;

	fromFile >> param >> m_contactSensorMutationRate;
	fromFile >> param >> m_initContactSensorChance;

	fromFile >> param >> m_neuralStructureMutationRate_numLayers;
	fromFile >> param >> m_neuralStructureMutationRate_numNeuronsPerHiddenLayer;
	fromFile >> param >> m_minHiddenLayers;
	fromFile >> param >> m_maxHiddenLayers;
	fromFile >> param >> m_minNeuronsPerHiddenLayer;
	fromFile >> param >> m_maxNeuronsPerHiddenLayer;
	fromFile >> param >> m_maxNeuronsPerHiddenLayerPerturbation;
	fromFile >> param >> m_mainBrainInputNeuronsMutationRate;
	fromFile >> param >> m_maxNumMainBrainInputNeurons;
	fromFile >> param >> m_mainBrainOutputsMutationRate;
	fromFile >> param >> m_minMainBrainOutputs;
	fromFile >> param >> m_maxMainBrainOutputs;
	fromFile >> param >> m_parentLimbInputNeuronsMutationRate;
	fromFile >> param >> m_maxNumParentLimbInputNeurons;
	fromFile >> param >> m_childLimbInputNeuronsMutationRate;
	fromFile >> param >> m_maxNumChildLimbInputNeurons;
	fromFile >> param >> m_neuralWeightMutationRate;
	fromFile >> param >> m_neuralWeightPerturbation;
	fromFile >> param >> m_minPossibleNeuronWeight;
	fromFile >> param >> m_maxPossibleNeuronWeight;

	fromFile >> param >> m_minInitTimerCount;
	fromFile >> param >> m_maxInitTimerCount;
	fromFile >> param >> m_minInitTimerRate;
	fromFile >> param >> m_maxInitTimerRate;
	fromFile >> param >> m_minTimerCount;
	fromFile >> param >> m_maxTimerCount;
	fromFile >> param >> m_minTimerRate;
	fromFile >> param >> m_maxTimerRate;
	fromFile >> param >> m_timerCountMutationRate;
	fromFile >> param >> m_timerRateMutationRate;
	fromFile >> param >> m_timerTimeMutationRate;
	fromFile >> param >> m_minPossibleTimerRate;
	fromFile >> param >> m_maxPossibleTimerRate;
	fromFile >> param >> m_maxTimerRatePerturbation;
	fromFile >> param >> m_maxTimerTimePerturbation;

	fromFile >> param >> m_densityMutationRate;
	fromFile >> param >> m_maxDensityPerturbation;
	fromFile >> param >> m_minDensity;
	fromFile >> param >> m_maxDensity;
	fromFile >> param >> m_frictionMutationRate;
	fromFile >> param >> m_maxFrictionPerturbation;
	fromFile >> param >> m_minFriction;
	fromFile >> param >> m_maxFriction;

	fromFile >> param >> m_dimensionalMutationRate;
	fromFile >> param >> m_maxLimbDimensionPerturbation;
	fromFile >> param >> m_minLimbDimension;
	fromFile >> param >> m_maxLimbDimension;

	fromFile >> param >> m_relativeOffsetRatioMutationRate;
	fromFile >> param >> m_maxRelativeOffsetRatioPerturbation;

	fromFile >> param >> m_limbBendAndTwistMutationRate;
	fromFile >> param >> m_maxLimbBendAndTwistPerturbation;
	fromFile >> param >> m_minLimbTwist;
	fromFile >> param >> m_minLimbBend;
	fromFile >> param >> m_maxLimbTwist;
	fromFile >> param >> m_maxLimbBend;
	
	fromFile >> param >> m_limbAngleMutationRate;
	fromFile >> param >> m_maxLimbAnglePerturbation;
	fromFile >> param >> m_maxLimbAngle;
	
	fromFile >> param >> m_limbStrengthMutationRate;
	fromFile >> param >> m_maxLimbStrengthPerturbation;
	fromFile >> param >> m_minLimbStrength;
	fromFile >> param >> m_maxLimbStrength;

	fromFile >> param >> m_limbCountMutationRate;
	fromFile >> param >> m_maxLimbPerturbation;
	fromFile >> param >> m_minNumLimbs;
	fromFile >> param >> m_maxNumLimbs;

	fromFile >> param >> m_limbParentMutationRate;

	fromFile >> param >> m_recursiveLimbMutationRate_whenRecursive;
	fromFile >> param >> m_recursiveLimbMutationRate_whenNotRecursive;
	fromFile >> param >> m_symmetricalLimbMutationRate_whenSymmetrical;
	fromFile >> param >> m_symmetricalLimbMutationRate_whenNotSymmetrical;
	fromFile >> param >> m_recursiveLimbUnitCountChangeRate;
	fromFile >> param >> m_initRecursiveLimbChance;
	fromFile >> param >> m_initSymetricalLimbChance;
	fromFile >> param >> m_inheritRecursionMutationRate;
	fromFile >> param >> m_inheritSymmetryMutationRate;
	fromFile >> param >> m_initInheritRecursionChance;
	fromFile >> param >> m_initInheritSymmetryChance;
	fromFile >> param >> m_minInitRecursiveUnits;
	fromFile >> param >> m_maxInitRecursiveUnits;
	fromFile >> param >> m_maxRecursiveUnits;
	fromFile >> param >> m_numBranchesMutationRate;
	fromFile >> param >> m_maxNumBranches;

	fromFile >> param >> m_numElites;

	if(fromFile.fail())
	{
		std::cerr << "Could not properly read file " << configFileName << "!" << std::endl;

		fromFile.close();
		
		return false;
	}

	fromFile.close();

	// Create the genotype population
	m_genotypePopulation.reserve(m_populationSize);

	for(unsigned int i = 0; i < m_populationSize; i++)
	{
		CreatureGenes* pGenes = new CreatureGenes();

		InitGenes(pGenes);

		m_genotypePopulation.push_back(pGenes);
	}

	return true;
}

void CreatureEvolver::InitGenes(CreatureGenes* pGenes)
{
	assert(pGenes->m_pLimbGenes.empty());

	pGenes->m_mainBrainOutputs = rand() % (m_maxMainBrainOutputs - m_minMainBrainOutputs) + m_minMainBrainOutputs;

	unsigned int numLimbs = rand() % (m_maxNumLimbs - m_minNumLimbs) + m_minNumLimbs;

	pGenes->m_pLimbGenes.reserve(numLimbs);

	for(unsigned int i = 0; i < numLimbs; i++)
	{
		LimbGenes* pNewGenes = new LimbGenes();

		InitGenes(pNewGenes, i);

		pGenes->m_pLimbGenes.push_back(pNewGenes);
	}
}

void CreatureEvolver::InitGenes(LimbGenes* pGenes, unsigned int index)
{
	if(index != 0) // Not root
		pGenes->m_parentIndexOffset = rand() % index + 1;
	else
		pGenes->m_parentIndexOffset = -1;

	if(Randf() < m_initRecursiveLimbChance)
		pGenes->m_recursiveUnits = rand() % (m_maxInitRecursiveUnits - m_minInitRecursiveUnits) + m_minInitRecursiveUnits;
	else
		pGenes->m_recursiveUnits = 0;

	pGenes->m_symmetrical = Randf() < m_initSymetricalLimbChance;

	pGenes->m_inheritsRecursion = Randf() < m_initInheritRecursionChance;
	pGenes->m_inheritsSymmetry = Randf() < m_initInheritSymmetryChance;

	pGenes->m_numBranches = rand() % m_maxNumBranches;

	pGenes->m_dims.x = Randf(m_minLimbDimension, m_maxLimbDimension);
	pGenes->m_dims.y = Randf(m_minLimbDimension, m_maxLimbDimension);
	pGenes->m_dims.z = Randf(m_minLimbDimension, m_maxLimbDimension);

	AttachmentPositionInit_Parent(pGenes->m_relativeAttachmentPositionOnParent);
	Quaternion normalQuat(pGenes->m_relativeAttachmentPositionOnParent);
	AttachmentPositionInit_This(pGenes->m_relativeAttachmentPositionOnThis);

	pGenes->m_bendLimit = Randf() * m_maxLimbBend;
	pGenes->m_twistLimit = Randf() * m_maxLimbTwist;

	// Random axis, rotate around it
	Quaternion perturbedNormal(0.0f, 1.0f, 0.0f, 0.0f);
	// Set relative rotation as normal to surface
	Vec3f axis(Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f));
	axis.NormalizeThis();
	perturbedNormal.Rotate(Randf() * pGenes->m_bendLimit, axis);
	pGenes->m_relativeRotation = normalQuat * perturbedNormal; 
	
	pGenes->m_density = Randf(m_minDensity, m_maxDensity);
	pGenes->m_friction = Randf(m_minFriction, m_maxFriction);

	pGenes->m_strength = Randf(m_minLimbStrength, m_maxLimbStrength);
		
	pGenes->m_hasContactSensor = Randf() < m_initContactSensorChance;

	pGenes->m_numTimerSensors = static_cast<int>(Randf(m_minInitTimerCount, m_maxInitTimerCount));

	for(int i = 0, numMainBrainInputNeurons = rand() % m_maxNumMainBrainInputNeurons; i < numMainBrainInputNeurons; i++)
		pGenes->m_mainBrainInputNeurons.push_back(rand() % (m_maxNumMainBrainInputNeurons + 1));

	for(int i = 0, numParentLimbInputNeurons = rand() % m_maxNumParentLimbInputNeurons; i < numParentLimbInputNeurons; i++)
		pGenes->m_parentLimbInputNeurons.push_back(rand() % (m_maxNumParentLimbInputNeurons + 1));

	for(int i = 0, numChildLimbInputNeurons = rand() % m_maxNumChildLimbInputNeurons; i < numChildLimbInputNeurons; i++)
	{
		LimbGenes::ChildAndOutputIndex coai;
		coai.m_childIndex = rand() % m_maxNumBranches;
		coai.m_outputIndex = rand() % m_maxNumChildLimbInputNeurons;
		pGenes->m_childLimbInputNeurons.push_back(coai);
	}

	// Structure
	if(m_minHiddenLayers == m_maxHiddenLayers)
		pGenes->m_numHiddenLayers = m_minHiddenLayers;
	else
		pGenes->m_numHiddenLayers = rand() % (m_maxHiddenLayers - m_minHiddenLayers) + m_minHiddenLayers;
	
	pGenes->m_numNeuronsPerHiddenLayer = rand() % (m_maxNeuronsPerHiddenLayer - m_minNeuronsPerHiddenLayer) + m_minNeuronsPerHiddenLayer;
}

void CreatureEvolver::Crossover(CreatureGenes* pGenes1, CreatureGenes* pGenes2)
{
	// Swap around limbs
	CrossOver_VaryingSize(pGenes1->m_pLimbGenes, pGenes2->m_pLimbGenes);

	// Shuffle around if they are below the minimum size
	if(pGenes1->m_pLimbGenes.size() < static_cast<unsigned>(m_minNumLimbs))
	{
		while(pGenes1->m_pLimbGenes.size() < static_cast<unsigned>(m_minNumLimbs))
		{
			unsigned int randIndex = rand() % pGenes2->m_pLimbGenes.size();
			pGenes1->m_pLimbGenes.push_back(pGenes2->m_pLimbGenes[randIndex]);
			pGenes2->m_pLimbGenes.erase(pGenes2->m_pLimbGenes.begin() + randIndex);
		}
	}
	else if(pGenes2->m_pLimbGenes.size() < static_cast<unsigned>(m_minNumLimbs))
	{
		while(pGenes2->m_pLimbGenes.size() < static_cast<unsigned>(m_minNumLimbs))
		{
			unsigned int randIndex = rand() % pGenes1->m_pLimbGenes.size();
			pGenes2->m_pLimbGenes.push_back(pGenes1->m_pLimbGenes[randIndex]);
			pGenes1->m_pLimbGenes.erase(pGenes1->m_pLimbGenes.begin() + randIndex);
		}
	}

	unsigned int minNumLimbs, maxNumLimbs;

	std::vector<LimbGenes*>* pLargerGene;
	
	if(pGenes1->m_pLimbGenes.size() < pGenes2->m_pLimbGenes.size())
	{
		minNumLimbs = pGenes1->m_pLimbGenes.size();
		maxNumLimbs = pGenes2->m_pLimbGenes.size();
		pLargerGene = &pGenes2->m_pLimbGenes;
	}
	else
	{
		maxNumLimbs = pGenes1->m_pLimbGenes.size();
		minNumLimbs = pGenes2->m_pLimbGenes.size();
		pLargerGene = &pGenes1->m_pLimbGenes;
	}

	// For each limb, perform crossovers (partially uniform, partially point)
	for(unsigned int i = 0; i < minNumLimbs; i++)
	{
		// Make sure parent is in bounds
		if(i != 0)
		{
			if(static_cast<signed>(i) - pGenes1->m_pLimbGenes[i]->m_parentIndexOffset < 0)
				pGenes1->m_pLimbGenes[i]->m_parentIndexOffset = i;

			if(static_cast<signed>(i) - pGenes2->m_pLimbGenes[i]->m_parentIndexOffset < 0)
				pGenes2->m_pLimbGenes[i]->m_parentIndexOffset = i;
		}

		// Continuity properties
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_symmetrical, pGenes2->m_pLimbGenes[i]->m_symmetrical);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_recursiveUnits, pGenes2->m_pLimbGenes[i]->m_recursiveUnits);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_inheritsRecursion, pGenes2->m_pLimbGenes[i]->m_inheritsRecursion);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_inheritsSymmetry, pGenes2->m_pLimbGenes[i]->m_inheritsSymmetry);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_numBranches, pGenes2->m_pLimbGenes[i]->m_numBranches);

		// Dimensions
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_dims.x, pGenes2->m_pLimbGenes[i]->m_dims.x);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_dims.y, pGenes2->m_pLimbGenes[i]->m_dims.y);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_dims.z, pGenes2->m_pLimbGenes[i]->m_dims.z);

		// Joint
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_bendLimit, pGenes2->m_pLimbGenes[i]->m_bendLimit);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_twistLimit, pGenes2->m_pLimbGenes[i]->m_twistLimit);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_strength, pGenes2->m_pLimbGenes[i]->m_strength);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_relativeAttachmentPositionOnParent.x, pGenes2->m_pLimbGenes[i]->m_relativeAttachmentPositionOnParent.x);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_relativeAttachmentPositionOnParent.y, pGenes2->m_pLimbGenes[i]->m_relativeAttachmentPositionOnParent.y);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_relativeAttachmentPositionOnParent.z, pGenes2->m_pLimbGenes[i]->m_relativeAttachmentPositionOnParent.z);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_relativeAttachmentPositionOnThis.x, pGenes2->m_pLimbGenes[i]->m_relativeAttachmentPositionOnThis.x);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_relativeAttachmentPositionOnThis.y, pGenes2->m_pLimbGenes[i]->m_relativeAttachmentPositionOnThis.y);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_relativeAttachmentPositionOnThis.z, pGenes2->m_pLimbGenes[i]->m_relativeAttachmentPositionOnThis.z);

		// Swap entire axis for rotation to avoid illegal angles
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_relativeRotation, pGenes2->m_pLimbGenes[i]->m_relativeRotation);

		// Physical
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_density, pGenes2->m_pLimbGenes[i]->m_density);
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_friction, pGenes2->m_pLimbGenes[i]->m_friction);

		// Sensors
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_hasContactSensor, pGenes2->m_pLimbGenes[i]->m_hasContactSensor);

		// Neural structure
		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_numHiddenLayers, pGenes2->m_pLimbGenes[i]->m_numHiddenLayers);

		if(Randf() > 0.5f)
			std::swap(pGenes1->m_pLimbGenes[i]->m_numNeuronsPerHiddenLayer, pGenes2->m_pLimbGenes[i]->m_numNeuronsPerHiddenLayer);

		CrossOver_VaryingSize(pGenes1->m_pLimbGenes[i]->m_mainBrainInputNeurons, pGenes2->m_pLimbGenes[i]->m_mainBrainInputNeurons);
		CrossOver_VaryingSize(pGenes1->m_pLimbGenes[i]->m_parentLimbInputNeurons, pGenes2->m_pLimbGenes[i]->m_parentLimbInputNeurons);
		CrossOver_VaryingSize(pGenes1->m_pLimbGenes[i]->m_childLimbInputNeurons, pGenes2->m_pLimbGenes[i]->m_childLimbInputNeurons);

		CrossOver_VaryingSize(pGenes1->m_pLimbGenes[i]->m_timerSensorData, pGenes2->m_pLimbGenes[i]->m_timerSensorData);
		CrossOver_VaryingSize(pGenes1->m_pLimbGenes[i]->m_neuronData, pGenes2->m_pLimbGenes[i]->m_neuronData);
	}

	// For remaining limbs, simly make sure no values are out of bounds
	for(unsigned int i = minNumLimbs; i < maxNumLimbs; i++)
	{
		if(static_cast<signed>(i) - (*pLargerGene)[i]->m_parentIndexOffset < 0)
			(*pLargerGene)[i]->m_parentIndexOffset = i;
	}
}

void CreatureEvolver::Mutate(CreatureGenes* pGenes)
{
	// Change limb count
	if(Randf() < m_limbCountMutationRate)
	{
		int dCount = Round_Nearest(Randf(-m_maxLimbPerturbation, m_maxLimbPerturbation));

		const int newSize = pGenes->m_pLimbGenes.size() + dCount;

		if(newSize < m_minNumLimbs)
			dCount = m_minNumLimbs - pGenes->m_pLimbGenes.size();
		else if(newSize > m_maxNumLimbs)
			dCount = m_maxNumLimbs - pGenes->m_pLimbGenes.size();

		if(dCount > 0)
		{
			for(int i = 0; i < dCount; i++)
			{
				LimbGenes* pNewLimbGenes = new LimbGenes();

				InitGenes(pNewLimbGenes, pGenes->m_pLimbGenes.size() + 1);

				pGenes->m_pLimbGenes.push_back(pNewLimbGenes);
			}
		}
		else if(dCount < 0) // Remove random limbs
		{
			dCount = -dCount;

			for(int i = 0; i < dCount && static_cast<signed>(pGenes->m_pLimbGenes.size()) > m_minNumLimbs; i++)
			{
				unsigned int index = rand() % pGenes->m_pLimbGenes.size();
				delete pGenes->m_pLimbGenes[index];
				pGenes->m_pLimbGenes[index] = NULL;
				pGenes->m_pLimbGenes.erase(pGenes->m_pLimbGenes.begin() + index);
			}
		}

		assert(pGenes->m_pLimbGenes.size() >= static_cast<unsigned>(m_minNumLimbs));
	}

	for(unsigned int i = 0, size = pGenes->m_pLimbGenes.size(); i < size; i++)
	{
		LimbGenes* pLG = pGenes->m_pLimbGenes[i];

		if(i != 0 && Randf() < m_limbParentMutationRate)
		{
			pLG->m_parentIndexOffset = pLG->m_parentIndexOffset + rand() % 3 - 1;

			if(pLG->m_parentIndexOffset < 1)
				pLG->m_parentIndexOffset = 1;
			else if(pLG->m_parentIndexOffset > static_cast<signed>(i))
				pLG->m_parentIndexOffset = static_cast<signed>(i);
		}

		if(pLG->m_recursiveUnits != 0 && Randf() < m_recursiveLimbMutationRate_whenRecursive)
			pLG->m_recursiveUnits = 0;
		else if(Randf() < m_recursiveLimbMutationRate_whenNotRecursive)
			pLG->m_recursiveUnits = rand() % (m_maxInitRecursiveUnits - m_minInitRecursiveUnits) + m_minInitRecursiveUnits;
		else if(Randf() < m_recursiveLimbUnitCountChangeRate)
			pLG->m_recursiveUnits = Clamp(pLG->m_recursiveUnits + rand() % 3 - 1, 0, m_maxInitRecursiveUnits);

		if(pLG->m_symmetrical && Randf() < m_symmetricalLimbMutationRate_whenSymmetrical)
			pLG->m_symmetrical = false;
		else if(Randf() < m_symmetricalLimbMutationRate_whenNotSymmetrical)
			pLG->m_symmetrical = true;

		if(Randf() < m_numBranchesMutationRate)
			pLG->m_numBranches = Clamp(pLG->m_numBranches + rand() % 3 - 1, 0, m_maxNumBranches);

		if(Randf() < m_inheritRecursionMutationRate)
			pLG->m_inheritsRecursion = Randf() < m_initInheritRecursionChance;

		if(Randf() < m_inheritSymmetryMutationRate)
			pLG->m_inheritsSymmetry = Randf() < m_initInheritSymmetryChance;

		if(Randf() < m_dimensionalMutationRate)
		{
			pLG->m_dims.x = Clamp(pLG->m_dims.x + Randf(-m_maxLimbDimensionPerturbation, m_maxLimbDimensionPerturbation), m_minLimbDimension, m_maxLimbDimension);
			pLG->m_dims.y = Clamp(pLG->m_dims.y + Randf(-m_maxLimbDimensionPerturbation, m_maxLimbDimensionPerturbation), m_minLimbDimension, m_maxLimbDimension);
			pLG->m_dims.z = Clamp(pLG->m_dims.z + Randf(-m_maxLimbDimensionPerturbation, m_maxLimbDimensionPerturbation), m_minLimbDimension, m_maxLimbDimension);
		}

		if(Randf() < m_relativeOffsetRatioMutationRate)
			AttachmentPositionMutate_Parent(pLG->m_relativeAttachmentPositionOnParent);

		if(Randf() < m_relativeOffsetRatioMutationRate)
			AttachmentPositionMutate_This(pLG->m_relativeAttachmentPositionOnThis);

		if(Randf() < m_limbAngleMutationRate)
		{
			Quaternion newRotation(pLG->m_relativeRotation);

			newRotation.x += Randf(-m_maxLimbAnglePerturbation, m_maxLimbAnglePerturbation);
			newRotation.y += Randf(-m_maxLimbAnglePerturbation, m_maxLimbAnglePerturbation);
			newRotation.z += Randf(-m_maxLimbAnglePerturbation, m_maxLimbAnglePerturbation);
			newRotation.w += Randf(-m_maxLimbAnglePerturbation, m_maxLimbAnglePerturbation);

			newRotation.NormalizeThis();

			Vec3f normal(NormalFromPosition(pLG->m_relativeAttachmentPositionOnParent));

			// If perturbed angle is legal, set it
			if((newRotation * Vec3f(1.0f, 0.0f, 0.0f)).Dot(normal) > 0.0f)
				pLG->m_relativeRotation = newRotation;
			else
			{
				// Not legal, get one that is for sure
				float currentAngle = acosf(normal.Dot(pLG->m_relativeRotation * Vec3f(1.0f, 0.0f, 0.0f)));
				float maxAdditionalAngle = pLG->m_bendLimit - currentAngle;

				newRotation.Reset();

				// Random axis
				Vec3f axis(Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f));
				axis.NormalizeThis();

				newRotation.Rotate(Randf() * maxAdditionalAngle, axis);

				pLG->m_relativeRotation *= newRotation;
			}
		}

		if(Randf() < m_limbBendAndTwistMutationRate)
			pLG->m_bendLimit = Clamp(pLG->m_bendLimit + Randf(-m_maxLimbBendAndTwistPerturbation, m_maxLimbBendAndTwistPerturbation), m_minLimbBend, m_maxLimbBend);

		if(Randf() < m_limbBendAndTwistMutationRate)
			pLG->m_twistLimit = Clamp(pLG->m_twistLimit + Randf(-m_maxLimbBendAndTwistPerturbation, m_maxLimbBendAndTwistPerturbation), m_minLimbTwist, m_maxLimbTwist);

		if(Randf() < m_limbStrengthMutationRate)
			pLG->m_strength = Clamp(pLG->m_strength + Randf(-m_maxLimbStrengthPerturbation, m_maxLimbStrengthPerturbation), m_minLimbStrength, m_maxLimbStrength);

		if(Randf() < m_densityMutationRate)
			pLG->m_density = Clamp(pLG->m_density + Randf(-m_maxDensityPerturbation, m_maxDensityPerturbation), m_minDensity, m_maxDensity);

		if(Randf() < m_frictionMutationRate)
			pLG->m_friction = Clamp(pLG->m_friction + Randf(-m_maxFrictionPerturbation, m_maxFrictionPerturbation), m_minFriction, m_maxFriction);

		if(Randf() < m_contactSensorMutationRate)
			pLG->m_hasContactSensor = !pLG->m_hasContactSensor;

		if(Randf() < m_mainBrainInputNeuronsMutationRate)
		{
			int choice = rand() % 2;

			if(choice == 1)
				pLG->m_mainBrainInputNeurons.push_back(rand() % (m_maxNumMainBrainInputNeurons + 1));
			else if(!pLG->m_mainBrainInputNeurons.empty())
				pLG->m_mainBrainInputNeurons.erase(pLG->m_mainBrainInputNeurons.begin() + rand() % pLG->m_mainBrainInputNeurons.size());
		}

		if(Randf() < m_parentLimbInputNeuronsMutationRate)
		{
			int choice = rand() % 2;

			if(choice == 1)
				pLG->m_parentLimbInputNeurons.push_back(rand() % (m_maxNumParentLimbInputNeurons + 1));
			else if(!pLG->m_parentLimbInputNeurons.empty())
				pLG->m_parentLimbInputNeurons.erase(pLG->m_parentLimbInputNeurons.begin() + rand() % pLG->m_parentLimbInputNeurons.size());
		}

		if(Randf() < m_childLimbInputNeuronsMutationRate)
		{
			int choice = rand() % 2;

			if(choice == 1)
			{
				LimbGenes::ChildAndOutputIndex coai;
				coai.m_childIndex = rand() % m_maxNumBranches;
				coai.m_outputIndex = rand() % m_maxNumChildLimbInputNeurons;

				pLG->m_childLimbInputNeurons.push_back(coai);
			}
			else if(!pLG->m_childLimbInputNeurons.empty())
				pLG->m_childLimbInputNeurons.erase(pLG->m_childLimbInputNeurons.begin() + rand() % pLG->m_childLimbInputNeurons.size());
		}

		// Mutate weights before changing structure randomly
		for(unsigned int j = 0, numNeurons = pLG->m_neuronData.size(); j < numNeurons; j++)
		{
			//if(Randf() < m_neuralWeightMutationRate)
			//	pLG->m_neuronData[j].m_threshold = Clamp(pLG->m_neuronData[j].m_threshold + Randf(-m_neuralWeightPerturbation, m_neuralWeightPerturbation), -1.0f, 1.0f);

			for(unsigned int k = 0, numWeights = pLG->m_neuronData[j].m_weights.size(); k < numWeights; k++)
				if(Randf() < m_neuralWeightMutationRate)
					pLG->m_neuronData[j].m_weights[k] = Clamp(pLG->m_neuronData[j].m_weights[k] + Randf(-m_neuralWeightPerturbation, m_neuralWeightPerturbation), m_minPossibleNeuronWeight, m_maxPossibleNeuronWeight);
		}

		// Mutate timers before changing structure randomly
		for(unsigned int j = 0, numTimers = pLG->m_timerSensorData.size(); j < numTimers; j++)
		{
			if(Randf() < m_timerRateMutationRate)
				pLG->m_timerSensorData[j].m_rate = Clamp(pLG->m_timerSensorData[j].m_rate + Randf(-m_maxTimerRatePerturbation, m_maxTimerRatePerturbation), m_minTimerRate, m_maxTimerRate);

			if(Randf() < m_timerTimeMutationRate)
				pLG->m_timerSensorData[j].m_initTime = Clamp(pLG->m_timerSensorData[j].m_initTime + Randf(-m_maxTimerTimePerturbation, m_maxTimerTimePerturbation), 0.0f, 1.0f);
		}

		// Mutate neuron structure
		if(Randf() < m_neuralStructureMutationRate_numLayers)
			pLG->m_numHiddenLayers = Clamp(pLG->m_numHiddenLayers + rand() % 3 - 1, m_minHiddenLayers, m_maxHiddenLayers);

		if(Randf() < m_neuralStructureMutationRate_numNeuronsPerHiddenLayer)
		{
			int pert = static_cast<int>(Randf(-m_maxNeuronsPerHiddenLayerPerturbation, m_maxNeuronsPerHiddenLayerPerturbation));
			int nphl = Clamp(static_cast<signed>(pLG->m_numNeuronsPerHiddenLayer) + pert, static_cast<signed>(m_minNeuronsPerHiddenLayer), static_cast<signed>(m_maxNeuronsPerHiddenLayer));

			pLG->m_numNeuronsPerHiddenLayer = static_cast<unsigned>(nphl);
		}

		// Mutate number of timers
		if(Randf() < m_timerCountMutationRate)
			pLG->m_numTimerSensors = Clamp(pLG->m_numTimerSensors + rand() % 3 - 1, m_minTimerCount, m_maxTimerCount);
	}
}

void CreatureEvolver::AttachmentPositionInit_Parent(Vec3f &position)
{
	position.x = Randf(0.0f, 1.0f); // x position always positive, so doesn't loop back on self too badly
	position.y = Randf(-1.0f, 1.0f);
	position.z = Randf(-1.0f, 1.0f);
		
	// If none are at limits, set one to limit
	if(position.x != 1.0f &&
		position.y != 1.0f && position.y != -1.0f &&
		position.z != 1.0f && position.z != -1.0f)
	{
		switch(rand() % 6)
		{
		case 0: // No negative x, only positive (or else loops back on self)
		case 1:
			position.x = 1.0f;
			break;
		case 2:
			position.y = 1.0f;
			break;
		case 3:
			position.y = -1.0f;
			break;
		case 4:
			position.z = 1.0f;
			break;
		case 5:
			position.z = -1.0f;
			break;
		}
	}
}

void CreatureEvolver::AttachmentPositionInit_This(Vec3f &position)
{
	position.x = -1.0f;
	position.y = Randf(-1.0f, 1.0f);
	position.z = Randf(-1.0f, 1.0f);
}

void CreatureEvolver::AttachmentPositionMutate_Parent(Vec3f &position)
{
	position.x = Clamp(position.x + Randf(-m_maxRelativeOffsetRatioPerturbation, m_maxRelativeOffsetRatioPerturbation), 0.0f, 1.0f); // x position always positive, so doesn't loop back on self too badly
	position.y = Clamp(position.y + Randf(-m_maxRelativeOffsetRatioPerturbation, m_maxRelativeOffsetRatioPerturbation), -1.0f, 1.0f);
	position.z = Clamp(position.z + Randf(-m_maxRelativeOffsetRatioPerturbation, m_maxRelativeOffsetRatioPerturbation), -1.0f, 1.0f);

	// If none are at limits, set one to limit
	if(position.x != 1.0f &&
		position.y != 1.0f && position.y != -1.0f &&
		position.z != 1.0f && position.z != -1.0f)
	{
		switch(rand() % 6)
		{
		case 0: // No negative x, only positive (or else loops back on self)
		case 1:
			position.x = 1.0f;
			break;
		case 2:
			position.y = 1.0f;
			break;
		case 3:
			position.y = -1.0f;
			break;
		case 4:
			position.z = 1.0f;
			break;
		case 5:
			position.z = -1.0f;
			break;
		}
	}
}

void CreatureEvolver::AttachmentPositionMutate_This(Vec3f &position)
{
	position.x = -1.0f; // Doesn't change
	position.y = Clamp(position.y + Randf(-m_maxRelativeOffsetRatioPerturbation, m_maxRelativeOffsetRatioPerturbation), -1.0f, 1.0f);
	position.z = Clamp(position.z + Randf(-m_maxRelativeOffsetRatioPerturbation, m_maxRelativeOffsetRatioPerturbation), -1.0f, 1.0f);
}

Quaternion CreatureEvolver::NormalQuaternionFromPosition(const Vec3f &position)
{
	if(position.x == 1.0f)
	{
		Quaternion q(0.0f, 1.0f, 0.0f, 0.0f);
		return q;
	}
	else if(position.x == -1.0f)
	{
		Quaternion q(0.0f, 1.0f, 0.0f, 0.0f);
		q.Rotate(pif, Vec3f(0.0f, 1.0f, 0.0f));
		return q;
	}
	else if(position.y == 1.0f)
	{
		Quaternion q(0.0f, 1.0f, 0.0f, 0.0f);
		q.Rotate(pif_over_2, Vec3f(0.0f, 0.0f, 1.0f));
		return q;
	}
	else if(position.y == -1.0f)
	{
		Quaternion q(0.0f, 1.0f, 0.0f, 0.0f);
		q.Rotate(-pif_over_2, Vec3f(0.0f, 0.0f, 1.0f));
		return q;
	}
	else if(position.z == 1.0f)
	{
		Quaternion q(0.0f, 1.0f, 0.0f, 0.0f);
		q.Rotate(pif_over_2, Vec3f(0.0f, 1.0f, 0.0f));
		return q;
	}
	else if(position.z == -1.0f)
	{
		Quaternion q(0.0f, 1.0f, 0.0f, 0.0f);
		q.Rotate(-pif_over_2, Vec3f(0.0f, 1.0f, 0.0f));
		return q;
	}

	std::cerr << "Could not find normal-quaternion!" << std::endl;

	return Quaternion();
}

Vec3f CreatureEvolver::NormalFromPosition(const Vec3f &position)
{
	// If none are at limits, set one to limit
	if(position.x == 1.0f)
		return Vec3f(1.0f, 0.0f, 0.0f);
	else if(position.x == -1.0f)
		return Vec3f(-1.0f, 0.0f, 0.0f);
	else if(position.y == 1.0f)
		return Vec3f(0.0f, 1.0f, 0.0f);
	else if(position.y == -1.0f)
		return Vec3f(0.0f, -1.0f, 0.0f);
	else if(position.z == 1.0f)
		return Vec3f(0.0f, 0.0f, 1.0f);
	else if(position.z == -1.0f)
		return Vec3f(0.0f, 0.0f, -1.0f);

	std::cerr << "Could not find normal!" << std::endl;

	return Vec3f(0.0f, 0.0f, 0.0f);
}

void CreatureEvolver::GetTotalFitness()
{
	m_totalFitness = 0.0f;

	for(std::list<FitnessInfo>::iterator it = m_fitnesses.begin(); it != m_fitnesses.end(); it++)
		m_totalFitness += (*it).m_fitness;
}

unsigned int CreatureEvolver::RouletteSelection()
{
	// Get the random value to overcome
	float randomCusp = Randf() * m_totalFitness - 0.005f; // Subtract small amount to avoid error due to precision issues

	float fitnessSum = 0.0f;

	for(std::list<FitnessInfo>::iterator it = m_fitnesses.begin(); it != m_fitnesses.end(); it++)
	{
		fitnessSum += (*it).m_fitness;

		if(fitnessSum >= randomCusp)
		{
			// Found the member. Remove the member from the array so it cannot be re-chosen.
			// The member's fitness will be re-added in the next generation when new fitness values are loaded after simulation.
			// In addition, update the total fitness so that the next selection will function properly. This, too, is updated when
			// new fitness values are loaded.
			int index = (*it).m_creatureIndex;

			m_totalFitness -= (*it).m_fitness;

			m_fitnesses.erase(it);

			return index;
		}
	}

	std::cerr << "Was unable to find a creature!" << std::endl;
	abort();

	return 0;
}

void CreatureEvolver::SetFitness(const std::vector<float> &fitnesses)
{
	assert(fitnesses.size() == m_populationSize);

	// Re-Create the fitness array
	m_fitnesses.clear();

	if(m_checkPositiveFitness)
	{
		float lowestValue = fitnesses[0];

		// While copying, find the lowest fitness value
		for(unsigned int f = 0; f < m_populationSize; f++)
		{
			FitnessInfo info;
			info.m_fitness = fitnesses[f];
			info.m_creatureIndex = f;

			if(fitnesses[f] < lowestValue)
				lowestValue = fitnesses[f];

			m_fitnesses.push_back(info);
		}

		// Now offset all values by subtracting this value from them, making the lowest fitness 0
		for(std::list<FitnessInfo>::iterator it = m_fitnesses.begin(); it != m_fitnesses.end(); it++)
			(*it).m_fitness -= lowestValue;
	}
	else // Normal copy
		for(unsigned int f = 0; f < m_populationSize; f++)
		{
			FitnessInfo info;
			info.m_fitness = fitnesses[f];
			info.m_creatureIndex = f;

			m_fitnesses.push_back(info);
		}
}

void CreatureEvolver::FindElites(std::vector<CreatureGenes> &genes)
{
	assert(m_populationSize - m_numElites >= 0);

	std::list<FitnessInfo> temp(m_fitnesses);

	for(unsigned int i = 0; i < m_numElites; i++)
	{
		std::list<FitnessInfo>::iterator it = temp.begin();

		FitnessInfo max = *it;
		std::list<FitnessInfo>::iterator maxIt = it;

		it++;

		for(; it != temp.end(); it++)
		{
			if((*it).m_fitness > max.m_fitness)
			{
				max = (*it);
				maxIt = it;
			}
		}

		temp.erase(maxIt);

		genes.push_back(*m_genotypePopulation[max.m_creatureIndex]);
	}

	assert(temp.size() != m_fitnesses.size());
}

void CreatureEvolver::RemoveChumps(std::vector<unsigned int> &chumpIndices)
{
	for(unsigned int i = 0; i < m_numElites && !m_fitnesses.empty(); i++)
	{
		std::list<FitnessInfo>::iterator it = m_fitnesses.begin();

		FitnessInfo min = *it;
		std::list<FitnessInfo>::iterator minIt = it;

		it++;

		for(; it != m_fitnesses.end(); it++)
		{
			if((*it).m_fitness < min.m_fitness)
			{
				min = (*it);
				minIt = it;
			}
		}

		m_fitnesses.erase(minIt);

		chumpIndices.push_back(min.m_creatureIndex);
	}
}

void CreatureEvolver::Generation()
{
	assert(m_fitnesses.size() == m_populationSize);

	// Get elites
	std::vector<CreatureGenes> eliteGenes;
	std::vector<unsigned int> chumpIndices;

	FindElites(eliteGenes);
	RemoveChumps(chumpIndices);
	GetTotalFitness();

	// Obtain their genes from the neuron data
	CreatureGenes* pGene1, * pGene2;

	//float halfNumFitnesses = static_cast<float>(m_fitnesses.size()) / 2.0f;

	while(m_fitnesses.size() >= 2)
	{
		// Choose the two members to procreate by index
		unsigned int index1 = RouletteSelection();
		unsigned int index2 = RouletteSelection();

		pGene1 = m_genotypePopulation[index1];
		pGene2 = m_genotypePopulation[index2];

		// Determine whether should crossover
		if(Randf() < m_crossoverRate) // if(Randf() * halfNumFitnesses < m_crossoverRate)
			Crossover(pGene1, pGene2);

		//halfNumFitnesses -= 1.0f;

		// Mutate both chromosomes
		Mutate(pGene1);
		Mutate(pGene2);
	}

	// Replace chumps with elites
	for(unsigned int i = 0; i < m_numElites; i++)
	{
#ifdef MUTATE_ELITES
		Mutate(&eliteGenes[i]);
#endif
		(*m_genotypePopulation[chumpIndices[i]]) = eliteGenes[i];
	}
}