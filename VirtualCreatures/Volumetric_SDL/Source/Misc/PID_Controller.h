#pragma once

class PID_Controller
{
public:
	// Mostly just used internally
	float m_previousError;
	float m_integral;

	float m_output;

	// Past, present, and future parameters (proportional, integral, and derivative values respectively) 
	float m_kp, m_ki, m_kd;

	// Desired output value
	float m_desiredValue;

	PID_Controller();
	PID_Controller(float kp, float ki, float kd);

	void Update(float measuredValue, float dt);

	void Reset();
};

