#pragma once
#include <windows.h>
#include <vector>
#include <string>
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
struct Shader {
	ID3D11VertexShader* pVertexShader = nullptr;
	ID3D11PixelShader* pPixelShader = nullptr;
	ID3D11InputLayout* pLayout = nullptr;
	ID3DBlob* pVS = nullptr;
	ID3DBlob* pPS = nullptr;
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
void InitD3D(HWND& hWnd, ID3D11Device*& dev, ID3D11DeviceContext*& devcon, IDXGISwapChain*& swapchain, ID3D11RenderTargetView*& backbuffer, ID3D11Buffer*& pVertexBuffer, ID3D11Buffer*& pIndexBuffer, ID3D11RasterizerState*& pRasterState, ID3D11Buffer*& matrixBuffer, ID3D11Buffer*& lightBuffer, const std::vector<std::wstring>& texturepaths, std::vector<ID3D11ShaderResourceView*>& textures, std::vector<ID3D11SamplerState*>& samplers);
void InitGraphics(ID3D11Device*& dev, ID3D11Buffer*& pVertexBuffer, ID3D11Buffer*& pIndexBuffer);
void InitConstBuffer(ID3D11Device*& dev, ID3D11Buffer*& matrixBuffer, ID3D11Buffer*& lightBuffer);
void InitRasterizerState(ID3D11Device*& dev, ID3D11DeviceContext*& devcon, ID3D11RasterizerState*& pRasterState);
void InitTextureSource(ID3D11Device*& dev, ID3D11DeviceContext*& devcon, const std::vector<std::wstring>& texturepaths, std::vector<ID3D11ShaderResourceView*>& textures);
void InitTextureSampler(ID3D11Device*& dev, size_t count, std::vector<ID3D11SamplerState*>& samplers);
void BindTextureAndSampler(ID3D11DeviceContext*& devcon, const std::vector<ID3D11ShaderResourceView*>& textures, const std::vector<ID3D11SamplerState*>& samplers);
HRESULT CompileAndCreateShader(ID3D11Device*& dev, const WCHAR* vsFile, const WCHAR* psFile, D3D11_INPUT_ELEMENT_DESC* inputElementDescs, UINT numElements, Shader& outShader);
void BindShader(ID3D11DeviceContext*& devcon, const Shader& shader);
