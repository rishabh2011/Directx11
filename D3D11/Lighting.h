#pragma once
#include "d3dUtil.h"

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT4 specColor;
};

struct DirectLight
{
	DirectLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT4 specColor;
	XMFLOAT3 lightDir;
	float pad;
};

struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT4 specColor;

	//Position and Range packed into a single 4D vector
	XMFLOAT3 lightPos;
	float range;

	//Attenuation
	XMFLOAT3 att;
	float pad;
};

struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 ambientColor;
	XMFLOAT4 diffuseColor;
	XMFLOAT4 specColor;

	//Position and Range packed into a single 4D vector
	XMFLOAT3 lightPos;
	float range;

	//Attenuation
	XMFLOAT3 att;
	float pad;

	//Direction and Spotlight Power
	XMFLOAT3 lightDir;
	float spotPower;
};