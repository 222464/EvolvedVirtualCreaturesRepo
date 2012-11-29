#pragma once

#include <Utilities/UtilFuncs.h>

class NeuronInput
{
public:
	static float s_maxTimeSinceLastFire;
	static float s_timeIncrement;

	static float s_diract_A;
	static float s_diract_A_squared_inverse;
	static float s_diract_oneOverARootPI;
	static float s_SPDT_stretchFactor_inverse;

	static float s_fireRateValueInfluence;
	static float s_fireRateValueStretchFactor_inverse;

	inline static float NormalDistribution_1_1(float val)
	{
		// Intersects y at 1 and x at 1
		return 1.58198f * FasterExpf(-val * val) - 0.581977f;
	}

	inline static float Logistic(float val)
	{
		return logf(val);
	}

	inline static float DiractDelta(float val)
	{
		return s_diract_oneOverARootPI * FasterExpf(-val * val * s_diract_A_squared_inverse);
	}

	inline static float STDP_Func(float tDiff)
	{
		if(tDiff < 0.0f)
			return -1.5f * FasterExpf(tDiff * s_SPDT_stretchFactor_inverse);

		return FasterExpf(-tDiff * s_SPDT_stretchFactor_inverse);
	}

	inline static float GetFireRateValue(float val)
	{
		return FasterExpf(s_fireRateValueInfluence - val * s_fireRateValueStretchFactor_inverse);
	}

	float m_output;
	float m_timeSinceLastFire;

	NeuronInput();
	virtual ~NeuronInput() {}

	virtual void Update() {}
	virtual void Update_Reinforce(float dopamine) {}
};
