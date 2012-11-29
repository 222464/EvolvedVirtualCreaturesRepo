#include <Constructs/Vec2f.h>

Vec2f::Vec2f() 
{
}

Vec2f::Vec2f(float X, float Y)
	: x(X), y(Y)
{
}

Vec2f Vec2f::operator*(float scale) const 
{
	return Vec2f(x * scale, y * scale);
}

Vec2f Vec2f::operator/(float scale) const 
{
	return Vec2f(x / scale, y / scale);
}

Vec2f Vec2f::operator+(const Vec2f &other) const 
{
	return Vec2f(x + other.x, y + other.y);
}

Vec2f Vec2f::operator-(const Vec2f &other) const 
{
	return Vec2f(x - other.x, y - other.y);
}

Vec2f Vec2f::operator*(const Vec2f &other) const 
{
	return Vec2f(x * other.x, y * other.y);
}

Vec2f Vec2f::operator/(const Vec2f &other) const 
{
	return Vec2f(x / other.x, y / other.y);
}

Vec2f Vec2f::operator-() const
{
	return Vec2f(-x, -y);
}

const Vec2f &Vec2f::operator*=(float scale) 
{
	x *= scale;
	y *= scale;

	return *this;
}

const Vec2f &Vec2f::operator/=(float scale) 
{
	x /= scale;
	y /= scale;

	return *this;
}

const Vec2f &Vec2f::operator+=(const Vec2f &other) 
{
	x += other.x;
	y += other.y;

	return *this;
}

const Vec2f &Vec2f::operator-=(const Vec2f &other) 
{
	x -= other.x;
	y -= other.y;

	return *this;
}

const Vec2f &Vec2f::operator*=(const Vec2f &other) 
{
	x *= other.x;
	y *= other.y;

	return *this;
}

const Vec2f &Vec2f::operator/=(const Vec2f &other) 
{
	x /= other.x;
	y /= other.y;

	return *this;
}

bool Vec2f::operator==(const Vec2f &other) const
{
	return x == other.x && y == other.y;
}

bool Vec2f::operator!=(const Vec2f &other) const
{
	return x != other.x || y != other.y;
}

float Vec2f::Magnitude() const 
{
	return sqrtf(x * x + y * y);
}

float Vec2f::MagnitudeSquared() const 
{
	return x * x + y * y;
}

void Vec2f::NormalizeThis()
{
	float m = sqrtf(x * x + y * y);

	x /= m;
	y /= m;
}

Vec2f Vec2f::Normalize() const 
{
	float m = sqrtf(x * x + y * y);
	return Vec2f(x / m, y / m);
}

float Vec2f::Dot(const Vec2f &other) const 
{
	return x * other.x + y * other.y;
}

Vec2f Vec2f::Project(const Vec2f &other) const
{
	return (Dot(other) / other.MagnitudeSquared()) * other;
}

Vec2f operator*(float scale, const Vec2f &v) 
{
	return v * scale;
}

std::ostream &operator<<(std::ostream &output, const Vec2f &v)
{
	std::cout << '(' << v.x << ", " << v.y << ')';
	return output;
}