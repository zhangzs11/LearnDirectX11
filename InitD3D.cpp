#include "InitD3D.h"
void InitD3D(HWND& hWnd, ID3D11Device*& dev, ID3D11DeviceContext*& devcon, IDXGISwapChain*& swapchain, ID3D11RenderTargetView*& backbuffer, ID3D11VertexShader*& pVertexShader, ID3D11PixelShader*& pPixelShader, ID3D11InputLayout*& pLayout, ID3D11Buffer*& pVertexBuffer, ID3D11Buffer*& pIndexBuffer, ID3D11RasterizerState*& pRasterState, ID3D11Buffer*& matrixBuffer, ID3D11Buffer*& lightBuffer) {
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 4;
	scd.Windowed = TRUE;

	HRESULT hrCD = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &scd, &swapchain, &dev, NULL, &devcon);
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

	InitPipeline(dev, devcon, pVertexShader, pPixelShader, pLayout);
	InitGraphics(dev, pVertexBuffer, pIndexBuffer);
	InitRasterizerState(dev, devcon, pRasterState);
	InitConstBuffer(dev, matrixBuffer, lightBuffer);

}
void InitPipeline(ID3D11Device*& dev, ID3D11DeviceContext*& devcon, ID3D11VertexShader*& pVertexShader, ID3D11PixelShader*& pPixelShader, ID3D11InputLayout*& pLayout) {
	ID3DBlob* pVS, * pPS, * pErrorBlob;
	HRESULT hrVS = D3DCompileFromFile(L"VertexShader.hlsl", 0, 0, "main", "vs_4_0", 0, 0, &pVS, &pErrorBlob);
	if (FAILED(hrVS)) {
		if (pErrorBlob) {
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			OutputDebugStringA("Failed to compileVS.\n");
			pErrorBlob->Release();
		}
		// Handle error
	}
	HRESULT hrPS = D3DCompileFromFile(L"PixelShader.hlsl", 0, 0, "main", "ps_4_0", 0, 0, &pPS, &pErrorBlob);
	if (FAILED(hrPS)) {
		if (pErrorBlob) {
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			OutputDebugStringA("Failed to compilePS.\n");
			pErrorBlob->Release();
		}
		// Handle error
	}

	dev->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), NULL, &pVertexShader);
	dev->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), NULL, &pPixelShader);

	devcon->VSSetShader(pVertexShader, 0, 0);
	devcon->PSSetShader(pPixelShader, 0, 0);

	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	dev->CreateInputLayout(ied, 1, pVS->GetBufferPointer(), pVS->GetBufferSize(), &pLayout);
	devcon->IASetInputLayout(pLayout);

	pVS->Release();
	pPS->Release();
}
void InitGraphics(ID3D11Device*& dev, ID3D11Buffer*& pVertexBuffer, ID3D11Buffer*& pIndexBuffer) {
	VertexType vertices[] = {
		{-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f},//FONRT
		{ 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f},
		{ 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f},
		{-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f},

		{-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f},//RARE
		{ 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f},
		{ 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f},
		{-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f},

		{-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f},//TOP
		{ 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f},
		{ 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f},
		{-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f},

		{-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f},//BOTTOM
		{ 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f},
		{ 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f},
		{-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f},

		{-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f},//LEFT
		{-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f},
		{-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f},
		{-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f},

		{ 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f},//RIGHT
		{ 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f},
		{ 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f},
		{ 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f},
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