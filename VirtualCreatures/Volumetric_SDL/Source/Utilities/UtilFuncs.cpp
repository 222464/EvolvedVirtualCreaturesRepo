#include <Utilities/UtilFuncs.h>

#include <random>

#include <assert.h>

float DegToRad(float deg)
{
	return deg * degToRadMultiplier;
}

float RadToDeg(float rad)
{
	return rad / degToRadMultiplier;
}

float absf(float val)
{
	return (val < 0) ? (-val) : (val);
}

unsigned int NextHigherPowerOf2(unsigned int val)
{
    val--;

    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;

	val++;

    return val;
}

unsigned int Log2(unsigned int val)
{
	unsigned int result = 0;

	while(val >>= 1)
		result++;

	return result;
}

int Round_Nearest(float val)
{
	return static_cast<int>(val + 0.5f);
}

Vec3f RotationToVector(float xRotRads, float yRotRads)
{
	Vec3f dir;
	float cosY = cosf(yRotRads);

	dir.x = sinf(xRotRads) * cosY;
	dir.y = -sinf(yRotRads);
	dir.z = cosf(xRotRads) * cosY;

	return dir;
}

float Lerp(float first, float second, float interpolationCoefficient)
{
	return first + (second - first) * interpolationCoefficient;
}

void RemoveOuterCharacters(std::string &str)
{
	assert(str.length() >= 2);

	// Always on front and back
	str.erase(str.begin());
	str.pop_back();
}

float Randf()
{
	return rand() / static_cast<float>(RAND_MAX);
}

float Randf(float min, float max)
{
	return min + Randf() * (max - min);
}

std::string GetRootName(const std::string &name)
{
	// Get root name
	unsigned int i;

	for(i = name.size() - 1; i > 0; i--)
	{
		if(name[i] == '/' || name[i] == '\\')
			break;
	}

	return name.substr(0, i + 1);
}