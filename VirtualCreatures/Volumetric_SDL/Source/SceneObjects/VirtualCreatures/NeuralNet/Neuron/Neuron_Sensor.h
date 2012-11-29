#pragma once

#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/NeuronInput.h>

#define NULL 0

// Interface for sensors
class Neuron_Sensor :
	public NeuronInput
{
protected:
	class Limb* m_pLimb;

public:
	virtual void Create() {} // Called after limb pointer is set
	virtual void PhysicsUpdate() = 0;

	Neuron_Sensor()
		: m_pLimb(NULL)
	{
	}

	virtual ~Neuron_Sensor() {}

	friend class Limb;
};