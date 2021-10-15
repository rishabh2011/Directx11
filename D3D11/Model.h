#pragma once
#include "Lighting.h"
#include "d3dUtil.h"

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

class Model
{
public:

	Model(UINT stride, UINT indexCount) : stride{stride}, indexCount{indexCount}
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
	Material material;

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
	float frameChangeTimer { 1.0f / 30.0f };
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

//-----------------------------------------------------//
//-----------------------CUBE--------------------------//
//-----------------------------------------------------//
VertexNormTex cubeVertices[] =
{
	//Top Face
	{XMFLOAT3{-0.5f,  0.5f, -0.5f},	 XMFLOAT3(),  XMFLOAT2{0.0f, 1.0f}},  //Bottom Left - 0
	{XMFLOAT3{-0.5f,  0.5f,  0.5f},  XMFLOAT3(),  XMFLOAT2{0.0f, 0.0f}},  //Top Left - 1
	{XMFLOAT3{ 0.5f,  0.5f,  0.5f},  XMFLOAT3(),  XMFLOAT2{1.0f, 0.0f}},  //Top Right - 2
	{XMFLOAT3{ 0.5f,  0.5f, -0.5f},  XMFLOAT3(),  XMFLOAT2{1.0f, 1.0f}},  //Bottom Right - 3

	//Bottom Face								  
	{XMFLOAT3{-0.5f, -0.5f,  0.5f},	 XMFLOAT3(),  XMFLOAT2{0.0f, 1.0f}},  //Bottom Left - 4
	{XMFLOAT3{-0.5f, -0.5f, -0.5f},  XMFLOAT3(),  XMFLOAT2{0.0f, 0.0f}},  //Top Left - 5
	{XMFLOAT3{ 0.5f, -0.5f, -0.5f},  XMFLOAT3(),  XMFLOAT2{1.0f, 0.0f}},  //Top Right - 6
	{XMFLOAT3{ 0.5f, -0.5f,  0.5f},  XMFLOAT3(),  XMFLOAT2{1.0f, 1.0f}},  //Bottom Right - 7

	//Left Face									  
	{XMFLOAT3{-0.5f, -0.5f,  0.5f},  XMFLOAT3(),  XMFLOAT2{0.0f, 1.0f}},  //Bottom Left - 8
	{XMFLOAT3{-0.5f,  0.5f,  0.5f},  XMFLOAT3(),  XMFLOAT2{0.0f, 0.0f}},  //Top Left - 9
	{XMFLOAT3{-0.5f,  0.5f, -0.5f},	 XMFLOAT3(),  XMFLOAT2{1.0f, 0.0f}},  //Top Right - 10
	{XMFLOAT3{-0.5f, -0.5f, -0.5f},	 XMFLOAT3(),  XMFLOAT2{1.0f, 1.0f}},  //Bottom Right - 11

	//Right Face								  
	{XMFLOAT3{ 0.5f, -0.5f, -0.5f},  XMFLOAT3(),  XMFLOAT2{0.0f, 1.0f}},  //Bottom Left - 12
	{XMFLOAT3{ 0.5f,  0.5f, -0.5f},  XMFLOAT3(),  XMFLOAT2{0.0f, 0.0f}},  //Top Left - 13
	{XMFLOAT3{ 0.5f,  0.5f,  0.5f},	 XMFLOAT3(),  XMFLOAT2{1.0f, 0.0f}},  //Top Right - 14
	{XMFLOAT3{ 0.5f, -0.5f,  0.5f},	 XMFLOAT3(),  XMFLOAT2{1.0f, 1.0f}},  //Bottom Right - 15	

	//Front Face								  
	{XMFLOAT3{-0.5f, -0.5f, -0.5f},  XMFLOAT3(),  XMFLOAT2{0.0f, 1.0f}},  //Bottom Left - 16
	{XMFLOAT3{-0.5f,  0.5f, -0.5f},	 XMFLOAT3(),  XMFLOAT2{0.0f, 0.0f}},  //Top Left - 17
	{XMFLOAT3{ 0.5f,  0.5f, -0.5f},	 XMFLOAT3(),  XMFLOAT2{1.0f, 0.0f}},  //Top Right - 18
	{XMFLOAT3{ 0.5f, -0.5f, -0.5f},	 XMFLOAT3(),  XMFLOAT2{1.0f, 1.0f}},  //Bottom Right - 19

	//Back Face									  
	{XMFLOAT3{ 0.5f, -0.5f,  0.5f},  XMFLOAT3(),  XMFLOAT2{0.0f, 1.0f}},  //Bottom Left - 20
	{XMFLOAT3{ 0.5f,  0.5f,  0.5f},	 XMFLOAT3(),  XMFLOAT2{0.0f, 0.0f}},  //Top Left - 21
	{XMFLOAT3{-0.5f,  0.5f,  0.5f},	 XMFLOAT3(),  XMFLOAT2{1.0f, 0.0f}},  //Top Right - 22
	{XMFLOAT3{-0.5f, -0.5f,  0.5f},	 XMFLOAT3(),  XMFLOAT2{1.0f, 1.0f}},  //Bottom Right - 23		
};

//Cube Indices
unsigned int cubeIndices[] = {
	//Top Face
	0, 1, 2,
	0, 2, 3,

	//Bottom Face
	4, 5, 6,
	4, 6, 7,

	//Left Face
	8, 9, 10,
	8, 10, 11,

	//Right Face
	12, 13, 14,
	12, 14, 15,

	//Front Face
	16, 17, 18,
	16, 18, 19,

	//Back Face
	20, 21, 22,
	20, 22, 23
};

//-----------------------------------------------------//
//-----------------------QUAD--------------------------//
//-----------------------------------------------------//
VertexNormTex quadVertices[] =
{
	//Front Face								  
	{XMFLOAT3{-0.5f, -0.5f, -0.5f},  XMFLOAT3(),  XMFLOAT2{0.0f, 1.0f}},  //Bottom Left - 0
	{XMFLOAT3{-0.5f,  0.5f, -0.5f},	 XMFLOAT3(),  XMFLOAT2{0.0f, 0.0f}},  //Top Left - 1
	{XMFLOAT3{ 0.5f,  0.5f, -0.5f},	 XMFLOAT3(),  XMFLOAT2{1.0f, 0.0f}},  //Top Right - 2
	{XMFLOAT3{ 0.5f, -0.5f, -0.5f},	 XMFLOAT3(),  XMFLOAT2{1.0f, 1.0f}},  //Bottom Right - 3
};

//Cube Indices
unsigned int quadIndices[] = {
	//Top Face
	0, 1, 2,
	0, 2, 3,
};