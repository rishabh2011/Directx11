#include "Turtle.h"
#include "MathHelper.h"

void Turtle::init(float x, float y, float angle)
{
	xPos = x;
	yPos = y;
	this->angle = angle;

	Vertex p;
	p.position = XMFLOAT3{ xPos, yPos, 0 };
	points.push_back(p);

	if (!penUp)
	{
		indices.push_back(pointCount);
	}
	++pointCount;
}

void Turtle::forward(float distance)
{
	if (penUp && indices.size() != 0)
	{
		//Remove the extra index added to facilitate LINELIST topology rendering
		//Effectively doesn't draw the line segment from previous point to this point
		indices.pop_back();
	}

	xPos += distance * std::cos(degToRadians(angle));
	yPos += distance * std::sin(degToRadians(angle));

	Vertex p;
	p.position = XMFLOAT3{ xPos, yPos, 0 };
	points.push_back(p);

	indices.push_back(pointCount);
	if (!penUp)
	{
		//Push back index one more time because we are using LINELIST Topology 
		indices.push_back(pointCount);
	}
	++pointCount;
}

void Turtle::right(float angle)
{
	this->angle -= angle;
}

void Turtle::left(float angle)
{
	this->angle += angle;
}

void Turtle::togglePen(bool value)
{
	penUp = value;
}

std::vector<Vertex> Turtle::getPoints()
{
	return points;
}

std::vector<unsigned int> Turtle::getIndices()
{
	return indices;
}