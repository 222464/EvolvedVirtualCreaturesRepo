#pragma once

#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/Neuron.h>
#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/Neuron_Sensor.h>

// Pulses at a consistent rate, without taking any inputs
class Neuron_Sensor_Timer :
	public Neuron_Sensor
{
public:
	float m_timer;
	float m_rate;

	Neuron_Sensor_Timer();

	// Inherited from Neuron_Sensor
	void PhysicsUpdate();
};

