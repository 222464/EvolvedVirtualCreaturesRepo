#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/Neuron.h>

#include <Utilities/UtilFuncs.h>

#include <iostream>

float Neuron::s_initPotential = -65.0f;
float Neuron::s_threshold = 30.0f;
float Neuron::s_traceDecay = 0.1f;

float Neuron::s_minRandomWeight = -5.0f;
float Neuron::s_maxRandomWeight = 20.0f;

Neuron::Neuron()
	: m_a(0.02f), m_b(0.2f), m_c(-65.0f), m_d(8.0f),
	m_potential(Neuron::s_threshold), m_recovery(m_b * m_potential),
	m_numInputs(0), m_inputs(NULL),
	m_timeSinceLastFire(s_maxTimeSinceLastFire)
{
}

Neuron::Neuron(float a, float b, float c, float d)
	: m_a(a), m_b(b), m_c(c), m_d(d),
	m_potential(Neuron::s_threshold), m_recovery(m_b * m_potential),
	m_numInputs(0), m_inputs(NULL),
	m_timeSinceLastFire(0.0f)
{
}

Neuron::Neuron(float a, float b, float c, float d, unsigned int numInputs, Neuron::NeuronInputAndWeight* inputs) // Takes over the array
	: m_a(a), m_b(b), m_c(c), m_d(d),
	m_potential(Neuron::s_threshold), m_recovery(m_b * m_potential),
	m_numInputs(numInputs), m_inputs(inputs),
	m_timeSinceLastFire(s_maxTimeSinceLastFire)
{
}

Neuron::Neuron(const Neuron &other)
	: m_potential(other.m_potential), m_recovery(other.m_recovery),
	m_numInputs(other.m_numInputs), m_inputs(NULL),
	m_a(other.m_a), m_b(other.m_b), m_c(other.m_c), m_d(other.m_d),
	 m_timeSinceLastFire(other.m_timeSinceLastFire)
{
	if(other.m_inputs != NULL)
	{
		m_inputs = new Neuron::NeuronInputAndWeight[m_numInputs];

		for(unsigned int i = 0; i < m_numInputs; i++)
			m_inputs[i] = other.m_inputs[i];
	}
}

Neuron::~Neuron()
{
	if(m_inputs != NULL)
		delete[] m_inputs;
}

Neuron &Neuron::operator=(const Neuron &other)
{
	// Self-assignment
	if(this == &other)
		return *this;

	if(m_inputs != NULL)
		delete[] m_inputs;

	m_potential = other.m_potential;
	m_recovery = other.m_recovery;
	m_numInputs = other.m_numInputs;
	m_a = other.m_a;
	m_b = other.m_b;
	m_c = other.m_c;
	m_d = other.m_d;

	if(other.m_inputs == NULL)
		m_inputs = NULL;
	else
	{
		m_inputs = new Neuron::NeuronInputAndWeight[m_numInputs];

		for(unsigned int i = 0; i < m_numInputs; i++)
			m_inputs[i] = other.m_inputs[i];
	}

	return *this;
}

void Neuron::SetInputs(unsigned int numInputs)
{
	if(m_inputs != NULL)
		delete[] m_inputs;

	if(numInputs == 0)
		m_inputs = NULL;
	else
	{
		m_numInputs = numInputs;
		m_inputs = new NeuronInputAndWeight[m_numInputs];

		// Init randomly
		for(unsigned int i = 0; i < m_numInputs; i++)
			m_inputs[i].m_weight = Randf(s_minRandomWeight, s_maxRandomWeight);
	}
}

void Neuron::SetInputs(unsigned int numInputs, Neuron::NeuronInputAndWeight* inputs) // Takes over the array
{
	if(m_inputs != NULL)
		delete[] m_inputs;

	if(numInputs == 0)
	{
		m_inputs = NULL;

		if(inputs != NULL)
			delete[] inputs;
	}
	else
	{
		m_numInputs = numInputs;
		m_inputs = inputs;
	}
}

void Neuron::Update()
{
	// Adjust weights based on reward and then add up all incoming signals
	float current = 0.0f;
	
	for(unsigned int i = 0; i < m_numInputs; i++)
		current += m_inputs[i].m_pInput->m_output * m_inputs[i].m_weight;
	
#ifdef NEURON_MULTIPLE_UPDATES
	for(unsigned int i = 0; i < NEURON_MULTIPLE_UPDATES; i++)
		m_potential += (0.04f * m_potential * m_potential + 5.0f * m_potential + 140.0f - m_recovery + current) / NEURON_MULTIPLE_UPDATES;
#else
	m_potential += 0.04f * m_potential * m_potential + 5.0f * m_potential + 140.0f - m_recovery + current;
#endif

	m_recovery += m_a * (m_b * m_potential - m_recovery);

	if(m_potential > Neuron::s_threshold)
	{
		m_potential = m_c;
		m_recovery = m_d;

		m_output = 1.0f;

		m_timeSinceLastFire = 0.0f;
	}
	else
	{
		m_output = 0.0f;

		if(m_timeSinceLastFire < s_maxTimeSinceLastFire)
			m_timeSinceLastFire += s_timeIncrement;
	}
}

void Neuron::Update_Reinforce(float dopamine)
{
	float current = 0.0f;
	
	for(unsigned int i = 0; i < m_numInputs; i++)
		current += m_inputs[i].m_pInput->m_output * m_inputs[i].m_weight;
	
#ifdef NEURON_MULTIPLE_UPDATES
	for(unsigned int i = 0; i < NEURON_MULTIPLE_UPDATES; i++)
		m_potential += (0.04f * m_potential * m_potential + 5.0f * m_potential + 140.0f - m_recovery + current) / NEURON_MULTIPLE_UPDATES;
#else
	m_potential += 0.04f * m_potential * m_potential + 5.0f * m_potential + 140.0f - m_recovery + current;
#endif

	m_recovery += m_a * (m_b * m_potential - m_recovery);

	if(m_potential > Neuron::s_threshold)
	{
		m_potential = m_c;
		m_recovery = m_d;

		m_output = 1.0f;

		m_timeSinceLastFire = 0.0f;
	}
	else
	{
		m_output = 0.0f;

		if(m_timeSinceLastFire < s_maxTimeSinceLastFire)
			m_timeSinceLastFire += s_timeIncrement;
	}

	// Reinforcement Learning
	for(unsigned int i = 0; i < m_numInputs; i++)
	{
		float tau = m_timeSinceLastFire - m_inputs[i].m_pInput->m_timeSinceLastFire;

		m_inputs[i].m_trace += -m_inputs[i].m_trace * s_traceDecay + STDP_Func(tau) * DiractDelta(tau);

		m_inputs[i].m_weight += m_inputs[i].m_trace * dopamine;
	}
}

std::ostream &operator<<(std::ostream &os, Neuron &neuron)
{
	for(unsigned int i = 0, size = neuron.GetNumInputs(); i < size; i++)
		std::cout << neuron[i].m_weight << " ";

	return os;
}