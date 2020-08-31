#include "D3DApp.h"

d3dApp* d3dAppPtr{ nullptr };

//--------------------------------------------------------
LRESULT CALLBACK
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return d3dAppPtr->msgHandler(hWnd, msg, wParam, lParam);
}

//--------------------------------------------------------------
d3dApp::d3dApp(HINSTANCE appInstance) : appInstance{appInstance}
{
	d3dAppPtr = this;
}

//-----------------
bool d3dApp::init()
{
	if (!initWindowApp())
	{
		return false;
	}

	if (!initD3D())
	{
		return false;
	}
}

//--------------------------
bool d3dApp::initWindowApp()
{
	//Create window class
	WNDCLASS wnd;
	wnd.hInstance = appInstance;
	wnd.style = CS_HREDRAW | CS_VREDRAW;
	wnd.hIcon = LoadIcon(appInstance, IDI_APPLICATION);
	wnd.hCursor = LoadCursor(appInstance, IDC_ARROW);
	wnd.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.lpszMenuName = 0;
	wnd.lpfnWndProc = WndProc;
	wnd.lpszClassName = L"D3D11WndClass";

	//Register class with Windows OS
	if (!RegisterClass(&wnd))
	{
		MessageBox(0, L"Failed To Register Window Class", 0, 0);
		return false;
	}

	RECT r{ 0, 0, appWidth, appHeight };
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
	int width = r.right - r.left;
	int height = r.bottom - r.top;

	//Create Window
	appWindow = CreateWindow(L"D3D11WndClass",
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		0,
		0,
		appInstance,
		0);

	if (appWindow == 0)
	{
		MessageBox(0, L"Failed to Create Window", 0, 0);
		return false;
	}

	ShowWindow(appWindow, SW_SHOW);
	UpdateWindow(appWindow);

	return true;
}

//Initialize Directx11
//--------------------
bool d3dApp::initD3D()
{
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//Create D3DDevice and D3DDeviceContext
	D3D_FEATURE_LEVEL featLevel;
	HRESULT hr = D3D11CreateDevice(0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		createDeviceFlags,
		0,
		0,
		D3D11_SDK_VERSION,
		d3dDevice.GetAddressOf(),
		&featLevel,
		immediateContext.GetAddressOf()
	);

	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to create D3D11 device", 0, 0);
		return false;
	}

	if (featLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Feature Level 11 not supported", 0, 0);
		return false;
	}

	//Check multisampling 4xaa support
	ThrowIfFailed(d3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &msaaQualityLevels));
	assert(msaaQualityLevels > 0);

	//Define swap chain data
	DXGI_SWAP_CHAIN_DESC swapDesc;

	swapDesc.BufferDesc.Width = appWidth;
	swapDesc.BufferDesc.Height = appHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 144;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	if (msaaEnabled)
	{
		swapDesc.SampleDesc.Count = 4;
		swapDesc.SampleDesc.Quality = msaaQualityLevels - 1;		
	}
	else
	{
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;
	}

	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = 1;
	swapDesc.Flags = 0;
	swapDesc.OutputWindow = appWindow;
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	
	//Create Swap Chain
	ComPtr<IDXGIDevice> device;
	ThrowIfFailed(d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)device.GetAddressOf()));

	ComPtr<IDXGIAdapter> devAdapter;
	ThrowIfFailed(device->GetAdapter(devAdapter.GetAddressOf()));

	ComPtr<IDXGIFactory> factory;
	ThrowIfFailed(devAdapter->GetParent(__uuidof(IDXGIFactory), (void**)factory.GetAddressOf()));

	ThrowIfFailed(factory->CreateSwapChain(d3dDevice.Get(), &swapDesc, swapChain.GetAddressOf()));

	//Disable alt+enter for fullscreen switching
	//ThrowIfFailed(factory->MakeWindowAssociation(appWindow, DXGI_MWA_NO_WINDOW_CHANGES));

	displayAdapterProperties(factory);
	onResize();
	return true;
}

//---------------------
void d3dApp::onResize()
{
	assert(d3dDevice);
	assert(immediateContext);
	assert(swapChain);

	depthStencilView.Reset();
	depthStencilBuffer.Reset();
	renderTargetView.Reset();

	//Create Render target view for swap chain back buffer
	ThrowIfFailed(swapChain->ResizeBuffers(1, appWidth, appHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf()));
	ThrowIfFailed(d3dDevice->CreateRenderTargetView(backBuffer.Get(), 0, renderTargetView.GetAddressOf()));
	
	//Create Depth/Stencil buffer
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texDesc.Height = appHeight;
	texDesc.Width = appWidth;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;

	if (msaaEnabled)
	{
		texDesc.SampleDesc.Count = 4;
		texDesc.SampleDesc.Quality = msaaQualityLevels - 1;
	}
	else
	{
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
	}

	ThrowIfFailed(d3dDevice->CreateTexture2D(&texDesc, 0, depthStencilBuffer.GetAddressOf()));
	ThrowIfFailed(d3dDevice->CreateDepthStencilView(depthStencilBuffer.Get(), 0, depthStencilView.GetAddressOf()));

	//Bind render target view and depthStencil view to output merger state
	immediateContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

	//Set Viewport
	mainViewPort.TopLeftX = 0.0f;
	mainViewPort.TopLeftY = 0.0f;
	mainViewPort.Width = static_cast<float>(appWidth);
	mainViewPort.Height = static_cast<float>(appHeight);
	mainViewPort.MinDepth = 0.0f;
	mainViewPort.MaxDepth = 1.0f;

	immediateContext->RSSetViewports(1, &mainViewPort);
}

//-----------------------------------------------------------------
void d3dApp::displayAdapterProperties(ComPtr<IDXGIFactory> factory)
{
	//Display total adapters in the computer
	UINT i = 0;
	ComPtr<IDXGIAdapter> pAdapter;
	std::vector<ComPtr<IDXGIAdapter>> vAdapters;
	while (factory->EnumAdapters(i, pAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(pAdapter);
		++i;
	}
	OutputDebugString((L"Number of Adapters : " + std::to_wstring(vAdapters.size()) + L"\n").c_str());
	
	//Display number of outputs associated with the default adapter
	//Default adapter is the adapter the primary display is connected to and is always returned first at index 0
	UINT j = 0;
	ComPtr<IDXGIOutput> pOutput;
	std::vector<ComPtr<IDXGIOutput>> outputs;
	while (vAdapters[0]->EnumOutputs(j, pOutput.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		outputs.push_back(pOutput);
		++j;
	}
	OutputDebugString((L"Number of Outputs for default adapter : " + std::to_wstring(outputs.size()) + L"\n").c_str());

	//Display supported display modes for the various outputs
	for (size_t i = 0; i < outputs.size(); i++)
	{
		//GetDisplayModeList is designed to be used twice. (Check MSDN)
		//1. To get the num of DXGI_MODE_DESC
		//2. To get the DXGI_MODE_DESC array
		OutputDebugString((L"Output : " + std::to_wstring(i + 1) + L"\n").c_str());
		UINT num = 0;
		ThrowIfFailed(outputs[i]->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &num, 0));

		DXGI_MODE_DESC* pDescs = new DXGI_MODE_DESC[num];
		ThrowIfFailed(outputs[i]->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &num, pDescs));

		for (size_t j = 0; j < num; j++)
		{
			std::wstring output = L"WIDTH = " + std::to_wstring(pDescs[j].Width) +
				L" HEIGHT = " + std::to_wstring(pDescs[j].Height) +
				L" REFRESH = " + std::to_wstring(pDescs[j].RefreshRate.Numerator) +
				L"/" + std::to_wstring(pDescs[j].RefreshRate.Denominator) + L"\n";
			OutputDebugString(output.c_str());
		}
		delete[] pDescs;
	}
	
	//Release COM objects as they are no longer needed
	for (size_t i = 0; i < outputs.size(); i++)
	{
		ReleaseCOM(outputs[i]);
	}
	outputs.clear();
	for (size_t i = 0; i < vAdapters.size(); i++)
	{
		ReleaseCOM(vAdapters[i]);
	}
	vAdapters.clear();		
}

//---------------
int d3dApp::run()
{
	MSG msg{ 0 };
	gameTimer.reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			gameTimer.tick();
			if (!appPaused)
			{
				calculateFrameStats();
				updateScene(gameTimer.getDeltaTime());
				drawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}
	return static_cast<int>(msg.wParam);
}

//---------------------------------------------------------------------------
LRESULT d3dApp::msgHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	//window is selected by the user or deselected 
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			appPaused = true;
			gameTimer.stop();
		}
		else
		{
			appPaused = false;
			gameTimer.start();
		}
		return 0;

	//Called anytime a windows size is changed in any scenario
	//lo and hi byte of lParam contains the updated window width and height
	case WM_SIZE:
		appWidth = LOWORD(lParam);
		appHeight = HIWORD(lParam);
		//if direct3d has been initialized already
		if (d3dDevice)
		{
			//window has been maximized
			if (wParam == SIZE_MAXIMIZED)
			{
				appMinimized = false;
				appMaximized = true;
				onResize();
			}
			//window has been minimized
			else if (wParam == SIZE_MINIMIZED)
			{
				appMinimized = true;
				appMaximized = false; 
				appPaused = true;
				gameTimer.stop();
			}
			//called when window's size change doesn't correspond to either minimize or maximize
			else if (wParam == SIZE_RESTORED)
			{
				//Resizing from minimized state
				if (appMinimized)
				{
					appMinimized = false;
					appPaused = false;
					gameTimer.start();
					onResize();
				}
				//Resizing from maximized state
				else if (appMaximized)
				{
					appMaximized = false;
					onResize();
				}
				//if the user is resizing the bars then we don't need to constantly resize buffers and view
				//therefore do nothing. WM_EXITSIZEMOVE handles resize when the user stops resizing
				else if (appResizing)
				{
					//Do nothing
				}
				else
				{
					/*for other scenarios such as:
					  1.Function SetWindowPos - used to change a windows position on the screen
					  2.Function swapChain->SetFullscreenState - automatically called when pressing alt + enter 
					  etc. */
					onResize();
				}
			}
		}
		return 0;

	/* following two cases consider user manually resizing window and as such
	   need not be handled in WM_SIZE case */
	//User clicked on resize bars of window
	case WM_ENTERSIZEMOVE:
		appPaused = true;
		appResizing = true;
		gameTimer.stop();
		return 0;
	//User lets go of the resize bars
	case WM_EXITSIZEMOVE:
		appPaused = false;
		appResizing = false;
		gameTimer.start();
		onResize();
		return 0;

	//Any key combinations pressed by the user that doesn't correspond to any window menu
	//function shortcut will cause Windows to make a "beep" sound
	//For eg:- when pressing alt + enter to enter fullscreen / leave fullscreen
	//we have no menu; therefore do not make beep sounds
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);

	//Prevent window size from getting too small
	case WM_GETMINMAXINFO:
		reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 200;
		reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 200;
		return 0;

	//Mouse events
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		onMouseButtonDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;		
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		onMouseButtonUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		onMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(appWindow);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//--------------------------------
void d3dApp::calculateFrameStats()
{
	static int frameCount{ 0 };
	static float timeElapsed{ 0 };
	frameCount++;

	//display fps
	if ((gameTimer.totalTime() - timeElapsed) > 1.0f)
	{
		float fps = static_cast<float>(frameCount);
		float mspf = 1000.0f / fps;  //ms per frame

		std::wostringstream wout;
		wout.precision(6);
		wout << windowName << L"  FPS : " << fps << L"  ms : " << mspf;
		SetWindowText(appWindow, wout.str().c_str());

		frameCount = 0;
		timeElapsed += 1.0f;
	}
}

//-------------------------
float d3dApp::aspectRatio()
{
	return static_cast<float>(appWidth) / appHeight;
}

//------------------
d3dApp::~d3dApp() {}