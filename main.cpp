#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

IDXGISwapChain* swapchain;
ID3D11Device* dev;
ID3D11DeviceContext* devcon;
ID3D11RenderTargetView* backbuffer;

ID3D11Buffer* pVertexBuffer;
ID3D11Buffer* pIndexBuffer;
ID3D11VertexShader* pVS;
ID3D11PixelShader* pPs;
ID3D11VertexShader* pVertexShader;
ID3D11PixelShader* pPixelShader;
ID3D11InputLayout* pLayout;
ID3D11Buffer* matrixBuffer;
ID3D11Buffer* lightBuffer;

ID3D11RasterizerState* pRasterState;
void InitD3D(HWND hWnd);
void InitPipeline(void);
void InitGraphics(void);
void InitConstBuffer(void);
void InitRasterizerState(void);
void UpdateConstBuffer(void);
void RenderFrame(void);
void CleanD3D(void);
void CreateMyWindow(HWND& hWnd, HINSTANCE hInstance, int nCmdSHow);

class Camera {
public:
	Camera() {
		position = DirectX::XMVectorSet(1.0f, 1.0f, -5.0f, 1.0f);
		focusPoint = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		upDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	}

	void Move(DirectX::XMVECTOR delta) {
		position = DirectX::XMVectorAdd(position, delta);
		focusPoint = DirectX::XMVectorAdd(focusPoint, delta);
	}
	void Rotate(int deltaX, int deltaY) {
		float rotationSpeed = 0.005f;
		float pitch = deltaY * rotationSpeed;
		float yaw = deltaX * rotationSpeed;

		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0);

		// ????focusPoint
		DirectX::XMVECTOR currentDirection = DirectX::XMVectorSubtract(focusPoint, position);
		DirectX::XMVECTOR newDirection = DirectX::XMVector3Transform(currentDirection, rotationMatrix);
		focusPoint = DirectX::XMVectorAdd(position, newDirection);
	}

	void UpdateViewMatrix() {
		viewMatrix = DirectX::XMMatrixLookAtLH(position, focusPoint, upDirection);
	}

	DirectX::XMMATRIX GetViewMatrix() const {
		return viewMatrix;
	}

private:
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR focusPoint;
	DirectX::XMVECTOR upDirection;
	DirectX::XMMATRIX viewMatrix;
};

Camera camera;

struct MatrixBufferType
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
};
struct LightBufferType
{
	DirectX::XMVECTOR lightPosition;
	DirectX::XMVECTOR lightColor;
};
struct VertexType 
{
	float x, y, z;
	float nx, ny, nz;
};
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
		UpdateConstBuffer();
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
	InitRasterizerState();
	InitConstBuffer();

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

	D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	dev->CreateInputLayout(ied, 1, pVS->GetBufferPointer(), pVS->GetBufferSize(), &pLayout);
	devcon->IASetInputLayout(pLayout);

	pVS->Release();
	pPS->Release();
}
void InitGraphics(void) {
	VertexType vertices[] = {
		{-1.5, -0.5, -0.5,  0,  0, -1},
		{ 1.5, -0.5, -0.5,  0,  0, -1},
		{ 1.5,  0.5, -0.5,  0,  0, -1},
		//// Front face
		//{-0.5, -0.5, -0.5,  0,  0, -1},
		//{ 0.5, -0.5, -0.5,  0,  0, -1},
		//{ 0.5,  0.5, -0.5,  0,  0, -1},
		//{-0.5,  0.5, -0.5,  0,  0, -1},
		//// Back face
		//{-0.5, -0.5,  0.5,  0,  0,  1},
		//{ 0.5, -0.5,  0.5,  0,  0,  1},
		//{ 0.5,  0.5,  0.5,  0,  0,  1},
		//{-0.5,  0.5,  0.5,  0,  0,  1},
		//// Left face
		//{-0.5, -0.5, -0.5, -1,  0,  0},
		//{-0.5,  0.5, -0.5, -1,  0,  0},
		//{-0.5,  0.5,  0.5, -1,  0,  0},
		//{-0.5, -0.5,  0.5, -1,  0,  0},
		//// Right face
		//{ 0.5, -0.5, -0.5,  1,  0,  0},
		//{ 0.5,  0.5, -0.5,  1,  0,  0},
		//{ 0.5,  0.5,  0.5,  1,  0,  0},
		//{ 0.5, -0.5,  0.5,  1,  0,  0},
		//// Top face
		//{-0.5,  0.5, -0.5,  0,  1,  0},
		//{ 0.5,  0.5, -0.5,  0,  1,  0},
		//{ 0.5,  0.5,  0.5,  0,  1,  0},
		//{-0.5,  0.5,  0.5,  0,  1,  0},
		//// Bottom face
		//{-0.5, -0.5, -0.5,  0, -1,  0},
		//{ 0.5, -0.5, -0.5,  0, -1,  0},
		//{ 0.5, -0.5,  0.5,  0, -1,  0},
		//{-0.5, -0.5,  0.5,  0, -1,  0},
	};
	unsigned short indices[] = {
		2, 1, 0,
		//0,  1,  2,  0,  2,  3, // Front face
		//4,  5,  6,  4,  6,  7, // Back face
		//8,  9, 10,  8, 10, 11, // Left face
	 //  12, 13, 14, 12, 14, 15, // Right face
	 //  16, 17, 18, 16, 18, 19, // Top face
	 //  20, 21, 22, 20, 22, 23, // Bottom face
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


	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;

	ID3D11Buffer* pIndexBuffer;
	dev->CreateBuffer(&indexBufferDesc, &indexData, &pIndexBuffer);
}
void InitRasterizerState(void) {
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK; // D3D11_CULL_BACK /or/ D3D11_CULL_NONE 
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME; // D3D11_FILL_SOLID /or/ D3D11_FILL_WIREFRAME
	rasterDesc.FrontCounterClockwise = true; // set true to make clockwise is front
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	dev->CreateRasterizerState(&rasterDesc, &pRasterState);
	devcon->RSSetState(pRasterState);
}
void InitConstBuffer(void) {
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
void UpdateConstBuffer(void) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;

	devcon->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, 800 / (FLOAT)600, 0.1f, 100.0f);
	dataPtr->world = DirectX::XMMatrixTranspose(world);
	dataPtr->view = DirectX::XMMatrixTranspose(camera.GetViewMatrix());
	dataPtr->projection = DirectX::XMMatrixTranspose(projection);
	devcon->Unmap(matrixBuffer, 0);
	devcon->VSSetConstantBuffers(0, 1, &matrixBuffer);

	devcon->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr2 = (LightBufferType*)mappedResource.pData;
	dataPtr2->lightPosition = DirectX::XMVectorSet(2.0f, 2.0f, 0.0f, 1.0f);
	dataPtr2->lightColor = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	devcon->Unmap(lightBuffer, 0);
	devcon->PSSetConstantBuffers(0, 1, &lightBuffer);
}
void RenderFrame(void) {
	float color[4] = { 0.0f, 0.9f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, color);

	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	devcon->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devcon->Draw(36, 0);

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
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_KEYDOWN: {
			switch (wParam) {
				case 'W':
					camera.Move(DirectX::XMVectorSet(0.0f, 0.0f, 0.1f, 0.0f));
					break;
				case 'S':
					camera.Move(DirectX::XMVectorSet(0.0f, 0.0f, -0.1f, 0.0f));
					break;
				case 'A':
					camera.Move(DirectX::XMVectorSet(-0.1f, 0.0f, 0.0f, 0.0f));
					break;
				case 'D':
					camera.Move(DirectX::XMVectorSet(0.1f, 0.0f, 0.0f, 0.0f));
					break;
				}
				camera.UpdateViewMatrix();  // update view matrix
				break;
		}

		case WM_MOUSEMOVE: {
			static POINTS lastMousePos = { 0, 0 };  // ????????????????

			POINTS currentMousePos = MAKEPOINTS(lParam);  // ??????
			int deltaX = currentMousePos.x - lastMousePos.x;  // ??X?????
			int deltaY = currentMousePos.y - lastMousePos.y;  // ??Y?????

			// ????delta????????
			camera.Rotate(deltaX, deltaY);  // ??????Rotate??????????
			camera.UpdateViewMatrix();
			lastMousePos = currentMousePos;  // ??????????
			break;
		}
				   // deal with other message...
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CreateMyWindow(HWND& hWnd, HINSTANCE hInstance, int nCmdSHow){
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"WindowClass";

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(0, L"WindowClass", L"Window", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);
}