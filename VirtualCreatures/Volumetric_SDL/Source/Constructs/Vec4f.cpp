#include <Constructs/Vec4f.h>

Vec4f::Vec4f() 
{
}

Vec4f::Vec4f(float X, float Y, float Z, float W)
	: x(X), y(Y), z(Z), w(W)
{
}

Vec4f Vec4f::operator*(float scale) const 
{
	return Vec4f(x * scale, y * scale, z * scale, w * scale);
}

Vec4f Vec4f::operator/(float scale) const 
{
	return Vec4f(x / scale, y / scale, z / scale, w / scale);
}

Vec4f Vec4f::operator+(const Vec4f &other) const 
{
	return Vec4f(x + other.x, y + other.y, z + other.z, w + other.w);
}

Vec4f Vec4f::operator-(const Vec4f &other) const 
{
	return Vec4f(x - other.x, y - other.y, z - other.z, w - other.w);
}

Vec4f Vec4f::operator*(const Vec4f &other) const 
{
	return Vec4f(x * other.x, y * other.y, z * other.z, w * other.w);
}

Vec4f Vec4f::operator/(const Vec4f &other) const 
{
	return Vec4f(x / other.x, y / other.y, z / other.z, w / other.w);
}

Vec4f Vec4f::operator-() const
{
	return Vec4f(-x, -y, -z, -w);
}

const Vec4f &Vec4f::operator*=(float scale) 
{
	x *= scale;
	y *= scale;
	z *= scale;

	return *this;
}

const Vec4f &Vec4f::operator/=(float scale) 
{
	x /= scale;
	y /= scale;
	z /= scale;

	return *this;
}

const Vec4f &Vec4f::operator+=(const Vec4f &other) 
{
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

const Vec4f &Vec4f::operator-=(const Vec4f &other) 
{
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

const Vec4f &Vec4f::operator*=(const Vec4f &other) 
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	w *= other.w;

	return *this;
}

const Vec4f &Vec4f::operator/=(const Vec4f &other) 
{
	x /= other.x;
	y /= other.y;
	z /= other.z;
	w /= other.w;

	return *this;
}

bool Vec4f::operator==(const Vec4f &other) const
{
	return x == other.x && y == other.y && z == other.z;
}

bool Vec4f::operator!=(const Vec4f &other) const
{
	return x != other.x || y != other.y || z != other.z || w != other.w;
}

float Vec4f::Magnitude() const 
{
	return sqrt(x * x + y * y + z * z + w * w);
}

float Vec4f::MagnitudeSquared() const 
{
	return x * x + y * y + z * z + w * w;
}

void Vec4f::NormalizeThis() 
{
	float m = sqrt(x * x + y * y + z * z + w * w);
	
	x /= m;
	y /= m;
	z /= m;
	w /= m;
}

Vec4f Vec4f::Normalize() const 
{
	float m = sqrt(x * x + y * y + z * z + w * w);
	return Vec4f(x / m, y / m, z / m, w / m);
}

float Vec4f::Dot(const Vec4f &other) const 
{
	return x * other.x + y * other.y + z * other.z;
}

Vec4f operator*(float scale, const Vec4f &v) 
{
	return v * scale;
}

std::ostream &operator<<(std::ostream &output, const Vec4f &v)
{
	std::cout << '(' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ')';
	return output;
}