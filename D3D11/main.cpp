#include "D3DApp.h"
#include "Model.h"
#include "Turtle.h"

class InitD3DApp : public d3dApp
{
private:

	ComPtr<ID3D11Buffer>vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;

	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11RasterizerState> rastState;

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;

	std::vector<Vertex> points;
	std::vector<unsigned int> indices;

	Turtle t;

public:

	InitD3DApp(HINSTANCE appInstance);
	~InitD3DApp();
	virtual bool init() override;


protected:

	virtual void onResize() override;
	virtual void updateScene(float deltaTime) override;
	virtual void drawScene() override;
	void drawObjectIndexed(const Model* desc);

	void TurtleCommands();
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

	TurtleCommands();

	buildGeometryData();
	buildShaderData();
	createRasterizerBlendStates();

	return true;
}

//-------------------------------
void InitD3DApp::TurtleCommands()
{
	t.init(0.0f, 0.0f, 0);
	t.togglePen(true);
	t.forward(0.2f);
	t.left(90);
	t.togglePen(false);
	t.forward(0.3f);
	t.left(90);
	t.forward(0.2f);
	t.left(90);
	t.forward(0.3f);
	t.left(90);
	t.forward(0.2f);

	points = t.getPoints();
	indices = t.getIndices();
}

//----------------------------------
void InitD3DApp::buildGeometryData()
{
	//Define buffer desc
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.ByteWidth = 1000 * sizeof(Vertex);
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;

	//Create vertex buffer
	ThrowIfFailed(d3dDevice->CreateBuffer(&vbDesc, nullptr, vertexBuffer.GetAddressOf()));

	D3D11_BUFFER_DESC idxDesc;
	idxDesc.ByteWidth = sizeof(unsigned int) * 1000;
	idxDesc.Usage = D3D11_USAGE_DYNAMIC;
	idxDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	idxDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	idxDesc.MiscFlags = 0;
	idxDesc.StructureByteStride = 0;

	ThrowIfFailed(d3dDevice->CreateBuffer(&idxDesc, nullptr, indexBuffer.GetAddressOf()));
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
		//{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
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
	d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

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
	d3dImmediateContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//Update Buffers with new data
	D3D11_MAPPED_SUBRESOURCE mappedSubResource;

	d3dImmediateContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
	memcpy(mappedSubResource.pData, &points[0], sizeof(Vertex) * points.size());
	d3dImmediateContext->Unmap(vertexBuffer.Get(), 0);

	d3dImmediateContext->Map(indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
	memcpy(mappedSubResource.pData, &indices[0], sizeof(unsigned int) * indices.size());
	d3dImmediateContext->Unmap(indexBuffer.Get(), 0);

	//Draw
	d3dImmediateContext->DrawIndexed(indices.size(), 0, 0);
}