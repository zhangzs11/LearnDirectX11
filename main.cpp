#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

IDXGISwapChain* swapchain;
ID3D11Device* dev;
ID3D11DeviceContext* devcon;
ID3D11RenderTargetView* backbuffer;

ID3D11Buffer* pVertexBuffer;
ID3D11VertexShader* pVS;
ID3D11PixelShader* pPs;
ID3D11VertexShader* pVertexShader;
ID3D11PixelShader* pPixelShader;
ID3D11InputLayout* pLayout;

void InitD3D(HWND hWnd);
void InitPipeline(void);
void InitGraphics(void);

void RenderFrame(void);
void CleanD3D(void);
void CreateMyWindow(HWND& hWnd, HINSTANCE hInstance, int nCmdSHow);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdSHow) {
	HWND hWnd;
	CreateMyWindow(hWnd, hInstance, nCmdSHow);

	InitD3D(hWnd);

	ShowWindow(hWnd, nCmdSHow);

	MSG msg;
	while (TRUE) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				break;
		}

		RenderFrame();
	}

	CleanD3D();

	return static_cast<int>(msg.wParam);
}

void InitD3D(HWND hWnd) {
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

	InitPipeline();
	InitGraphics();
}
void InitPipeline(void) {
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

	D3D11_INPUT_ELEMENT_DESC ied[] = { //describe the struct of vertex datas which will input
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	dev->CreateInputLayout(ied, 1, pVS->GetBufferPointer(), pVS->GetBufferSize(), &pLayout);
	devcon->IASetInputLayout(pLayout);

	pVS->Release();
	pPS->Release();
}
void InitGraphics(void) {
	float vertices[] = {
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
	};


	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	dev->CreateBuffer(&bd, NULL, &pVertexBuffer);

	D3D11_MAPPED_SUBRESOURCE ms;
	devcon->Map(pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms); //start write pVertexBuffer,discard the old data and cover
	memcpy(ms.pData, vertices, sizeof(vertices));
	devcon->Unmap(pVertexBuffer, NULL);//stop write so that GPU can read the buffer

	//D3D11_BUFFER_DESC bd;
	//ZeroMemory(&bd, sizeof(bd));

	//bd.Usage = D3D11_USAGE_DEFAULT; // Change this
	//bd.ByteWidth = sizeof(vertices);
	//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	//D3D11_SUBRESOURCE_DATA InitData;
	//ZeroMemory(&InitData, sizeof(InitData));
	//InitData.pSysMem = vertices;

	//dev->CreateBuffer(&bd, &InitData, &pVertexBuffer); // Provide initial data here

}
void RenderFrame(void) {
	float color[4] = { 0.0f, 0.9f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, color);

	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devcon->Draw(3, 0);

	swapchain->Present(0, 0);
}

void CleanD3D(void) {
	swapchain->Release();
	backbuffer->Release();
	pVertexShader->Release();
	pPixelShader->Release();
	dev->Release();
	devcon->Release();
}
void CreateMyWindow(HWND& hWnd, HINSTANCE hInstance, int nCmdSHow){
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(0, L"WindowClass", L"Window", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);
}