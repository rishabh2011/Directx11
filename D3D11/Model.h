#pragma once
#include "Lighting.h"
#include "d3dUtil.h"

struct VertexWithNormal
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
};

struct VertexWithColor
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

struct DRAW_INDEXED_DESC
{
	//Vertex Buffer
	ComPtr<ID3D11Buffer>vertexBuffer;
	UINT stride{ 0 };
	UINT offset{ 0 };

	XMFLOAT4X4 worldMatrix;
	Material material;

	//Index Buffer
	ComPtr<ID3D11Buffer>indexBuffer;
	UINT indexCount{ 0 };
	UINT startIndex{ 0 };
	UINT baseVertex{ 0 };
};