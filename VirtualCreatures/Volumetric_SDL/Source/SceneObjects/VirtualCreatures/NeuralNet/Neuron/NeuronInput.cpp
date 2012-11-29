#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/NeuronInput.h>

float NeuronInput::s_diract_A = 0.2f;
float NeuronInput::s_diract_A_squared_inverse = 1.0f / (s_diract_A * s_diract_A);
float NeuronInput::s_diract_oneOverARootPI = 1.0f / (s_diract_A * std::sqrtf(pif));
float NeuronInput::s_SPDT_stretchFactor_inverse = 1.0f;

float NeuronInput::s_maxTimeSinceLastFire = 1000.0f;
float NeuronInput::s_timeIncrement = 1.0f;

float NeuronInput::s_fireRateValueInfluence = 1.0f;
float NeuronInput::s_fireRateValueStretchFactor_inverse = 1.0f / 2.0f;

NeuronInput::NeuronInput()
	: m_output(0.0f), m_timeSinceLastFire(s_maxTimeSinceLastFire)
{
}