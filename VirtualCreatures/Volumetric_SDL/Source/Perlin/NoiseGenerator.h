#pragma once

class NoiseGenerator
{
private:
	int m_varyingInternalSeed;

public:
	int m_seed;
	int m_smoothDist;
	
	NoiseGenerator();

	float Noise(int value);
	float Noise3D(int x, int y, int z);
	float Interpolate_Cosine(float val1, float val2, float coeff_interpolate);
	float SmoothNoise3D(int x, int y, int z);
	float InterpolatedNoise3D(float x, float y, float z);
	float InterpolatedSmoothNoise3D(float x, float y, float z);
	float PerlinNoise3D(float x, float y, float z, int octaves, float frequencyBase, float persistence);
	float SmoothedPerlinNoise3D(float x, float y, float z, int octaves, float frequencyBase, float persistence);
};

