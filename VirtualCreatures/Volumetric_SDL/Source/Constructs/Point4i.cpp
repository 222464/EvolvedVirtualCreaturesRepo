#include <Constructs/Point4i.h>

Point4i::Point4i()
{
}

Point4i::Point4i(int X, int Y, int Z, int W)
	: x(X), y(Y), z(Z), w(W)
{
}

bool Point4i::operator==(const Point4i &other) const
{
	if(x == other.x && y == other.y && z == other.z && w == other.w)
		return true;

	return false;
}

bool Point4i::operator!=(const Point4i &other) const
{
	if(x != other.x || y != other.y || z != other.z || w != other.w)
		return true;

	return false;
}