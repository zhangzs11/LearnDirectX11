#include "InitD3D.h"
void InitD3D(HWND& hWnd, ID3D11Device*& dev, ID3D11DeviceContext*& devcon, IDXGISwapChain*& swapchain, ID3D11RenderTargetView*& backbuffer,ID3D11Buffer*& pVertexBuffer, ID3D11Buffer*& pIndexBuffer, ID3D11RasterizerState*& pRasterState, ID3D11Buffer*& matrixBuffer, ID3D11Buffer*& lightBuffer, const std::vector<std::wstring>& texturepaths, std::vector<ID3D11ShaderResourceView*>& textures, std::vector<ID3D11SamplerState*>& samplers) {
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;					//scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD / DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 4;
	scd.Windowed = TRUE;

	HRESULT hrCD = D3D11CreateDeviceAndSwapChain(
		NULL, 
		D3D_DRIVER_TYPE_HARDWARE, 
		NULL, 
		D3D11_CREATE_DEVICE_DEBUG,
		NULL, 
		NULL,
		D3D11_SDK_VERSION, 
		&scd, 
		&swapchain,
		&dev,
		NULL, 
		&devcon
	);
	if (FAILED(hrCD)) {
		OutputDebugStringA("Failed to create device.\n");
		return;
	}

	ID3D11Texture2D* pBackBuffer = nullptr;
	HRESULT hrGB = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hrGB) || !pBackBuffer) {
		// Handle error here. For example, log the error and then return or throw an exception.
		OutputDebugStringA("Failed to get the back buffer.\n");
		return;  // or you might handle it differently, depending on your error-handling strategy
	}
	hrGB = dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	if (FAILED(hrGB)) {
		// Handle error here. 
		OutputDebugStringA("Failed to create render target view.\n");
		pBackBuffer->Release();
		return;  // or handle error differently
	}
	pBackBuffer->Release();
	devcon->OMSetRenderTargets(1, &backbuffer, NULL);

	// Set the viewport here
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 800;
	viewport.Height = 600;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	devcon->RSSetViewports(1, &viewport);  // Set the viewport

	InitGraphics(dev, pVertexBuffer, pIndexBuffer);
	InitRasterizerState(dev, devcon, pRasterState);
	InitTextureSource(dev, devcon, texturepaths, textures);
	InitTextureSampler(dev, textures.size(), samplers);
	BindTextureAndSampler(devcon, textures, samplers);
	InitConstBuffer(dev, matrixBuffer, lightBuffer);
}
void InitGraphics(ID3D11Device*& dev, ID3D11Buffer*& pVertexBuffer, ID3D11Buffer*& pIndexBuffer) {
	VertexType vertices[] = {
		//FONRT
		{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f,  0.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f,  0.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(0.0f,  0.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(0.0f,  0.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		//RARE
		{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), DirectX::XMFLOAT3(0.0f,  0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), DirectX::XMFLOAT3(0.0f,  0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f,  0.5f, 0.5f), DirectX::XMFLOAT3(0.0f,  0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(-0.5f,  0.5f, 0.5f), DirectX::XMFLOAT3(0.0f,  0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		//TOP
		{DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), DirectX::XMFLOAT3(0.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), DirectX::XMFLOAT3(0.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f,  0.5f, 0.5f), DirectX::XMFLOAT3(0.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(-0.5f,  0.5f, 0.5f), DirectX::XMFLOAT3(0.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		//BOTTOM
		{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f,  -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f,  -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f,  -0.5f, 0.5f), DirectX::XMFLOAT3(0.0f,  -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(-0.5f,  -0.5f, 0.5f), DirectX::XMFLOAT3(0.0f,  -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		//LEFT
		{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(-1.0f,  0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), DirectX::XMFLOAT3(-1.0f,  0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(-0.5f,  0.5f, 0.5f), DirectX::XMFLOAT3(-1.0f,  0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(-1.0f,  0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		//RIGHT
		{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(1.0f,  0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), DirectX::XMFLOAT3(1.0f,  0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f,  0.5f, 0.5f), DirectX::XMFLOAT3(1.0f,  0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
		{DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT3(1.0f,  0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f),DirectX::XMFLOAT3(0.0f,  0.0f, 0.0f)},
	};
	unsigned short indices[] = {
		0,  1,  2,
		0,  2,  3,

		6,  5,  4,
		7,  6,  4,

		8,  9,  10,
		8,  10, 11,

		14, 13, 12,
		15, 14, 12,

		18, 17, 16,
		19, 18, 16,

		20, 21, 22,
		20, 22, 23,
	};
	for (int i = 0; i < sizeof(indices) / sizeof(unsigned short); i += 3) {
		VertexType& v0 = vertices[indices[i]];
		VertexType& v1 = vertices[indices[i + 1]];
		VertexType& v2 = vertices[indices[i + 2]];

		DirectX::XMVECTOR edge1 = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&v1.position), DirectX::XMLoadFloat3(&v0.position));
		DirectX::XMVECTOR edge2 = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&v2.position), DirectX::XMLoadFloat3(&v0.position));

		DirectX::XMVECTOR deltaUV1 = DirectX::XMVectorSubtract(DirectX::XMLoadFloat2(&v1.texCoord), DirectX::XMLoadFloat2(&v0.texCoord));
		DirectX::XMVECTOR deltaUV2 = DirectX::XMVectorSubtract(DirectX::XMLoadFloat2(&v2.texCoord), DirectX::XMLoadFloat2(&v0.texCoord));

		float f = 1.0f / (DirectX::XMVectorGetX(deltaUV1) * DirectX::XMVectorGetY(deltaUV2) - DirectX::XMVectorGetX(deltaUV2) * DirectX::XMVectorGetY(deltaUV1));

		DirectX::XMVECTOR tangent = DirectX::XMVectorZero();;
		tangent = DirectX::XMVectorSetX(tangent, f * (DirectX::XMVectorGetY(deltaUV2) * DirectX::XMVectorGetX(edge1) - DirectX::XMVectorGetY(deltaUV1) * DirectX::XMVectorGetX(edge2)));
		tangent = DirectX::XMVectorSetY(tangent, f * (DirectX::XMVectorGetY(deltaUV2) * DirectX::XMVectorGetY(edge1) - DirectX::XMVectorGetY(deltaUV1) * DirectX::XMVectorGetY(edge2)));
		tangent = DirectX::XMVectorSetZ(tangent, f * (DirectX::XMVectorGetY(deltaUV2) * DirectX::XMVectorGetZ(edge1) - DirectX::XMVectorGetY(deltaUV1) * DirectX::XMVectorGetZ(edge2)));
		tangent = DirectX::XMVector3Normalize(tangent);

		DirectX::XMVECTOR bitangent = DirectX::XMVectorZero();;
		bitangent = DirectX::XMVectorSetX(bitangent, f * (-DirectX::XMVectorGetX(deltaUV2) * DirectX::XMVectorGetX(edge1) + DirectX::XMVectorGetX(deltaUV1) * DirectX::XMVectorGetX(edge2)));
		bitangent = DirectX::XMVectorSetY(bitangent, f * (-DirectX::XMVectorGetX(deltaUV2) * DirectX::XMVectorGetY(edge1) + DirectX::XMVectorGetX(deltaUV1) * DirectX::XMVectorGetY(edge2)));
		bitangent = DirectX::XMVectorSetZ(bitangent, f * (-DirectX::XMVectorGetX(deltaUV2) * DirectX::XMVectorGetZ(edge1) + DirectX::XMVectorGetX(deltaUV1) * DirectX::XMVectorGetZ(edge2)));
		bitangent = DirectX::XMVector3Normalize(bitangent);

		DirectX::XMStoreFloat3(&v0.tangent, tangent);
		DirectX::XMStoreFloat3(&v1.tangent, tangent);
		DirectX::XMStoreFloat3(&v2.tangent, tangent);
		DirectX::XMStoreFloat3(&v0.bitangent, bitangent);
		DirectX::XMStoreFloat3(&v1.bitangent, bitangent);
		DirectX::XMStoreFloat3(&v2.bitangent, bitangent);
	}
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices;

	dev->CreateBuffer(&bd, &vertexData, &pVertexBuffer);

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;

	dev->CreateBuffer(&indexBufferDesc, &indexData, &pIndexBuffer);
}
void InitRasterizerState(ID3D11Device*& dev, ID3D11DeviceContext*& devcon, ID3D11RasterizerState*& pRasterState) {
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK; // D3D11_CULL_BACK /or/ D3D11_CULL_NONE 
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_SOLID /or/ D3D11_FILL_WIREFRAME
	rasterDesc.FrontCounterClockwise = true; // set true to make counterclockwise is front
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	dev->CreateRasterizerState(&rasterDesc, &pRasterState);
	devcon->RSSetState(pRasterState);
}
void InitConstBuffer(ID3D11Device*& dev, ID3D11Buffer*& matrixBuffer, ID3D11Buffer*& lightBuffer) {
	D3D11_BUFFER_DESC matrixBufferDesc;
	ZeroMemory(&matrixBufferDesc, sizeof(D3D11_BUFFER_DESC));
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dev->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	D3D11_BUFFER_DESC lightBufferDesc;
	ZeroMemory(&lightBufferDesc, sizeof(D3D11_BUFFER_DESC));
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dev->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
}
void InitTextureSource(ID3D11Device*& dev, ID3D11DeviceContext*& devcon, const std::vector<std::wstring>& texturepaths, std::vector<ID3D11ShaderResourceView*>& textures) {
	for (const auto& path : texturepaths) {
		ID3D11ShaderResourceView* tempTexture;
		HRESULT hr = DirectX::CreateWICTextureFromFile(dev, devcon, path.c_str(), nullptr, &tempTexture, 0);
		if (SUCCEEDED(hr)) {
			textures.push_back(tempTexture);
		}
		else if (FAILED(hr)) {
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPWSTR)&lpMsgBuf,
				0,
				NULL
			);
			OutputDebugStringA("Failed to load texture.\n");
			OutputDebugString((LPWSTR)lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
	}
}
void InitTextureSampler(ID3D11Device*& dev, size_t count, std::vector<ID3D11SamplerState*>& samplers) {
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	for (size_t i = 0; i < count; i++) {
		ID3D11SamplerState* sampler;
		if (SUCCEEDED(dev->CreateSamplerState(&sampDesc, &sampler))) {
			samplers.push_back(sampler);
		}
		else
		{
			OutputDebugStringA("Failed to create Texture Sampler.\n");
		}

	}
}
void BindTextureAndSampler(ID3D11DeviceContext*& devcon, const std::vector<ID3D11ShaderResourceView*>& textures, const std::vector<ID3D11SamplerState*>& samplers) {
	if (textures.size() == samplers.size()) {
		for (size_t i = 0; i < textures.size(); i++) {
			devcon->PSSetShaderResources(i, 1, &textures[i]);
			devcon->PSSetSamplers(i, 1, &samplers[i]);
		}
	}
	else
	{
		OutputDebugStringA("The number of textures with sammpler are mismatch.\n");
	}
}
HRESULT CompileAndCreateShader(
	ID3D11Device*& dev,
	const WCHAR* vsFile,
	const WCHAR* psFile,
	D3D11_INPUT_ELEMENT_DESC* inputElementDescs,
	UINT numElements,
	Shader& outShader)
{
	ID3DBlob* pErrorBlob = nullptr;

	// Compile Vertex Shader
	HRESULT hrVS = D3DCompileFromFile(vsFile, 0, 0, "main", "vs_4_0", 0, 0, &outShader.pVS, &pErrorBlob);
	if (FAILED(hrVS)) {
		if (pErrorBlob) {
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		return hrVS; // Return the error code
	}

	// Compile Pixel Shader
	HRESULT hrPS = D3DCompileFromFile(psFile, 0, 0, "main", "ps_4_0", 0, 0, &outShader.pPS, &pErrorBlob);
	if (FAILED(hrPS)) {
		if (pErrorBlob) {
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		return hrPS; // Return the error code
	}

	// Create Vertex Shader from compiled blob
	HRESULT hrCreateVS = dev->CreateVertexShader(outShader.pVS->GetBufferPointer(), outShader.pVS->GetBufferSize(), NULL, &outShader.pVertexShader);
	if (FAILED(hrCreateVS)) {
		return hrCreateVS;
	}

	// Create Pixel Shader from compiled blob
	HRESULT hrCreatePS = dev->CreatePixelShader(outShader.pPS->GetBufferPointer(), outShader.pPS->GetBufferSize(), NULL, &outShader.pPixelShader);
	if (FAILED(hrCreatePS)) {
		return hrCreatePS;
	}

	HRESULT hrCreateIL = dev->CreateInputLayout(inputElementDescs, numElements, outShader.pVS->GetBufferPointer(), outShader.pVS->GetBufferSize(), &outShader.pLayout);
	if (FAILED(hrCreateIL)) {
		return hrCreateIL;
	}

	return S_OK; // Return success code
}
void BindShader(ID3D11DeviceContext*& devcon, const Shader& shader) {
	devcon->VSSetShader(shader.pVertexShader, 0, 0);
	devcon->PSSetShader(shader.pPixelShader, 0, 0);
	devcon->IASetInputLayout(shader.pLayout);
}