#include "D3DApp.h"
#include "Model.h"

class InitD3DApp : public d3dApp
{
private:

	ComPtr<ID3D11Buffer>vertexBuffer;	

	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11RasterizerState> rastState;

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;

	std::vector<Vertex> points;

public:

	InitD3DApp(HINSTANCE appInstance);
	~InitD3DApp();
	virtual bool init() override;

protected:

	virtual void onResize() override;
	virtual void updateScene(float deltaTime) override;
	virtual void drawScene() override;
	void drawObjectIndexed(const Model * desc);
	
	void buildGeometryData();
	void buildShaderData();
	void createRasterizerBlendStates();
};

//------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(171);	
#endif
	try
	{
		InitD3DApp initD3DApp(appInstance);
		if (!initD3DApp.init())
		{
			return 0;
		}
		return initD3DApp.run();
	}
	catch (const d3dException& d3dexception)
	{
		MessageBox(0, d3dexception.toString().c_str(), L"HR Failed", 0);
		return 0;
	}
}

//-----------------------------------------------------------------
InitD3DApp::InitD3DApp(HINSTANCE appInstance) : d3dApp(appInstance)
{
	
}

//--------------------------
InitD3DApp::~InitD3DApp() {}

//---------------------
bool InitD3DApp::init()
{
	if (!d3dApp::init()) { return false; }

	buildGeometryData();
	buildShaderData();
	createRasterizerBlendStates();
	
	return true;
}

//----------------------------------
void InitD3DApp::buildGeometryData()
{
	Vertex* p;
	generateSierpinskiGasketPoints(points, 3);
	
	//Define buffer desc
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.ByteWidth = points.size() * sizeof(VertexColor);
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;

	//Define sub resource data
	p = &points[0]; //Create a pointer which acts as a built in array for the vector array in memory
	D3D11_SUBRESOURCE_DATA vbData;
	vbData.pSysMem = p;

	//Create vertex buffer
	ThrowIfFailed(d3dDevice->CreateBuffer(&vbDesc, &vbData, vertexBuffer.GetAddressOf()));
}

//--------------------------------
void InitD3DApp::buildShaderData()
{
	unsigned int compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3D10Blob> compiledCode;
	ComPtr<ID3D10Blob> errorMsgs;

	//------------------------------------------------------------//
	//-----------------------VERTEX SHADER------------------------//
	//------------------------------------------------------------//
#if defined(ONLINE) //Runtime shader compilation
	HRESULT result = S_OK;
	result = D3DCompileFromFile(L"Shaders/Box.hlsl", nullptr, nullptr, "vertexShader", "vs_5_0",
		compileFlags, 0, compiledCode.GetAddressOf(), errorMsgs.GetAddressOf());
	if (errorMsgs != nullptr)
	{
		OutputDebugStringA(reinterpret_cast<char*>(errorMsgs->GetBufferPointer()));
		errorMsgs.Reset();
	}
	ThrowIfFailed(result);
#else  //Offline shader compilation
	compiledCode = loadCompiledShaderCodeFromFile("Shaders/box_vs.cso");
#endif
	ThrowIfFailed(d3dDevice->CreateVertexShader(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(),
		nullptr, vertexShader.GetAddressOf()));

	//Define the input element desc structure
	D3D11_INPUT_ELEMENT_DESC inpDesc[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	ThrowIfFailed(d3dDevice->CreateInputLayout(inpDesc, 1, compiledCode->GetBufferPointer(),
		compiledCode->GetBufferSize(), inputLayout.GetAddressOf()));
	compiledCode.Reset();

	//------------------------------------------------------------//
	//-----------------------PIXEL SHADER-------------------------//
	//------------------------------------------------------------//
#if defined(ONLINE)
	result = D3DCompileFromFile(L"Shaders/Box.hlsl", nullptr, nullptr, "pixelShader", "ps_5_0",
		compileFlags, 0, compiledCode.GetAddressOf(), errorMsgs.GetAddressOf());

	if (errorMsgs != nullptr)
	{
		OutputDebugStringA(reinterpret_cast<char*>(errorMsgs->GetBufferPointer()));
		errorMsgs.Reset();
	}
	ThrowIfFailed(result);
#else
	compiledCode = loadCompiledShaderCodeFromFile("Shaders/box_ps.cso");
#endif
	ThrowIfFailed(d3dDevice->CreatePixelShader(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(),
		nullptr, pixelShader.GetAddressOf()));
}

//--------------------------------------------
void InitD3DApp::createRasterizerBlendStates()
{
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(rastDesc));
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;
	ThrowIfFailed(d3dDevice->CreateRasterizerState(&rastDesc, rastState.GetAddressOf()));
}

//-------------------------
void InitD3DApp::onResize()
{
	d3dApp::onResize();
}

//-------------------------------------------
void InitD3DApp::updateScene(float deltaTime)
{}

//--------------------------
void InitD3DApp::drawScene()
{
	assert(swapChain);
	assert(d3dImmediateContext);

	d3dImmediateContext->ClearRenderTargetView(renderTargetView.Get(), Colors::White);
	d3dImmediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Set shader programs
	d3dImmediateContext->VSSetShader(vertexShader.Get(), 0, 0);
	d3dImmediateContext->PSSetShader(pixelShader.Get(), 0, 0);

	//Set Rasterizer state
	d3dImmediateContext->RSSetState(rastState.Get());

	//Set Input Layout
	d3dImmediateContext->IASetInputLayout(inputLayout.Get());

	//Set primitive topology
	d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	drawObjectIndexed(nullptr);
	
	//Draw Cube
	ThrowIfFailed(swapChain->Present(0, 0));
}

//----------------------------------------------------
void InitD3DApp::drawObjectIndexed(const Model* model)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	//Set Buffers
	d3dImmediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	//Draw
	d3dImmediateContext->Draw(points.size(), 0);
}