#include "Point.h"


Point::Point()
{
	xposition = 0.0;
	yposition = 0.0;
}
Point::Point(int id, int x, int y):nodeid(id), xposition(x), yposition(y)
{
}

Point::~Point()
{
}
