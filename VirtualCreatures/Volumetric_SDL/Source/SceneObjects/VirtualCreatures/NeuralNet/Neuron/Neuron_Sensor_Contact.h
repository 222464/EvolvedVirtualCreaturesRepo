#pragma once

#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/Neuron_Sensor.h>
#include <SceneObjects/Physics/SceneObject_PhysicsWorld.h>

#include <System/Uncopyable.h>

// Pulses at a consistent rate, without taking any inputs
class Neuron_Sensor_Contact :
	public Neuron_Sensor, Uncopyable
{
private:
	btPairCachingGhostObject* m_pGhostObject;

	bool Hitting();

public:
	Neuron_Sensor_Contact();
	~Neuron_Sensor_Contact();

	// Inherited from Neuron_Sensor
	void Create();
	void PhysicsUpdate();
};

