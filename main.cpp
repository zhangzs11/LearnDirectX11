#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include "Camera.h"
#include "InitD3D.h"
#include "Light.h"
#include "Scene.h"
IDXGISwapChain* swapchain;
ID3D11Device* dev;
ID3D11DeviceContext* devcon;
ID3D11RenderTargetView* backbuffer;

ID3D11Buffer* pVertexBuffer;
ID3D11Buffer* pIndexBuffer;
ID3D11Buffer* matrixBuffer;
ID3D11Buffer* lightBuffer;

std::vector<std::wstring> texturePaths;
std::vector<ID3D11ShaderResourceView*> textures;
std::vector<ID3D11SamplerState*> samplers;
ID3D11ShaderResourceView* texture;
ID3D11SamplerState* samplerState;
ID3D11ShaderResourceView* normaltexture;
ID3D11SamplerState* normalsamplerState;

ID3D11RasterizerState* pRasterState;

Camera camera;
Shader mainSceneShader;
Shader shadowMapShader;
Light light;

D3D11_VIEWPORT viewport;
D3D11_INPUT_ELEMENT_DESC ied[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
};
D3D11_INPUT_ELEMENT_DESC shadowLayout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
void SetTexturePathes(void);
void UpdateConstBuffer(void);
void RenderFrame(void);
void CleanD3D(void);
void CreateMyWindow(HWND& hWnd, HINSTANCE hInstance, int nCmdSHow);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdSHow) {
	CoInitialize(NULL);

	HWND hWnd;
	CreateMyWindow(hWnd, hInstance, nCmdSHow);
	SetTexturePathes();
	InitD3D(hWnd, dev, devcon, swapchain, backbuffer, pVertexBuffer, pIndexBuffer, pRasterState, matrixBuffer, lightBuffer, texturePaths, textures, samplers);
	CompileAndCreateShader(dev, L"VertexShader.hlsl", L"PixelShader.hlsl", ied, sizeof(ied) / sizeof(ied[0]), mainSceneShader);
	CompileAndCreateShader(dev, L"ShadowMapVertexShader.hlsl", L"ShadowMapPixelShader.hlsl", shadowLayout, sizeof(shadowLayout) / sizeof(shadowLayout[0]), shadowMapShader);
	light.CreateCubeTextureAndView(1024, 1024, dev);
	//BindShader(devcon, mainSceneShader);
	ShowWindow(hWnd, nCmdSHow);
	MSG msg;
	while (TRUE) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				//CoUninitialize();
				break;
		}
		UpdateConstBuffer();
		RenderFrame();
	}

	CleanD3D();

	return static_cast<int>(msg.wParam);
}
void SetTexturePathes(void) {
	texturePaths = { L"resource/brickwall.jpg", L"resource/brickwall_normal.jpg" };
}
void UpdateConstBuffer(void) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;

	BindShader(devcon, mainSceneShader);
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
	//dataPtr2->lightPosition = DirectX::XMVectorSet(2.0f, 2.0f, 0.0f, 1.0f);
	//dataPtr2->lightColor = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	dataPtr2->lightPosition = light.lightPosition;
	dataPtr2->lightColor = light.lightColor;
	devcon->Unmap(lightBuffer, 0);
	devcon->PSSetConstantBuffers(1, 1, &lightBuffer);
}
void RenderFrame(void) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 1024;
	viewport.Height = 1024;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	MatrixBufferType* dataPtr;
	devcon->RSSetViewports(1, &viewport);  // Set the viewport
	BindShader(devcon, shadowMapShader);
	for (int i=0; i<6; i++) {
		devcon->ClearDepthStencilView(light.depthStencilView[i], D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	for (int i = 0; i < 6; ++i) {
		// Set the render target to the current face and clear it
		devcon->OMSetRenderTargets(0, nullptr, light.depthStencilView[i]);

		devcon->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		dataPtr = (MatrixBufferType*)mappedResource.pData;
		DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, 800 / (FLOAT)600, 0.1f, 100.0f);
		dataPtr->world = DirectX::XMMatrixTranspose(world);
		dataPtr->view = DirectX::XMMatrixTranspose(light.viewMatrix[i]);
		dataPtr->projection = DirectX::XMMatrixTranspose(projection);
		devcon->Unmap(matrixBuffer, 0);
		devcon->VSSetConstantBuffers(2, 1, &matrixBuffer);


		UINT stride = sizeof(VertexType);
		UINT offset = 0;
		devcon->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
		devcon->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		devcon->DrawIndexed(42, 0, 0);
		// Set up your camera with 'lightPosition', 'directions[i]', and 'ups[i]'

		// Render your scene from this point of view
	}
	//depthStencilViewForThisFace->Release();//Important!

	//render the main scene
	UpdateConstBuffer();
	devcon->OMSetRenderTargets(1, &backbuffer, NULL);
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 800;
	viewport.Height = 600;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	devcon->RSSetViewports(1, &viewport);  // Set the viewport
	BindShader(devcon, mainSceneShader);
	float color[4] = { 0.0f, 0.9f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, color);

	UINT stride = sizeof(VertexType);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	devcon->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devcon->DrawIndexed(42, 0, 0);

	swapchain->Present(0, 0);
}

void CleanD3D(void) {
	swapchain->Release();
	backbuffer->Release();
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
					camera.Move(DirectX::XMVector4Normalize(camera.GetForwardDirection()));
					break;
				case 'S':
					camera.Move(DirectX::XMVector4Normalize(DirectX::XMVectorNegate(camera.GetForwardDirection())));
					break;
				case 'A':;
					camera.Move(DirectX::XMVector4Normalize(DirectX::XMVectorNegate(camera.GetLeftDirection())));
					break;
				case 'D':
					camera.Move(DirectX::XMVector4Normalize(camera.GetLeftDirection()));
					break;
				}
				camera.UpdateViewMatrix();  // update view matrix
				break;
		}

		case WM_MOUSEMOVE: {
			static POINTS lastMousePos = { 0, 0 };  

			POINTS currentMousePos = MAKEPOINTS(lParam);  
			int deltaX = currentMousePos.x - lastMousePos.x;  
			int deltaY = currentMousePos.y - lastMousePos.y;  

			camera.Rotate(deltaX, deltaY);  
			camera.UpdateViewMatrix();
			lastMousePos = currentMousePos;  
			break;
		}
				   
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