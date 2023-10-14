#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

#include "Camera.h"
#include "InitD3D.h"

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

ID3D11ShaderResourceView* texture;
ID3D11SamplerState* samplerState;
ID3D11ShaderResourceView* normaltexture;
ID3D11SamplerState* normalsamplerState;

ID3D11RasterizerState* pRasterState;

Camera camera;

void UpdateConstBuffer(void);
void RenderFrame(void);
void CleanD3D(void);
void CreateMyWindow(HWND& hWnd, HINSTANCE hInstance, int nCmdSHow);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdSHow) {
	CoInitialize(NULL);

	HWND hWnd;
	CreateMyWindow(hWnd, hInstance, nCmdSHow);

	InitD3D(hWnd, dev, devcon, swapchain, backbuffer, pVertexShader, pPixelShader, pLayout, pVertexBuffer, pIndexBuffer, pRasterState, matrixBuffer, lightBuffer, texture, samplerState, normaltexture, normalsamplerState);

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
	devcon->PSSetConstantBuffers(1, 1, &lightBuffer);
}
void RenderFrame(void) {
	float color[4] = { 0.0f, 0.9f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, color);

	UINT stride = sizeof(VertexType);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	devcon->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devcon->DrawIndexed(36, 0, 0);

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