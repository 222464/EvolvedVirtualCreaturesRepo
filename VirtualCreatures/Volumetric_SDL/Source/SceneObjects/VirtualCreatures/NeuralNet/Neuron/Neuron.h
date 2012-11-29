#pragma once

#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/NeuronInput.h>

#include <ostream>

#include <assert.h>

// Uncomment to perform multiple updates to potential for numerical stability. Number of updates is macro
//#define NEURON_MULTIPLE_UPDATES 2

class Neuron :
	public NeuronInput
{
public:
	static float s_initPotential;
	static float s_threshold;
	static float s_traceDecay;

	static float s_minRandomWeight;
	static float s_maxRandomWeight;

	struct NeuronInputAndWeight
	{
		NeuronInput* m_pInput;
		float m_weight;
		float m_trace;

		NeuronInputAndWeight()
			: m_pInput(NULL), m_weight(0.0f), m_trace(0.0f)
		{
		}
	};
private:
	unsigned int m_numInputs;

	float m_timeSinceLastFire;

	NeuronInputAndWeight* m_inputs;

	float m_trace;

public:
	float m_potential;
	float m_recovery;

	// Izhikevich model parameters, can vary between neurons
	float m_a, m_b, m_c, m_d;

	Neuron();
	Neuron(const Neuron &other);
	Neuron(float a, float b, float c, float d);
	Neuron(float a, float b, float c, float d, unsigned int numInputs, Neuron::NeuronInputAndWeight* inputs); // Takes over the array
	virtual ~Neuron();

	Neuron &operator=(const Neuron &other);

	void SetInputs(unsigned int numInputs);
	void SetInputs(unsigned int numInputs, NeuronInputAndWeight* inputs); // Takes over the array

	// Inherited from NeuronInput
	virtual void Update();
	virtual void Update_Reinforce(float dopamine);

	unsigned int GetNumInputs() const
	{
		return m_numInputs;
	}

	Neuron::NeuronInputAndWeight &operator[](unsigned int index)
	{
		assert(index < m_numInputs);
		return m_inputs[index];
	}

	float GetTimeSinceLastFire() const
	{
		return m_timeSinceLastFire;
	}
};

std::ostream &operator<<(std::ostream &os, Neuron &neuron);