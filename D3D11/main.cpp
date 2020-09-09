#include "D3DApp.h"
#include "Lighting.h"
#include "MathHelper.h"

struct cbufferPerFrame
{
	cbufferPerFrame() { ZeroMemory(this, sizeof(this)); }

	DirectLight dirLight;
	PointLight pointLight;
	SpotLight spotLight;
	XMFLOAT4 viewPos;
} cbufferperframe;

struct cbufferPerObject
{
	cbufferPerObject() { ZeroMemory(this, sizeof(this)); }

	XMMATRIX worldViewProj;
	XMMATRIX worldInvTranspose;
	XMMATRIX world;
	Material material;
} cbufferperobject;

class InitD3DApp : public d3dApp
{
private:

	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11RasterizerState> rastState;

	ComPtr<ID3D11Buffer> cubeVertexBuffer;
	ComPtr<ID3D11Buffer> cubeIndexBuffer;
	ComPtr<ID3D11Buffer> pyramidVertexBuffer;
	ComPtr<ID3D11Buffer> pyramidIndexBuffer;
	ComPtr<ID3D11Buffer> constantBufferPerObject;
	ComPtr<ID3D11Buffer> constantBufferPerFrame;

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;

	XMFLOAT4X4 cubeWorldMatrix;
	XMFLOAT4X4 pyramidWorldMatrix; 
	XMFLOAT4X4 fViewMatrix;
	XMFLOAT4X4 fProjMatrix;

	//Materials
	Material cubeMaterial;
	Material pyramidMaterial;	

	DRAW_INDEXED_DESC drawIndexedDesc;

public:

	InitD3DApp(HINSTANCE appInstance);
	~InitD3DApp();
	virtual bool init() override;

protected:

	virtual void onResize() override;
	virtual void updateScene(float deltaTime) override;
	virtual void drawScene() override;
	void drawObjectIndexed(const DRAW_INDEXED_DESC * desc);
	
	void buildGeometryData();
	void buildShaderData();
	void setupLightingData();
	void createRasterizerStates();
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
	XMStoreFloat4x4(&pyramidWorldMatrix, identity);
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
	setupLightingData();
	createRasterizerStates();

	return true;
}

//----------------------------------
void InitD3DApp::buildGeometryData()
{
	//-----------------------------------------------------//
	//-----------------------CUBE--------------------------//
	//-----------------------------------------------------//
	VertexWithNormal cubeVertices[] =
	{
		{XMFLOAT3{-0.5f, -0.5f, 0.5f}, XMFLOAT3()},  //Rear Bottom Left - 0
		{XMFLOAT3{-0.5f, 0.5f, 0.5f}, XMFLOAT3()}, //Rear Top Left - 1
		{XMFLOAT3{0.5f, 0.5f, 0.5f}, XMFLOAT3()},  //Rear Top Right - 2
		{XMFLOAT3{0.5f, -0.5f, 0.5f}, XMFLOAT3()}, //Rear Bottom Right - 3
		{XMFLOAT3{-0.5f, -0.5f, -0.5f}, XMFLOAT3()}, //Front Bottom Left - 4
		{XMFLOAT3{-0.5f, 0.5f, -0.5f}, XMFLOAT3()}, //Front Top Left - 5
		{XMFLOAT3{0.5f, 0.5f, -0.5f}, XMFLOAT3()}, //Front Top Right - 6
		{XMFLOAT3{0.5f, -0.5f, -0.5f}, XMFLOAT3()}, //Front Bottom Right - 7		
	};

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

	calculateNormals(cubeVertices, 8, cubeIndices, 12);

	//Define buffer desc
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.ByteWidth = 8 * sizeof(VertexWithNormal);
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

	//Create Index buffer
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

	//------------------------------------------------------//
	//---------------------PYRAMID--------------------------//
	//------------------------------------------------------//
	VertexWithNormal pyramidVertices[] =
	{
		{XMFLOAT3{-0.5f, -0.5f, 0.5f}, XMFLOAT3()},  //Rear Bottom Left - 0
		{XMFLOAT3{0.5f, -0.5f, 0.5f}, XMFLOAT3()}, //Rear Bottom Right - 1
		{XMFLOAT3{-0.5f, -0.5f, -0.5f}, XMFLOAT3()}, //Front Bottom Left - 2
		{XMFLOAT3{0.5f, -0.5f, -0.5f}, XMFLOAT3()}, //Front Bottom Right - 3		
		{XMFLOAT3{0.0f, 0.5f, 0.0f}, XMFLOAT3()}, //Top - 4		
	};

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

	calculateNormals(pyramidVertices, 5, pyramidIndices, 6);

	vbDesc.ByteWidth = 5 * sizeof(VertexWithNormal);
	vbData.pSysMem = pyramidVertices;
	//Create vertex buffer
	ThrowIfFailed(d3dDevice->CreateBuffer(&vbDesc, &vbData, pyramidVertexBuffer.GetAddressOf()));

	idxDesc.ByteWidth = sizeof(unsigned int) * 18;
	idxData.pSysMem = pyramidIndices;

	ThrowIfFailed(d3dDevice->CreateBuffer(&idxDesc, &idxData, pyramidIndexBuffer.GetAddressOf()));
	
	//-----------------------------------------------------------------------//
	//-----------------------------CONSTANT----------------------------------//
	//-----------------------------------------------------------------------//

	D3D11_BUFFER_DESC constDesc;
	ZeroMemory(&constDesc, sizeof(D3D11_BUFFER_DESC));
	constDesc.ByteWidth = sizeof(cbufferPerObject);
	constDesc.Usage = D3D11_USAGE_DYNAMIC;
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ThrowIfFailed(d3dDevice->CreateBuffer(&constDesc, nullptr, constantBufferPerObject.GetAddressOf()));

	constDesc.ByteWidth = sizeof(cbufferPerFrame);
	ThrowIfFailed(d3dDevice->CreateBuffer(&constDesc, nullptr, constantBufferPerFrame.GetAddressOf()));

	XMMATRIX translate = XMMatrixTranslation(0.0f, -1.0f, 0.0f);
	XMMATRIX rotation = XMMatrixRotationAxis(XMVECTORF32{ 0.0f, 1.0f, 0.0f }, XMConvertToRadians(45.0f));
	XMStoreFloat4x4(&cubeWorldMatrix, rotation * translate);

	translate = XMMatrixTranslation(3.5f, -1.0f, 0.0f);
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

	//------------------------------------------------------------//
	//-----------------------VERTEX SHADER------------------------//
	//------------------------------------------------------------//
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
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	ThrowIfFailed(d3dDevice->CreateInputLayout(inpDesc, 2, compiledCode->GetBufferPointer(),
		compiledCode->GetBufferSize(), inputLayout.GetAddressOf()));
	compiledCode.Reset();

	//------------------------------------------------------------//
	//-----------------------PIXEL SHADER-------------------------//
	//------------------------------------------------------------//
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

//----------------------------------
void InitD3DApp::setupLightingData()
{
	//Material settings
	cubeMaterial.ambientColor = XMFLOAT4(Colors::DarkRed);
	cubeMaterial.diffuseColor = XMFLOAT4(Colors::DarkRed);
	cubeMaterial.specColor = { 1.0f, 1.0f, 1.0f, 8.0f };  //w = specular Power

	pyramidMaterial.ambientColor = XMFLOAT4(Colors::DarkGreen);
	pyramidMaterial.diffuseColor = XMFLOAT4(Colors::DarkGreen);
	pyramidMaterial.specColor = { 1.0f, 1.0f, 1.0f, 8.0f };  //w = specular Power

	//Directional Light settings
	cbufferperframe.dirLight.ambientColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	cbufferperframe.dirLight.diffuseColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	cbufferperframe.dirLight.specColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	cbufferperframe.dirLight.lightDir = {0.0f, 1.0f, 0.0f};

	//Point Light settings
	cbufferperframe.pointLight.ambientColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	cbufferperframe.pointLight.diffuseColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	cbufferperframe.pointLight.specColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	cbufferperframe.pointLight.lightPos = { 0.0f, 1.0f, 0.0f };
	cbufferperframe.pointLight.range = 10.0f;
	cbufferperframe.pointLight.att = { 0.0f, 1.0f, 0.0f };

	//Spot Light settings
	cbufferperframe.spotLight.ambientColor = { 0.0f, 0.0f, 1.0f, 1.0f };
	cbufferperframe.spotLight.diffuseColor = { 0.0f, 0.0f, 1.0f, 1.0f };
	cbufferperframe.spotLight.specColor = { 0.0f, 0.0f, 1.0f, 1.0f };
	cbufferperframe.spotLight.range = 10.0f;
	cbufferperframe.spotLight.att = { 0.0f, 1.0f, 0.0f };
	cbufferperframe.spotLight.spotPower = 8.0f;
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

	immediateContext->ClearRenderTargetView(renderTargetView.Get(), Colors::Black);
	immediateContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Set shader programs
	immediateContext->VSSetShader(vertexShader.Get(), 0, 0);
	immediateContext->PSSetShader(pixelShader.Get(), 0, 0);

	//Set Rasterizer state
	immediateContext->RSSetState(rastState.Get());

	//Set Constant Buffers
	immediateContext->VSSetConstantBuffers(0, 1, constantBufferPerObject.GetAddressOf());
	immediateContext->PSSetConstantBuffers(0, 1, constantBufferPerObject.GetAddressOf());
	immediateContext->PSSetConstantBuffers(1, 1, constantBufferPerFrame.GetAddressOf());
	
	//Set Input Layout
	immediateContext->IASetInputLayout(inputLayout.Get());

	//Set primitive topology
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//CBUFFER PER FRAME
	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	XMFLOAT3 spotLightPos = Float4ToFloat3(&(getCameraPos()));
	XMFLOAT3 spotLightDir = getCameraTarget();

	cbufferperframe.viewPos = getCameraPos();
	cbufferperframe.spotLight.lightPos = spotLightPos;
	cbufferperframe.spotLight.lightDir = spotLightDir;

	ZeroMemory(&mappedSubResource, sizeof(mappedSubResource));
	immediateContext->Map(constantBufferPerFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
	memcpy(mappedSubResource.pData, &cbufferperframe, sizeof(cbufferPerFrame));
	immediateContext->Unmap(constantBufferPerFrame.Get(), 0);

	//Draw Cube
	drawIndexedDesc.vertexBuffer = cubeVertexBuffer;
	drawIndexedDesc.stride = sizeof(VertexWithNormal);
	drawIndexedDesc.worldMatrix = cubeWorldMatrix;
	drawIndexedDesc.material = cubeMaterial;
	drawIndexedDesc.indexBuffer = cubeIndexBuffer;
	drawIndexedDesc.indexCount = 36;
	drawObjectIndexed(&drawIndexedDesc);

	//Draw Pyramid
	drawIndexedDesc.vertexBuffer = pyramidVertexBuffer;
	drawIndexedDesc.worldMatrix = pyramidWorldMatrix;
	drawIndexedDesc.material = pyramidMaterial;
	drawIndexedDesc.indexBuffer = pyramidIndexBuffer;
	drawIndexedDesc.indexCount = 18;
	drawObjectIndexed(&drawIndexedDesc);
	ThrowIfFailed(swapChain->Present(0, 0));
}

//----------------------------------------------------------------
void InitD3DApp::drawObjectIndexed(const DRAW_INDEXED_DESC * desc)
{
	//==================//
	//CBUFFER PER OBJECT//
	//==================//

	//World View Projection Matrix
	XMMATRIX worldViewProj;
	XMMATRIX world = XMLoadFloat4x4(&(desc->worldMatrix));
	XMMATRIX view = XMLoadFloat4x4(&fViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&fProjMatrix);
	worldViewProj = world * view * proj;
	//DirectXMath - Matrix(row major), HLSL - Matrix(Column Major) 
	//Therefore, transpose to a column matrix as then it will be a row matrix in hlsl
	worldViewProj = XMMatrixTranspose(worldViewProj);

	//World Inverse Transpose Matrix
	XMMATRIX worldInvTranspose;
	//No transpose as have to store matrix in column format for it to be stored as row format in hlsl
	worldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world); 

	cbufferperobject.worldInvTranspose = worldInvTranspose;
	cbufferperobject.worldViewProj = worldViewProj;
	cbufferperobject.world = XMMatrixTranspose(world);
	cbufferperobject.material = desc->material;

	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	ZeroMemory(&mappedSubResource, sizeof(mappedSubResource));
	immediateContext->Map(constantBufferPerObject.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
	memcpy(mappedSubResource.pData, &cbufferperobject, sizeof(cbufferPerObject));
	immediateContext->Unmap(constantBufferPerObject.Get(), 0);

	//=================================================//

	//Set Buffers
	immediateContext->IASetVertexBuffers(0, 1, desc->vertexBuffer.GetAddressOf(), &(desc->stride), &(desc->offset));
	immediateContext->IASetIndexBuffer(desc->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//Draw
	immediateContext->DrawIndexed(desc->indexCount, desc->startIndex, desc->baseVertex);
}