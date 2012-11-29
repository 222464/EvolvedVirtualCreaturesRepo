#include <SceneObjects/VirtualCreatures/NeuralNet/Neuron/Neuron_Sensor_Timer.h>

#include <SceneObjects/VirtualCreatures/Limb.h>
#include <Scene/Scene.h>

#include <assert.h>

Neuron_Sensor_Timer::Neuron_Sensor_Timer()
	: m_timer(0.0f),
	m_rate(0.05f)
{
}

void Neuron_Sensor_Timer::PhysicsUpdate()
{
	assert(m_rate > 0.0f && m_rate <= 1.0f);

	m_timer += m_rate * m_pLimb->GetPhysicsWorld()->GetScene()->m_frameTimer.GetTimeMultiplier();

	if(m_timer > 1.0f)
	{
		m_timer = 0.0f;

		if(m_output == 1.0f)
			m_output = 0.0f;
		else
			m_output = 1.0f;
	}
}