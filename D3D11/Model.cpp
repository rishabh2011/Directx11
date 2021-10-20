#include "Model.h"

void XM_CALLCONV addTriangleToData(FXMVECTOR a, FXMVECTOR b, FXMVECTOR c, std::vector<Vertex>& points)
{
	Vertex p;

	XMStoreFloat3(&p.position, a);
	points.push_back(p);

	XMStoreFloat3(&p.position, b);
	points.push_back(p);

	XMStoreFloat3(&p.position, c);
	points.push_back(p);
}

void XM_CALLCONV divideTriangle(FXMVECTOR a, FXMVECTOR b, FXMVECTOR c, int count, std::vector<Vertex>& points)
{
	if (count == 0)
	{
		addTriangleToData(a, b, c, points);
	}
	else
	{
		XMVECTOR ab = XMVectorLerp(a, b, 0.5f);
		XMVECTOR bc = XMVectorLerp(b, c, 0.5f);
		XMVECTOR ac = XMVectorLerp(a, c, 0.5f);

		--count;

		divideTriangle(a, ab, ac, count, points);
		divideTriangle(ab, b, bc, count, points);
		divideTriangle(ac, bc, c, count, points);
	}
}

void generateSierpinskiGasketPoints(std::vector<Vertex>& points, int count)
{
	std::vector<Vertex> mainTriangleVertices
	{
		{XMFLOAT3{-0.5f, -0.5f, 0.8f}},
		{XMFLOAT3{0.0f, 0.5f, 0.8f}},
		{XMFLOAT3{0.5f, -0.5f, 0.8f}}
	};


	XMVECTOR a = XMLoadFloat3(&mainTriangleVertices[0].position);
	XMVECTOR b = XMLoadFloat3(&mainTriangleVertices[1].position);
	XMVECTOR c = XMLoadFloat3(&mainTriangleVertices[2].position);

	divideTriangle(a, b, c, count, points);
}

