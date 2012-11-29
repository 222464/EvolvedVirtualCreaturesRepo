#pragma once

class Point4i
{
public:
	int x, y, z, w;

	Point4i(int X, int Y, int Z, int W);
	Point4i();
	
	bool operator==(const Point4i &other) const;
	bool operator!=(const Point4i &other) const;
};