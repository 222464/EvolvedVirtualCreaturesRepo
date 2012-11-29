#include <Constructs/Vec4d.h>

Vec4d::Vec4d() 
{
}

Vec4d::Vec4d(double X, double Y, double Z, double W)
	: x(X), y(Y), z(Z), w(W)
{
}

Vec4d Vec4d::operator*(double scale) const 
{
	return Vec4d(x * scale, y * scale, z * scale, w * scale);
}

Vec4d Vec4d::operator/(double scale) const 
{
	return Vec4d(x / scale, y / scale, z / scale, w / scale);
}

Vec4d Vec4d::operator+(const Vec4d &other) const 
{
	return Vec4d(x + other.x, y + other.y, z + other.z, w + other.w);
}

Vec4d Vec4d::operator-(const Vec4d &other) const 
{
	return Vec4d(x - other.x, y - other.y, z - other.z, w - other.w);
}

Vec4d Vec4d::operator*(const Vec4d &other) const 
{
	return Vec4d(x * other.x, y * other.y, z * other.z, w * other.w);
}

Vec4d Vec4d::operator/(const Vec4d &other) const 
{
	return Vec4d(x / other.x, y / other.y, z / other.z, w / other.w);
}

Vec4d Vec4d::operator-() const
{
	return Vec4d(-x, -y, -z, -w);
}

const Vec4d &Vec4d::operator*=(double scale) 
{
	x *= scale;
	y *= scale;
	z *= scale;

	return *this;
}

const Vec4d &Vec4d::operator/=(double scale) 
{
	x /= scale;
	y /= scale;
	z /= scale;

	return *this;
}

const Vec4d &Vec4d::operator+=(const Vec4d &other) 
{
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

const Vec4d &Vec4d::operator-=(const Vec4d &other) 
{
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

const Vec4d &Vec4d::operator*=(const Vec4d &other) 
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	w *= other.w;

	return *this;
}

const Vec4d &Vec4d::operator/=(const Vec4d &other) 
{
	x /= other.x;
	y /= other.y;
	z /= other.z;
	w /= other.w;

	return *this;
}

bool Vec4d::operator==(const Vec4d &other) const
{
	return x == other.x && y == other.y && z == other.z;
}

bool Vec4d::operator!=(const Vec4d &other) const
{
	return x != other.x || y != other.y || z != other.z || w != other.w;
}

double Vec4d::Magnitude() const 
{
	return sqrt(x * x + y * y + z * z + w * w);
}

double Vec4d::MagnitudeSquared() const 
{
	return x * x + y * y + z * z + w * w;
}

void Vec4d::NormalizeThis() 
{
	double m = sqrt(x * x + y * y + z * z + w * w);
	
	x /= m;
	y /= m;
	z /= m;
	w /= m;
}

Vec4d Vec4d::Normalize() const 
{
	double m = sqrt(x * x + y * y + z * z + w * w);
	return Vec4d(x / m, y / m, z / m, w / m);
}

double Vec4d::Dot(const Vec4d &other) const 
{
	return x * other.x + y * other.y + z * other.z;
}

Vec4d operator*(double scale, const Vec4d &v) 
{
	return v * scale;
}

std::ostream &operator<<(std::ostream &output, const Vec4d &v)
{
	std::cout << '(' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ')';
	return output;
}