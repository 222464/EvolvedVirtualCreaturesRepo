#pragma once

class Point3i
{
public:
	int x, y, z;

	Point3i(int X, int Y, int Z);
	Point3i();
	
	bool operator==(const Point3i &other) const;
	bool operator!=(const Point3i &other) const;
};