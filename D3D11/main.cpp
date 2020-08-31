#include "D3DApp.h"

class InitD3DApp : public d3dApp
{
public:

	InitD3DApp(HINSTANCE appInstance);
	~InitD3DApp();
	virtual bool init() override;

protected:

	virtual void onResize() override;
	virtual void updateScene(float deltaTime) override;
	virtual void drawScene() override;
	void drawObjectIndexed(ComPtr<ID3D11Buffer>vertexBuffer, ComPtr<ID3D11Buffer>indexBuffer, XMFLOAT4X4 worldMatrix,
		UINT indexCount, UINT stride, UINT offset = 0, UINT startIndex = 0, UINT baseVertex = 0);
	virtual LRESULT msgHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

	virtual void onMouseMove(WPARAM wParam, int x, int y) override;
	virtual void onMouseButtonDown(WPARAM wParam, int x, int y) override;

	void moveCamera(WPARAM wParam);
	void buildGeometryData();
	void buildShaderData();
	void createRasterizerStates();

private:

	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11RasterizerState> rastState;

	ComPtr<ID3D11Buffer> cubeVertexBuffer;
	ComPtr<ID3D11Buffer> cubeIndexBuffer;
	ComPtr<ID3D11Buffer> pyramidVertexBuffer;
	ComPtr<ID3D11Buffer> pyramidIndexBuffer;
	ComPtr<ID3D11Buffer> constantBuffer;

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	XMFLOAT4X4 cubeWorldMatrix;
	XMFLOAT4X4 pyramidWorldMatrix;
	XMFLOAT4X4 fViewMatrix;
	XMFLOAT4X4 fProjMatrix;

	//Camera variables
	XMFLOAT3 camLookAt;
	XMFLOAT4 camPos;
	float yaw{ 0 };
	float pitch{ 0 };
	float lastX{ 0 };
	float lastY{ 0 };
	float camSpeed{ 100.0f };
	float camSens{ 0.1f };
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
	XMMATRIX identity = XMMatrixIdentity();
	XMStoreFloat4x4(&cubeWorldMatrix, identity);
	XMStoreFloat4x4(&fViewMatrix, identity);
	XMStoreFloat4x4(&fProjMatrix, identity);

	camPos = { 0.0f, 0.0f, -2.5f, 1.0f };
	camLookAt = { 0.0f, 0.0f, 1.0f };
}

//--------------------------
InitD3DApp::~InitD3DApp() {}

//---------------------
bool InitD3DApp::init()
{
	if (!d3dApp::init()) { return false; }
	buildShaderData();
	buildGeometryData();
	createRasterizerStates();

	return true;
}

//----------------------------------
void InitD3DApp::buildGeometryData()
{
	//------------------------------------------------------------------//
	//------------------------VERTEX------------------------------------//
	//------------------------------------------------------------------//
	//Cube Data
	Vertex cubeVertices[] =
	{
		{XMFLOAT3{-0.5f, -0.5f, 0.5f}, XMFLOAT4(Colors::BlanchedAlmond)},  //Rear Bottom Left - 0
		{XMFLOAT3{-0.5f, 0.5f, 0.5f}, XMFLOAT4(Colors::Chocolate)}, //Rear Top Left - 1
		{XMFLOAT3{0.5f, 0.5f, 0.5f}, XMFLOAT4(Colors::Aquamarine)},  //Rear Top Right - 2
		{XMFLOAT3{0.5f, -0.5f, 0.5f}, XMFLOAT4(Colors::DarkGray)}, //Rear Bottom Right - 3
		{XMFLOAT3{-0.5f, -0.5f, -0.5f}, XMFLOAT4(Colors::Red)}, //Front Bottom Left - 4
		{XMFLOAT3{-0.5f, 0.5f, -0.5f}, XMFLOAT4(Colors::Green)}, //Front Top Left - 5
		{XMFLOAT3{0.5f, 0.5f, -0.5f}, XMFLOAT4(Colors::Blue)}, //Front Top Right - 6
		{XMFLOAT3{0.5f, -0.5f, -0.5f}, XMFLOAT4(Colors::DarkMagenta)}, //Front Bottom Right - 7		
	};

	//Define buffer desc
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.ByteWidth = 8 * sizeof(Vertex);
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;

	//Define sub resource data
	D3D11_SUBRESOURCE_DATA vbData;
	vbData.pSysMem = cubeVertices;

	//Create vertex buffer
	ThrowIfFailed(d3dDevice->CreateBuffer(&vbDesc, &vbData, cubeVertexBuffer.GetAddressOf()));

	//Pyramid Data
	Vertex pyramidVertices[] =
	{
		{XMFLOAT3{-0.5f, -0.5f, 0.5f}, XMFLOAT4(Colors::BlanchedAlmond)},  //Rear Bottom Left - 0
		{XMFLOAT3{0.5f, -0.5f, 0.5f}, XMFLOAT4(Colors::DarkGray)}, //Rear Bottom Right - 1
		{XMFLOAT3{-0.5f, -0.5f, -0.5f}, XMFLOAT4(Colors::Red)}, //Front Bottom Left - 2
		{XMFLOAT3{0.5f, -0.5f, -0.5f}, XMFLOAT4(Colors::Blue)}, //Front Bottom Right - 3		
		{XMFLOAT3{0.0f, 0.5f, 0.0f}, XMFLOAT4(Colors::Green)}, //Top - 4		
	};

	vbDesc.ByteWidth = 5 * sizeof(Vertex);
	vbData.pSysMem = pyramidVertices;

	//Create vertex buffer
	ThrowIfFailed(d3dDevice->CreateBuffer(&vbDesc, &vbData, pyramidVertexBuffer.GetAddressOf()));

	//------------------------------------------------------------------//
	//-------------------------INDEX------------------------------------//
	//------------------------------------------------------------------//
	//Cube Indices
	unsigned int cubeIndices[] = {
		//Top Face
		5, 1, 2,
		5, 2, 6,

		//Back Face
		1, 0, 3,
		1, 3, 2,

		//Bottom Face
		0, 4, 7,
		0, 7, 3,

		//Front Face
		4, 5, 6,
		4, 6, 7,

		//Left Face
		0, 1, 5,
		0, 5, 4,

		//Right Face
		7, 6, 2,
		7, 2, 3
	};

	D3D11_BUFFER_DESC idxDesc;
	idxDesc.ByteWidth = sizeof(unsigned int) * 36;
	idxDesc.Usage = D3D11_USAGE_IMMUTABLE;
	idxDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	idxDesc.CPUAccessFlags = 0;
	idxDesc.MiscFlags = 0;
	idxDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA idxData;
	idxData.pSysMem = cubeIndices;

	ThrowIfFailed(d3dDevice->CreateBuffer(&idxDesc, &idxData, cubeIndexBuffer.GetAddressOf()));

	//Pyramid Indices
	unsigned int pyramidIndices[] = {
		//Back Face
		4, 0, 1,

		//Bottom Face
		2, 1, 0,
		2, 3, 1,

		//Front Face
		2, 4, 3,

		//Left Face
		0, 4, 2,

		//Right Face
		3, 4, 1
	};

	idxDesc.ByteWidth = sizeof(unsigned int) * 18;
	idxData.pSysMem = pyramidIndices;

	ThrowIfFailed(d3dDevice->CreateBuffer(&idxDesc, &idxData, pyramidIndexBuffer.GetAddressOf()));

	//-----------------------------------------------------------------------//
	//-----------------------------CONSTANT----------------------------------//
	//-----------------------------------------------------------------------//

	D3D11_BUFFER_DESC constDesc;
	ZeroMemory(&constDesc, sizeof(D3D11_BUFFER_DESC));
	constDesc.ByteWidth = sizeof(XMMATRIX);
	constDesc.Usage = D3D11_USAGE_DYNAMIC;
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ThrowIfFailed(d3dDevice->CreateBuffer(&constDesc, nullptr, constantBuffer.GetAddressOf()));

	XMMATRIX translate = XMMatrixTranslation(0.0f, -1.0f, 0.0f);
	XMMATRIX rotation = XMMatrixRotationAxis(XMVECTORF32{ 0.0f, 1.0f, 0.0f }, XMConvertToRadians(45.0f));
	XMStoreFloat4x4(&cubeWorldMatrix, rotation * translate);

	translate = XMMatrixTranslation(1.5f, -1.0f, 0.0f);
	XMStoreFloat4x4(&pyramidWorldMatrix, translate);
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

	//Vertex Shader
#if defined(ONLINE) //Runtime shader compilation
	HRESULT result = S_OK;
	result = D3DCompileFromFile(L"Box.hlsl", nullptr, nullptr, "vertexShader", "vs_5_0",
		compileFlags, 0, compiledCode.GetAddressOf(), errorMsgs.GetAddressOf());
	if (errorMsgs != nullptr)
	{
		OutputDebugStringA(reinterpret_cast<char*>(errorMsgs->GetBufferPointer()));
		errorMsgs.Reset();
	}
	ThrowIfFailed(result);
#else  //Offline shader compilation
	compiledCode = loadShaderCodeFromFile("box_vs.cso");
#endif
	ThrowIfFailed(d3dDevice->CreateVertexShader(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(),
		nullptr, vertexShader.GetAddressOf()));

	//Define the input element desc structure
	D3D11_INPUT_ELEMENT_DESC inpDesc[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	ThrowIfFailed(d3dDevice->CreateInputLayout(inpDesc, 2, compiledCode->GetBufferPointer(),
		compiledCode->GetBufferSize(), inputLayout.GetAddressOf()));
	compiledCode.Reset();

	//Pixel Shader
#if defined(ONLINE)
	result = D3DCompileFromFile(L"Box.hlsl", nullptr, nullptr, "pixelShader", "ps_5_0",
		compileFlags, 0, compiledCode.GetAddressOf(), errorMsgs.GetAddressOf());

	if (errorMsgs != nullptr)
	{
		OutputDebugStringA(reinterpret_cast<char*>(errorMsgs->GetBufferPointer()));
		errorMsgs.Reset();
	}
	ThrowIfFailed(result);
#else
	compiledCode = loadShaderCodeFromFile("box_ps.cso");
#endif
	ThrowIfFailed(d3dDevice->CreatePixelShader(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(),
		nullptr, pixelShader.GetAddressOf()));
}

//---------------------------------------
void InitD3DApp::createRasterizerStates()
{
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(rastDesc));
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;
	
	ThrowIfFailed(d3dDevice->CreateRasterizerState(&rastDesc, rastState.GetAddressOf()));
}

//-------------------------------------------------------------------------------
LRESULT InitD3DApp::msgHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		moveCamera(wParam);
		break;   //No return statement as ESCAPE is handled separately in base msgHandler
	}

	return d3dApp::msgHandler(hWnd, msg, wParam, lParam);
}

//-------------------------------------------------------------
void InitD3DApp::onMouseButtonDown(WPARAM wParam, int x, int y)
{
	lastX = x;
	lastY = y;
}

//-------------------------------------------------------
void InitD3DApp::onMouseMove(WPARAM wParam, int x, int y)
{
	if ((wParam & MK_LBUTTON) != 0)
	{
		float xOffset = x - lastX;
		float yOffset = y - lastY;

		xOffset *= camSens;
		yOffset *= camSens;

		lastX = x;
		lastY = y;

		yaw += xOffset;
		pitch += yOffset;

		if (pitch > 90) { pitch = 90; }
		if (pitch < -90) { pitch = -90; }

		float newX, newY, newZ;
		newX = sinf(XMConvertToRadians(yaw)) * cosf(XMConvertToRadians(pitch));
		newY = -sinf(XMConvertToRadians(pitch));
		newZ = cosf(XMConvertToRadians(pitch)) * cosf(XMConvertToRadians(yaw));

		camLookAt.x = newX;
		camLookAt.y = newY;
		camLookAt.z = newZ;

		//Normalize
		XMVECTOR camLookAt = XMLoadFloat3(&(this->camLookAt));
		camLookAt = XMVector3Normalize(camLookAt);
		XMStoreFloat3(&(this->camLookAt), camLookAt);
	}
}

//----------------------------------------
void InitD3DApp::moveCamera(WPARAM wParam)
{
	XMVECTOR camPos = XMLoadFloat4(&(this->camPos));
	XMVECTOR camLookAt = XMLoadFloat3(&(this->camLookAt));
	XMVECTOR right = XMVectorZero();

	//W - Move Forward
	if (wParam == 0x57)
	{
		camPos += camLookAt * camSpeed * gameTimer.getDeltaTime();
		XMStoreFloat4(&(this->camPos), camPos);
	}

	//S - Move Backward
	if (wParam == 0x53)
	{
		camPos -= camLookAt * camSpeed * gameTimer.getDeltaTime();
		XMStoreFloat4(&(this->camPos), camPos);
	}

	//A - Move Left
	if (wParam == 0x41)
	{
		right = XMVector3Normalize(XMVector3Cross({ 0.0f, 1.0f, 0.0f }, camLookAt));
		camPos -= right * camSpeed * gameTimer.getDeltaTime();
		XMStoreFloat4(&(this->camPos), camPos);
	}

	//D - Move Right
	if (wParam == 0x44)
	{
		right = XMVector3Normalize(XMVector3Cross({ 0.0f, 1.0f, 0.0f }, camLookAt));
		camPos += right * camSpeed * gameTimer.getDeltaTime();
		XMStoreFloat4(&(this->camPos), camPos);
	}
}

//-------------------------------------------
void InitD3DApp::updateScene(float deltaTime)
{
	XMVECTOR camPos = XMLoadFloat4(&(this->camPos));
	XMVECTOR camLookAt = XMLoadFloat3(&(this->camLookAt));
	XMVECTORF32 worldUp{ 0.0f, 1.0f, 0.0f };

	XMMATRIX mViewMatrix = XMMatrixLookAtLH(camPos, camPos + camLookAt, worldUp);
	XMStoreFloat4x4(&fViewMatrix, mViewMatrix);
}

//-------------------------
void InitD3DApp::onResize()
{
	d3dApp::onResize();

	//Calculate new projection matrix
	XMMATRIX mProjMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), aspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&fProjMatrix, mProjMatrix);
}

//--------------------------
void InitD3DApp::drawScene()
{
	assert(swapChain);
	assert(immediateContext);

	immediateContext->ClearRenderTargetView(renderTargetView.Get(), Colors::White);
	immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Set shader programs
	immediateContext->VSSetShader(vertexShader.Get(), 0, 0);
	immediateContext->PSSetShader(pixelShader.Get(), 0, 0);

	//Set Rasterizer state
	immediateContext->RSSetState(rastState.Get());

	//Set Constant Buffer
	immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

	//Set Input Layout
	immediateContext->IASetInputLayout(inputLayout.Get());

	//Set primitive topology
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Draw
	drawObjectIndexed(cubeVertexBuffer, cubeIndexBuffer, cubeWorldMatrix, 36, sizeof(Vertex));
	drawObjectIndexed(pyramidVertexBuffer, pyramidIndexBuffer, pyramidWorldMatrix, 18, sizeof(Vertex));
	ThrowIfFailed(swapChain->Present(0, 0));
}

//---------------------------------------------------------------------------------------------------------------------------
void InitD3DApp::drawObjectIndexed(ComPtr<ID3D11Buffer>vertexBuffer, ComPtr<ID3D11Buffer>indexBuffer, XMFLOAT4X4 worldMatrix,
	UINT indexCount, UINT stride, UINT offset, UINT startIndex, UINT baseVertex)
{
	XMMATRIX worldViewProj;
	XMMATRIX world = XMLoadFloat4x4(&worldMatrix);
	XMMATRIX view = XMLoadFloat4x4(&fViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&fProjMatrix);
	worldViewProj = world * view * proj;
	//DirectXMath - Matrix(row major), HLSL - Matrix(Column Major) 
	//Therefore, transpose to a column matrix as then it will be a row matrix in hlsl
	worldViewProj = XMMatrixTranspose(worldViewProj);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(mappedResource));
	immediateContext->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &worldViewProj, sizeof(XMMATRIX));
	immediateContext->Unmap(constantBuffer.Get(), 0);

	immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	immediateContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	immediateContext->DrawIndexed(indexCount, startIndex, baseVertex);
}