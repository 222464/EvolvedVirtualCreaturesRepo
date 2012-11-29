#include <Misc/PID_Controller.h>

PID_Controller::PID_Controller()
	: m_previousError(0.0f), m_integral(0.0f),
	m_output(0.0f), m_desiredValue(0.0f),
	m_kp(1.0f), m_ki(1.0f), m_kd(1.0f)
{
}

PID_Controller::PID_Controller(float kp, float ki, float kd)
	: m_previousError(0.0f), m_integral(0.0f),
	m_output(0.0f), m_desiredValue(0.0f),
	m_kp(kp), m_ki(ki), m_kd(kd)
{
}

void PID_Controller::Update(float measuredValue, float dt)
{
	float error = m_desiredValue - measuredValue;

	m_integral += error * dt;

	float derivative = (error - m_previousError) / dt;

	m_output = m_kp * error + m_ki * m_integral + m_kd * derivative;

	m_previousError = error;
}

void PID_Controller::Reset()
{
	m_previousError = 0.0f;
	m_output = 0.0f;
	m_desiredValue = 0.0f;
	m_integral = 0.0f;
}