#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <WICTextureLoader.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")
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
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texCoord;		//UV texture coordinates
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 bitangent;
	//float x, y, z;			//Position
	//float nx, ny, nz;		//Normal
	//float u, v;				//UV texture coordinates
	//float tx, ty, tz;		//Tangent
	//float btx, bty, btz;	//Bitangent
};
void InitD3D(HWND& hWnd, ID3D11Device*& dev, ID3D11DeviceContext*& devcon, IDXGISwapChain*& swapchain, ID3D11RenderTargetView*& backbuffer, ID3D11VertexShader*& pVertexShader, ID3D11PixelShader*& pPixelShader, ID3D11InputLayout*& pLayout, ID3D11Buffer*& pVertexBuffer, ID3D11Buffer*& pIndexBuffer, ID3D11RasterizerState*& pRasterState, ID3D11Buffer*& matrixBuffer, ID3D11Buffer*& lightBuffer, ID3D11ShaderResourceView*& texture1, ID3D11SamplerState*& samplerState1, ID3D11ShaderResourceView*& texture2, ID3D11SamplerState*& samplerState2);
void InitPipeline(ID3D11Device*& dev, ID3D11DeviceContext*& devcon, ID3D11VertexShader*& pVertexShader, ID3D11PixelShader*& pPixelShader, ID3D11InputLayout*& pLayout);
void InitGraphics(ID3D11Device*& dev, ID3D11Buffer*& pVertexBuffer, ID3D11Buffer*& pIndexBuffer);
void InitConstBuffer(ID3D11Device*& dev, ID3D11Buffer*& matrixBuffer, ID3D11Buffer*& lightBuffer);
void InitRasterizerState(ID3D11Device*& dev, ID3D11DeviceContext*& devcon, ID3D11RasterizerState*& pRasterState);
void InitTextureSource(ID3D11Device*& dev, ID3D11DeviceContext*& devcon, ID3D11ShaderResourceView*& texture1, ID3D11ShaderResourceView*& texture2);
void InitTextureSampler(ID3D11Device*& dev, ID3D11SamplerState*& samplerState1, ID3D11SamplerState*& samplerState2);
void BindTextureAndSampler(ID3D11DeviceContext*& devcon, ID3D11ShaderResourceView*& texture1, ID3D11SamplerState*& samplerState1, ID3D11ShaderResourceView*& texture2, ID3D11SamplerState*& samplerState2);
