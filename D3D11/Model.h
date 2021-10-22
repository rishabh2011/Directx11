#pragma once
#include "d3dUtil.h"
#include "GameTimer.h"

struct Vertex
{
	XMFLOAT3 position;
};

struct VertexNorm
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
};

struct VertexNormTex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texCoords;
};

struct VertexColor
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

//Sierpinski Gasket Tetrahedron
extern void XM_CALLCONV addTriangleToData(FXMVECTOR a, FXMVECTOR b, FXMVECTOR c, GXMVECTOR color, std::vector<VertexColor>& points);
extern void XM_CALLCONV addTetraToData(FXMVECTOR a, FXMVECTOR b, FXMVECTOR c, GXMVECTOR d, std::vector<VertexColor>& points);
extern void XM_CALLCONV divideTetra(FXMVECTOR a, FXMVECTOR b, FXMVECTOR c, GXMVECTOR d, int count, std::vector<VertexColor>& points);
extern void generateSierpinskiGasketPoints(std::vector<VertexColor>& points, int count);

class Model
{
public:

	Model(UINT stride, UINT indexCount) : stride{ stride }, indexCount{ indexCount }
	{
		XMMATRIX identity = XMMatrixIdentity();
		XMStoreFloat4x4(&worldMatrix, identity);
		XMStoreFloat4x4(&texTransformMatrix, identity);
	}

	//Vertex Buffer
	ComPtr<ID3D11Buffer>vertexBuffer;
	UINT stride{ 0 };
	UINT offset{ 0 };

	XMFLOAT4X4 worldMatrix;

	//Index Buffer
	ComPtr<ID3D11Buffer>indexBuffer;
	UINT indexCount{ 0 };
	UINT startIndex{ 0 };
	UINT baseVertex{ 0 };

	//Texture
	ComPtr<ID3D11ShaderResourceView> texViews[120];
	ComPtr<ID3D11SamplerState> sampler;
	XMFLOAT4X4 texTransformMatrix;
	float animTimer{ 0.0f };
	float frameChangeTimer{ 1.0f / 30.0f };
	int currentFrame{ 0 };
	bool useTexture{ true };
	bool clipAlpha{ false };
	float uOffset{ 0.0f };
	float vOffset{ 0.0f };

	//----------------------------------------------------------
	void performInefficientAnimation(const GameTimer& gameTimer)
	{
		if ((gameTimer.totalTime() - animTimer) > frameChangeTimer)
		{
			animTimer += frameChangeTimer;
			currentFrame++;
			if (currentFrame == 120)
			{
				currentFrame = 0;
			}
		}
	}
};
