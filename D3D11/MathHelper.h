#pragma once
#include "Model.h"

template <typename T>
void calculateNormals(T vertices[], int numVertices, UINT indices[], int numTriangles)
{
	for (size_t i{ 0 }; i < numTriangles; i++)
	{
		UINT i0 = indices[i * 3 + 0];
		UINT i1 = indices[i * 3 + 1];
		UINT i2 = indices[i * 3 + 2];

		XMVECTOR vertex1 = XMLoadFloat3(&(vertices[i0].position));
		XMVECTOR vertex2 = XMLoadFloat3(&(vertices[i1].position));
		XMVECTOR vertex3 = XMLoadFloat3(&(vertices[i2].position));
		XMVECTOR normal1 = XMLoadFloat3(&(vertices[i0].normal));
		XMVECTOR normal2 = XMLoadFloat3(&(vertices[i1].normal));
		XMVECTOR normal3 = XMLoadFloat3(&(vertices[i2].normal));

		//Triangle sides
		XMVECTOR side1 = vertex2 - vertex1;
		XMVECTOR side2 = vertex3 - vertex1;

		//Triangle normal calc and added to vertex normal
		XMVECTOR normal = XMVector3Cross(side1, side2);
		normal1 += normal;
		normal2 += normal;
		normal3 += normal;

		XMStoreFloat3(&(vertices[i0].normal), normal1);
		XMStoreFloat3(&(vertices[i1].normal), normal2);
		XMStoreFloat3(&(vertices[i2].normal), normal3);
	}

	for (size_t i{ 0 }; i < numVertices; i++)
	{
		XMVECTOR normal = XMLoadFloat3(&(vertices[i].normal));
		normal = XMVector3Normalize(normal);
		XMStoreFloat3(&(vertices[i].normal), normal);
	}
}