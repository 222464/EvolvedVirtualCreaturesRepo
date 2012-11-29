#pragma once

class Point2i
{
public:
	int x, y;

	Point2i(int X, int Y);
	Point2i();
	
	bool operator==(const Point2i &other) const;
	bool operator!=(const Point2i &other) const;
};