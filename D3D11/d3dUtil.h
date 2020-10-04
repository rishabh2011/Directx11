#pragma once
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "dxgi.lib")

#include <windows.h>
#include <windowsx.h>
#include <comdef.h>
#include <wrl.h>
#include <d3d11.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h> 
#include <DirectXColors.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>

using namespace DirectX;
using namespace DirectX::PackedVector;
using Microsoft::WRL::ComPtr;

#define ReleaseCOM(x) { if(x){ (x)->Release(); (x) = nullptr; } }

inline std::wstring ansiToWString(const std::string& inputString)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, inputString.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class d3dException
{
public:
	d3dException() = default;
	d3dException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int line);

	std::wstring toString() const;
	
private:
	HRESULT errorCode = S_OK;
	std::wstring functionName;
	std::wstring fileName;
	int line;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)								        \
{														        \
	HRESULT hr = (x);									        \
	if(FAILED(hr))										        \
	{													        \
		std::wstring wFileName = ansiToWString(__FILE__);		\
		throw d3dException{ hr, L#x, wFileName, __LINE__ };	    \
	}													        \
}
#endif

//---------------------------------------------------------------------------
static ComPtr<ID3D10Blob> loadShaderCodeFromFile(const std::string& fileName)
{
	std::ifstream file{ fileName, std::ios::in | std::ios::binary };
	file.seekg(0, std::ios::end);
	int sizeInBytes{ static_cast<int>(file.tellg()) };
	file.seekg(0, std::ios::beg);

	ComPtr<ID3D10Blob> compiledCode;
	ThrowIfFailed(D3DCreateBlob(sizeInBytes, compiledCode.GetAddressOf()));
	file.read(reinterpret_cast<char*>(compiledCode->GetBufferPointer()), sizeInBytes);
	file.close();

	return compiledCode;
}

//-------------------------------------//
//--------Texture loading--------------//
//-------------------------------------//
enum class texType : unsigned int { WIC, DDS, HDR, TGA };

// WIC -> .BMP, .PNG, .GIF, .TIFF, .JPEG
// DDS 
// HDR
// TGA
// Pass in correct file type enum
// Returns a shader resource view to the created texture resource
//----------------------------------------------------------------------------------------------------------
static void createShaderResourceViewFromImageFile(const std::wstring& fileName, ComPtr<ID3D11Device> device,
	ComPtr<ID3D11DeviceContext> deviceContext, texType type, ComPtr<ID3D11ShaderResourceView> *texView)
{
	switch (type)
	{
	case texType::WIC:
		ThrowIfFailed(CreateWICTextureFromFile(device.Get(), deviceContext.Get(), fileName.c_str(), nullptr, (*texView).GetAddressOf()));
		break;

	case texType::DDS:
		ThrowIfFailed(CreateDDSTextureFromFile(device.Get(), fileName.c_str(), nullptr, (*texView).GetAddressOf()));
		break;
	}	
}

//--------------------------------------------
static XMVECTORF32 RGBAToBGRA(FXMVECTOR color)
{
	XMFLOAT4 fColor;
	XMStoreFloat4(&fColor, color);

	XMVECTORF32 adjustedColor{ fColor.z, fColor.y, fColor.x, fColor.w };
	return adjustedColor;
}

//-----------------------------------------------------------
static inline XMFLOAT3 Float4ToFloat3(const XMFLOAT4* float4)
{
	return XMFLOAT3(reinterpret_cast<const float*>(float4));
}

//--------------------------------------------------------------------
static inline XMFLOAT4 Float3ToFloat4(const XMFLOAT3* float3, float w)
{
	XMFLOAT4 convValue(reinterpret_cast<const float*>(float3));
	convValue.w = w;
	return convValue;
}