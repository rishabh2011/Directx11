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
	XMMATRIX texTransformMatrix;
	Material material;
	int useTexture;
	int clipAlpha;
} cbufferperobject;

class InitD3DApp : public d3dApp
{
private:

	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11RasterizerState> rastState;
	ComPtr<ID3D11BlendState> blendState;

	ComPtr<ID3D11Buffer> constantBufferPerObject;
	ComPtr<ID3D11Buffer> constantBufferPerFrame;

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;

	XMFLOAT4X4 fViewMatrix;
	XMFLOAT4X4 fProjMatrix;

	//Models
	Model cubeModel{ sizeof(VertexNormTex), 36 };
	std::vector<XMFLOAT3> cubeTranslateVectors;

	Model quadModel{ sizeof(VertexNormTex), 6 };
	std::vector<XMFLOAT3> quadTranslateVectors;	
	std::map<float, XMFLOAT3, std::less<float>> quadDistancesFromCamera;

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
	void setupLightingData();
	void setupModelTextureData();
	void setupSamplerState();
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
	XMMATRIX identity = XMMatrixIdentity();
	XMStoreFloat4x4(&fViewMatrix, identity);
	XMStoreFloat4x4(&fProjMatrix, identity);
	
	camPos = { 0.0f, 0.0f, -5.0f, 1.0f };
	camLookAt = { 0.0f, 0.0f, 1.0f };

	cubeTranslateVectors.push_back({ 0.5f, -1.0f, -2.0f });
	cubeTranslateVectors.push_back({ 1.0f, -0.5f, -0.5f });
	cubeTranslateVectors.push_back({ -0.5f, 0.47f, 0.4f });
	
	quadTranslateVectors.push_back({ 1.0f, -1.0f, -2.2f });
	quadTranslateVectors.push_back({ 0.0f, -0.5f, -1.0f });
	quadTranslateVectors.push_back({ 0.27f, 0.47f, 0.3f });
	quadTranslateVectors.push_back({ 0.0f,  0.0f, -2.4f });
	quadTranslateVectors.push_back({ 0.0f,  0.83f, 0.0f });
}

//--------------------------
InitD3DApp::~InitD3DApp() {}

//---------------------
bool InitD3DApp::init()
{
	if (!d3dApp::init()) { return false; }

	buildGeometryData();
	buildShaderData();
	setupLightingData();
	setupModelTextureData();
	setupSamplerState();
	createRasterizerBlendStates();
	
	return true;
}

//----------------------------------
void InitD3DApp::buildGeometryData()
{
	//-----------------------------------------------------//
	//-----------------------CUBE--------------------------//
	//-----------------------------------------------------//
	calculateNormals(cubeVertices, 24, cubeIndices, 12);
	
	//Define buffer desc
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.ByteWidth = 24 * sizeof(VertexNormTex);
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;

	//Define sub resource data
	D3D11_SUBRESOURCE_DATA vbData;
	vbData.pSysMem = cubeVertices;

	//Create vertex buffer
	ThrowIfFailed(d3dDevice->CreateBuffer(&vbDesc, &vbData, cubeModel.vertexBuffer.GetAddressOf()));

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

	ThrowIfFailed(d3dDevice->CreateBuffer(&idxDesc, &idxData, cubeModel.indexBuffer.GetAddressOf()));

	//Setup World Matrix
	XMMATRIX translate = XMMatrixTranslation(0.0f, -1.0f, 0.0f);
	XMMATRIX rotation = XMMatrixRotationAxis(XMVECTORF32{ 0.0f, 1.0f, 0.0f }, XMConvertToRadians(45.0f));
	XMStoreFloat4x4(&cubeModel.worldMatrix, rotation * translate);

	//-----------------------------------------------------//
	//-----------------------QUAD--------------------------//
	//-----------------------------------------------------//
	calculateNormals(quadVertices, 4, quadIndices, 2);

	vbDesc.ByteWidth = sizeof(VertexNormTex) * 4;
	vbData.pSysMem = quadVertices;
	ThrowIfFailed(d3dDevice->CreateBuffer(&vbDesc, &vbData, quadModel.vertexBuffer.GetAddressOf()));
	
	idxDesc.ByteWidth = sizeof(unsigned int) * 6;
	idxData.pSysMem = quadIndices;
	ThrowIfFailed(d3dDevice->CreateBuffer(&idxDesc, &idxData, quadModel.indexBuffer.GetAddressOf()));
		
	//----------------
	//CONSTANT BUFFERS
	//----------------
	D3D11_BUFFER_DESC constDesc;
	ZeroMemory(&constDesc, sizeof(D3D11_BUFFER_DESC));
	constDesc.ByteWidth = sizeof(cbufferPerObject);
	constDesc.Usage = D3D11_USAGE_DEFAULT;
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ThrowIfFailed(d3dDevice->CreateBuffer(&constDesc, nullptr, constantBufferPerObject.GetAddressOf()));

	constDesc.Usage = D3D11_USAGE_DYNAMIC;
	constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constDesc.ByteWidth = sizeof(cbufferPerFrame);
	ThrowIfFailed(d3dDevice->CreateBuffer(&constDesc, nullptr, constantBufferPerFrame.GetAddressOf()));	
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
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	ThrowIfFailed(d3dDevice->CreateInputLayout(inpDesc, 3, compiledCode->GetBufferPointer(),
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
	ID3D11ShaderReflection* reflectionInterface;
	D3DReflect(compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflectionInterface);

	//Used to check which register given resource is bound to in shader file
	D3D11_SHADER_INPUT_BIND_DESC bindDesc;
	reflectionInterface->GetResourceBindingDescByName("diffuseMap", &bindDesc);
}

//----------------------------------
void InitD3DApp::setupLightingData()
{
	//Cube Material settings
	cubeModel.material.ambientColor = {0.5f, 0.5f, 0.5f, 1.0f};
	cubeModel.material.diffuseColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	cubeModel.material.specColor = { 1.0f, 1.0f, 1.0f, 8.0f };  //w = specular Power

	//Quad Material settings
	quadModel.material.ambientColor = { 0.5f, 0.5f, 0.5f, 1.0f };
	quadModel.material.diffuseColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	quadModel.material.specColor = { 1.0f, 1.0f, 1.0f, 8.0f };  //w = specular Power
	
	//Directional Light settings
	cbufferperframe.dirLight.ambientColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	cbufferperframe.dirLight.diffuseColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	cbufferperframe.dirLight.specColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	cbufferperframe.dirLight.lightDir = {0.0f, 1.0f, -5.0f};

	//Point Light settings
	cbufferperframe.pointLight.ambientColor = { 0.0f, 0.8f, 0.0f, 1.0f };
	cbufferperframe.pointLight.diffuseColor = { 0.0f, 0.8f, 0.0f, 1.0f };
	cbufferperframe.pointLight.specColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	cbufferperframe.pointLight.lightPos = { 0.0f, 1.0f, -1.0f };
	cbufferperframe.pointLight.range = 10.0f;
	cbufferperframe.pointLight.att = { 0.0f, 1.0f, 0.0f };

	//Spot Light settings
	cbufferperframe.spotLight.ambientColor = { 0.0f, 0.0f, 0.8f, 1.0f };
	cbufferperframe.spotLight.diffuseColor = { 0.0f, 0.0f, 0.8f, 1.0f };
	cbufferperframe.spotLight.specColor = { 0.0f, 0.0f, 1.0f, 1.0f };
	cbufferperframe.spotLight.range = 10.0f;
	cbufferperframe.spotLight.att = { 0.0f, 1.0f, 0.0f };
	cbufferperframe.spotLight.spotPower = 8.0f;
}

//--------------------------------------
void InitD3DApp::setupModelTextureData()
{
	//-----------------------------------------------------//
	//-----------------------CUBE--------------------------//
	//-----------------------------------------------------//
	/*for (size_t i = 0; i < 120; i++)
	{
		std::wstring fileNumber;
		if (i < 9)
		{
			fileNumber = L"00" + std::to_wstring(i + 1);
		}
		else if (i < 99)
		{
			fileNumber = L"0" + std::to_wstring(i + 1);
		}
		else
		{
			fileNumber = std::to_wstring(i + 1);			
		}

		createShaderResourceViewFromImageFile(L"Images/FireAnim/Fire" + fileNumber + L".bmp", d3dDevice, immediateContext, texType::WIC, &cubeModel.texViews[i]);
	}*/
	createShaderResourceViewFromImageFile(L"Images/WireFence.dds", d3dDevice, d3dImmediateContext, texType::DDS, &cubeModel.texViews[0]);
	
	XMMATRIX mtexTransformMatrix = XMLoadFloat4x4(&cubeModel.texTransformMatrix);
	mtexTransformMatrix *= XMMatrixScaling(1.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&cubeModel.texTransformMatrix, mtexTransformMatrix);

	//-----------------------------------------------------//
	//-----------------------QUAD--------------------------//
	//-----------------------------------------------------//
	createShaderResourceViewFromImageFile(L"Images/transWindow.png", d3dDevice, d3dImmediateContext, texType::WIC, &quadModel.texViews[0]);	
}

//----------------------------------
void InitD3DApp::setupSamplerState()
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.Filter = D3D11_FILTER_ANISOTROPIC;  
	desc.MaxAnisotropy = 16;
	
	/*desc.BorderColor[0] = 1.0f;
	desc.BorderColor[1] = 0.0f;
	desc.BorderColor[2] = 0.0f;	
	desc.BorderColor[3] = 1.0f;*/

	ThrowIfFailed(d3dDevice->CreateSamplerState(&desc, cubeModel.sampler.GetAddressOf()));	
	ThrowIfFailed(d3dDevice->CreateSamplerState(&desc, quadModel.sampler.GetAddressOf()));	
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

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	ThrowIfFailed(d3dDevice->CreateBlendState(&blendDesc, blendState.GetAddressOf()))
}

//-------------------------
void InitD3DApp::onResize()
{
	d3dApp::onResize();

	//Calculate new Projection Matrix
	XMMATRIX mProjMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), aspectRatio(), 0.1f, 1000.0f);
	XMStoreFloat4x4(&fProjMatrix, mProjMatrix);
}

//-------------------------------------------
void InitD3DApp::updateScene(float deltaTime)
{
	//Calculate new View Matrix
	XMVECTOR camPos = XMLoadFloat4(&this->camPos);
	XMVECTOR camLookAt = XMLoadFloat3(&this->camLookAt);
	XMVECTORF32 worldUp{ 0.0f, 1.0f, 0.0f };

	XMMATRIX mViewMatrix = XMMatrixLookAtLH(camPos, camPos + camLookAt, worldUp);
	XMStoreFloat4x4(&fViewMatrix, mViewMatrix);

	/*cubeModel.uOffset += gameTimer.getDeltaTime() * 0.05f;
	cubeModel.vOffset += gameTimer.getDeltaTime() * 0.08f;
	XMMATRIX rotate = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, cubeModel.uOffset);
	XMStoreFloat4x4(&cubeModel.texTransformMatrix, rotate);*/
}

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
	
	//Set Constant Buffers
	d3dImmediateContext->VSSetConstantBuffers(0, 1, constantBufferPerObject.GetAddressOf());
	d3dImmediateContext->PSSetConstantBuffers(0, 1, constantBufferPerObject.GetAddressOf());
	d3dImmediateContext->PSSetConstantBuffers(1, 1, constantBufferPerFrame.GetAddressOf());

	//Set Input Layout
	d3dImmediateContext->IASetInputLayout(inputLayout.Get());

	//Set primitive topology
	d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//CBUFFER PER FRAME
	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	XMFLOAT3 spotLightPos = Float4ToFloat3(&getCameraPos());
	XMFLOAT3 spotLightDir = getCameraTarget();

	cbufferperframe.viewPos = getCameraPos();
//	cbufferperframe.spotLight.lightPos = spotLightPos;
	cbufferperframe.spotLight.lightDir = spotLightDir;

	ZeroMemory(&mappedSubResource, sizeof(mappedSubResource));
	d3dImmediateContext->Map(constantBufferPerFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
	memcpy(mappedSubResource.pData, &cbufferperframe, sizeof(cbufferPerFrame));
	d3dImmediateContext->Unmap(constantBufferPerFrame.Get(), 0);
	
	//Draw Cube
	//cubeModel.performInefficientAnimation(gameTimer);
	for (size_t i = 0; i < cubeTranslateVectors.size(); i++)
	{
		XMMATRIX translate = XMMatrixTranslation(cubeTranslateVectors[i].x, cubeTranslateVectors[i].y, cubeTranslateVectors[i].z);
		XMStoreFloat4x4(&cubeModel.worldMatrix, translate);
		cubeModel.clipAlpha = true;
		drawObjectIndexed(&cubeModel);
	}
	
	//Enable blending
	d3dImmediateContext->RSSetState(0);
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3dImmediateContext->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

	//Calculate drawing order of quads according to distance from camera
	for (size_t i = 0; i < quadTranslateVectors.size(); i++)
	{
		XMVECTOR viewPos = XMLoadFloat4(&cbufferperframe.viewPos);
		XMVECTOR quadPos = XMLoadFloat3(&quadTranslateVectors[i]);
		XMVECTOR diff = XMVectorSubtract(viewPos, quadPos);
		XMVECTOR xDistance = XMVector3Length(diff);

		XMFLOAT3 fDistance;
		XMStoreFloat3(&fDistance, xDistance);
		float distance = fDistance.x;   //All components hold the same length value

		quadDistancesFromCamera[distance] = quadTranslateVectors[i];
	}
	//Draw Quads in reverse order i.e. furthest quad from camera is drawn first (std::map is using ascending order)
	for (auto i = quadDistancesFromCamera.crbegin(); i != quadDistancesFromCamera.crend(); i++)
	{
		XMMATRIX translate = XMMatrixTranslation(i->second.x, i->second.y, i->second.z);
		XMStoreFloat4x4(&quadModel.worldMatrix, translate);
		drawObjectIndexed(&quadModel);
	}

	ThrowIfFailed(swapChain->Present(0, 0));
}

//----------------------------------------------------
void InitD3DApp::drawObjectIndexed(const Model* model)
{
	//==================//
	//CBUFFER PER OBJECT//
	//==================//

	//World View Projection Matrix
	XMMATRIX worldViewProj;
	XMMATRIX world = XMLoadFloat4x4(&model->worldMatrix);
	XMMATRIX view = XMLoadFloat4x4(&fViewMatrix);
	XMMATRIX proj = XMLoadFloat4x4(&fProjMatrix);
	worldViewProj = world * view * proj;
	//DirectXMath - Matrix(row major), HLSL - Matrix(Column Major) 
	//Therefore, transpose to a column matrix as then it will be a row matrix in hlsl
	worldViewProj = XMMatrixTranspose(worldViewProj);

	//World Inverse Transpose Matrix
	XMMATRIX worldInvTranspose;
	//No transpose as we have to store matrix in column format for it to be stored as row format in hlsl
	//worldInvTranspose = XMMatrixInverse(&XMMatrixDeterminant(world), world);

	XMMATRIX mTexTransformMatrix = XMLoadFloat4x4(&model->texTransformMatrix);

//	cbufferperobject.worldInvTranspose = worldInvTranspose;
	cbufferperobject.worldViewProj = worldViewProj;
	cbufferperobject.world = XMMatrixTranspose(world);
	cbufferperobject.material = model->material;
	cbufferperobject.useTexture = model->useTexture;
	cbufferperobject.clipAlpha = model->clipAlpha;
	cbufferperobject.texTransformMatrix = XMMatrixTranspose(mTexTransformMatrix);

	/*D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	ZeroMemory(&mappedSubResource, sizeof(mappedSubResource));
	d3dImmediateContext->Map(constantBufferPerObject.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource);
	memcpy(mappedSubResource.pData, &cbufferperobject, sizeof(cbufferPerObject));
	d3dImmediateContext->Unmap(constantBufferPerObject.Get(), 0);*/
	d3dImmediateContext->UpdateSubresource(constantBufferPerObject.Get(), 0, 0, &cbufferperobject, 0, 0);

	//=================================================//

	//Set Buffers
	d3dImmediateContext->IASetVertexBuffers(0, 1, model->vertexBuffer.GetAddressOf(), &model->stride, &model->offset);
	d3dImmediateContext->IASetIndexBuffer(model->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//Bind Textures
	d3dImmediateContext->PSSetShaderResources(0, 1, model->texViews[model->currentFrame].GetAddressOf());
	d3dImmediateContext->PSSetSamplers(0, 1, model->sampler.GetAddressOf());

	//Draw
	d3dImmediateContext->DrawIndexed(model->indexCount, model->startIndex, model->baseVertex);
}