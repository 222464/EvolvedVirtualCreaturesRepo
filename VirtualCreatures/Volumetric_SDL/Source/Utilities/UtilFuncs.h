#pragma once

#include <stdint.h>

#ifdef DEBUG
#include <vld.h>
#endif

// Override macro, to remain cross platform. Override keyword is only available in VS. If not using VS, OVERRIDE does nothing
#ifdef _MSC_BUILD
#define OVERRIDE override
#else
#define OVERRIDE
#endif

// Keeps function call even when asserts are disabledW
#ifdef _DEBUG
#define KEEP_ASSERT(func) assert(func);
#else
#define KEEP_ASSERT(func) func;
#endif

#include <Constructs/Vec3f.h>

#define _USE_MATH_DEFINES
#include <math.h>

const float pif = static_cast<float>(M_PI);
const float pif_times_2 = pif * 2.0f;
const float pif_over_2 = pif / 2.0f;
const float pif_over_4 = pif / 4.0f;
const float degToRadMultiplier = pif / 180.0f;

template<class T>
T Wrap(T val, T size)
{
	if(val < 0)
		return val + size;

	if(val >= size)
		return val - size;

	return val;
}

template<class T>
T Clamp(T val, T min, T max)
{
	if(val > max)
		return max;
	else if(val < min)
		return min;

	return val;
}

int Round_Nearest(float val);

float DegToRad(float deg);
float RadToDeg(float rad);

float absf(float val);

unsigned int NextHigherPowerOf2(unsigned int val);

unsigned int Log2(unsigned int val);

template<class T>
T Pow2(T exponent)
{
	T val = 1;

	return val << exponent;
}

Vec3f RotationToVector(float xRotRads, float yRotRads);

float Lerp(float first, float second, float interpolationCoefficient);

void RemoveOuterCharacters(std::string &str);

// Random float in range [0, 1]
float Randf();

float Randf(float min, float max);

std::string GetRootName(const std::string &name);

// From http://code.google.com/p/fastapprox/
inline float FasterPow2f(float p)
{
	float clipp = (p < -126) ? -126.0f : p;
	union {uint32_t i; float f;} v = {static_cast<uint32_t>((1 << 23) * (clipp + 126.94269504f))};
	return v.f;
}

inline float FasterExpf(float p)
{
	return FasterPow2f(1.442695040f * p);
}