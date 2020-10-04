#pragma once
#include "D3DUtil.h"
#include "GameTimer.h"

class d3dApp
{
protected:

	//---------------APP VARS---------------//
	HINSTANCE appInstance{ 0 };
	HWND appWindow{ 0 };
	const wchar_t* windowName{ L"D3D11" };
	//App states
	bool appPaused{ false };
	bool appMinimized{ false };
	bool appMaximized{ false };
	bool appResizing{ false };

	//--------------Device Vars----------------------//
	UINT createDeviceFlags{ 0 };
	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<ID3D11DeviceContext> immediateContext;

	//MSAA vars
	UINT msaaQualityLevels{ 0 };
	bool msaaEnabled{ true };

	//App Resolution
	int appWidth{ 800 };
	int appHeight{ 600 };

	//Swap Chain
	ComPtr<IDXGISwapChain> swapChain;
	ComPtr<ID3D11RenderTargetView> renderTargetView;

	//Depth stencil buffer
	ComPtr<ID3D11Texture2D> depthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> depthStencilView;

	//Viewport
	D3D11_VIEWPORT mainViewPort{ 0 };

	//Timer vars
	GameTimer gameTimer;

	//Camera variables
	XMFLOAT3 camLookAt;
	XMFLOAT4 camPos;
	float yaw{ 0 };
	float pitch{ 0 };
	float lastX{ 0 };
	float lastY{ 0 };
	float camSpeed{ 10.0f };
	float camSens{ 0.1f };

public:

	explicit d3dApp(HINSTANCE appInstance);
	int run();
	void displayAdapterProperties(ComPtr<IDXGIFactory> factory);
	virtual ~d3dApp();
	
	virtual bool init();
	virtual void onResize();
	virtual void updateScene(float deltaTime) = 0;
	virtual void drawScene() = 0;
	virtual LRESULT msgHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void onMouseButtonDown(WPARAM wParam, int x, int y);
	virtual void onMouseButtonUp(WPARAM wParam, int x, int y) {}
	virtual void onMouseMove(WPARAM wParam, int x, int y);

	XMFLOAT4 getCameraPos();
	XMFLOAT3 getCameraTarget();

protected:
	void calculateFrameStats();
	bool initWindowApp();
	bool initD3D();
	float aspectRatio();
	void moveCamera(WPARAM wParam);
};