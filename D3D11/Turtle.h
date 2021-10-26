#pragma once
#include "d3dUtil.h"
#include "Model.h"

class Turtle
{
private:
	bool penUp{ false };
	int toggleCount{ 1 };
	float xPos;
	float yPos;
	float angle;

	std::vector<Vertex> points;
	std::vector<unsigned int> indices;
	unsigned int pointCount{ 0 };

public:

	//pixel coordinates
	//angle in degrees
	void init(float x, float y, float angle);

	//pixel coordinates
	void forward(float distance);

	//angle in degrees
	void right(float angle);

	//angle in degrees
	void left(float angle);

	//true : pen up
	//false : pen down 
	void togglePen(bool value);

	std::vector<Vertex> getPoints();
	std::vector<unsigned int> getIndices();
};