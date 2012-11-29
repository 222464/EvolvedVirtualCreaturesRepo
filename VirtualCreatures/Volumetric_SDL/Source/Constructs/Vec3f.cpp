#include <Constructs/Vec3f.h>

Vec3f::Vec3f() 
{
}

Vec3f::Vec3f(float X, float Y, float Z)
	: x(X), y(Y), z(Z)
{
}

Vec3f Vec3f::operator*(float scale) const 
{
	return Vec3f(x * scale, y * scale, z * scale);
}

Vec3f Vec3f::operator/(float scale) const 
{
	return Vec3f(x / scale, y / scale, z / scale);
}

Vec3f Vec3f::operator+(const Vec3f &other) const 
{
	return Vec3f(x + other.x, y + other.y, z + other.z);
}

Vec3f Vec3f::operator-(const Vec3f &other) const 
{
	return Vec3f(x - other.x, y - other.y, z - other.z);
}

Vec3f Vec3f::operator*(const Vec3f &other) const 
{
	return Vec3f(x * other.x, y * other.y, z * other.z);
}

Vec3f Vec3f::operator/(const Vec3f &other) const 
{
	return Vec3f(x / other.x, y / other.y, z / other.z);
}

Vec3f Vec3f::operator-() const
{
	return Vec3f(-x, -y, -z);
}

const Vec3f &Vec3f::operator*=(float scale) 
{
	x *= scale;
	y *= scale;
	z *= scale;

	return *this;
}

const Vec3f &Vec3f::operator/=(float scale) 
{
	x /= scale;
	y /= scale;
	z /= scale;

	return *this;
}

const Vec3f &Vec3f::operator+=(const Vec3f &other) 
{
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

const Vec3f &Vec3f::operator-=(const Vec3f &other) 
{
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

const Vec3f &Vec3f::operator*=(const Vec3f &other) 
{
	x *= other.x;
	y *= other.y;
	z *= other.z;

	return *this;
}

const Vec3f &Vec3f::operator/=(const Vec3f &other) 
{
	x /= other.x;
	y /= other.y;
	z /= other.z;

	return *this;
}

bool Vec3f::operator==(const Vec3f &other) const
{
	return x == other.x && y == other.y && z == other.z;
}

bool Vec3f::operator!=(const Vec3f &other) const
{
	return x != other.x || y != other.y || z != other.z;
}

float Vec3f::Magnitude() const 
{
	return sqrtf(x * x + y * y + z * z);
}

float Vec3f::MagnitudeSquared() const 
{
	return x * x + y * y + z * z;
}

void Vec3f::NormalizeThis() 
{
	float m = sqrtf(x * x + y * y + z * z);
	
	x /= m;
	y /= m;
	z /= m;
}

Vec3f Vec3f::Normalize() const 
{
	float m = sqrtf(x * x + y * y + z * z);
	return Vec3f(x / m, y / m, z / m);
}

float Vec3f::Dot(const Vec3f &other) const 
{
	return x * other.x + y * other.y + z * other.z;
}

Vec3f Vec3f::Cross(const Vec3f &other) const 
{
	return Vec3f(y * other.z - z * other.y,
				 z * other.x - x * other.z,
				 x * other.y - y * other.x);
}

Vec3f Vec3f::Project(const Vec3f &other) const
{
	return (Dot(other) / other.MagnitudeSquared()) * other;
}

Vec3f operator*(float scale, const Vec3f &v) 
{
	return v * scale;
}

std::ostream &operator<<(std::ostream &output, const Vec3f &v)
{
	std::cout << '(' << v.x << ", " << v.y << ", " << v.z << ')';
	return output;
}

Vec3f Lerp(const Vec3f &first, const Vec3f &second, float interpolationCoefficient)
{
	return first + (second - first) * interpolationCoefficient;
}