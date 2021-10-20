#include "Model.h"

void XM_CALLCONV addTriangleToData(FXMVECTOR a, FXMVECTOR b, FXMVECTOR c, GXMVECTOR color, std::vector<VertexColor>& points)
{
	VertexColor p;
	XMStoreFloat4(&p.color, color);

	XMStoreFloat3(&p.position, a);
	points.push_back(p);

	XMStoreFloat3(&p.position, b);
	points.push_back(p);

	XMStoreFloat3(&p.position, c);
	points.push_back(p);
}

void XM_CALLCONV addTetraToData(FXMVECTOR a, FXMVECTOR b, FXMVECTOR c, GXMVECTOR d, std::vector<VertexColor>& points)
{
	addTriangleToData(a, b, c, Colors::Black, points);
	addTriangleToData(a, c, d, Colors::Green, points);
	addTriangleToData(b, c, d, Colors::Blue, points);
	addTriangleToData(a, b, d, Colors::Red, points);
}

void XM_CALLCONV divideTetra(FXMVECTOR a, FXMVECTOR b, FXMVECTOR c, GXMVECTOR d, int count, std::vector<VertexColor>& points)
{
	if (count == 0)
	{
		addTetraToData(a, b, c, d, points);
	}
	else
	{
		XMVECTOR ab = XMVectorLerp(a, b, 0.5f);
		XMVECTOR bc = XMVectorLerp(b, c, 0.5f);
		XMVECTOR ac = XMVectorLerp(a, c, 0.5f);
		XMVECTOR cd = XMVectorLerp(c, d, 0.5f);
		XMVECTOR ad = XMVectorLerp(a, d, 0.5f);
		XMVECTOR bd = XMVectorLerp(b, d, 0.5f);

		--count;

		divideTetra(a, ab, ac, ad, count, points);
		divideTetra(ab, b, bc, bd, count, points);
		divideTetra(ac, bc, c, cd, count, points);
		divideTetra(ad, bd, cd, d, count, points);
	}
}

void generateSierpinskiGasketPoints(std::vector<VertexColor>& points, int count)
{
	std::vector<VertexColor> mainTriangleVertices
	{
		{XMFLOAT3{-0.5f, -0.5f, 0.8f}, XMFLOAT4()},
		{XMFLOAT3{0.0f, 0.5f, 0.8f}, XMFLOAT4()},
		{XMFLOAT3{0.5f, -0.5f, 0.8f}, XMFLOAT4()},
		{XMFLOAT3{0.0f, 0.0f, 0.5f}, XMFLOAT4()}
	};


	XMVECTOR a = XMLoadFloat3(&mainTriangleVertices[0].position);
	XMVECTOR b = XMLoadFloat3(&mainTriangleVertices[1].position);
	XMVECTOR c = XMLoadFloat3(&mainTriangleVertices[2].position);
	XMVECTOR d = XMLoadFloat3(&mainTriangleVertices[3].position);

	divideTetra(a, b, c, d, count, points);
}

