#include <Constructs/Point3i.h>

Point3i::Point3i()
{
}

Point3i::Point3i(int X, int Y, int Z)
	: x(X), y(Y), z(Z)
{
}

bool Point3i::operator==(const Point3i &other) const
{
	if(x == other.x && y == other.y && z == other.z)
		return true;

	return false;
}

bool Point3i::operator!=(const Point3i &other) const
{
	if(x != other.x || y != other.y || z != other.z)
		return true;

	return false;
}