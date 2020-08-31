#pragma once
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "dxgi.lib")

#include <windows.h>
#include <windowsx.h>
#include <comdef.h>
#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXPackedVector.h>
#include <DirectXMath.h>
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

//--------------------------------------------
static XMVECTORF32 RGBAToBGRA(FXMVECTOR color)
{
	XMFLOAT4 fColor;
	XMStoreFloat4(&fColor, color);

	XMVECTORF32 adjustedColor{ fColor.z, fColor.y, fColor.x, fColor.w };
	return adjustedColor;
}