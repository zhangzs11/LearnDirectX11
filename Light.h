#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
class Light {
public:
	Light() {
		lightPosition = DirectX::XMVectorSet(0.0f, 0.0f, 2.0f, 1.0f);
		lightColor = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		UpdateViewMatrix();
	}
	void CreateCubeTextureAndView(UINT width, UINT height, ID3D11Device*& device);
	void UpdateViewMatrix();


	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	DirectX::XMVECTOR lightPosition = DirectX::XMVectorSet(0.0f, 0.0f, 2.0f, 1.0f);
	DirectX::XMVECTOR lightColor;
	DirectX::XMMATRIX viewMatrix[6];

	ID3D11Texture2D* depthCubeTexture = nullptr;
	ID3D11DepthStencilView* depthStencilView[6];
	ID3D11ShaderResourceView* shaderResourceView = nullptr;
};
/*
DirectX::XMVECTOR getLightPosition() { return lightPosition; }
DirectX::XMVECTOR getLightColor() { return lightColor; }
ID3D11Texture2D* getDepthCubeTexture() { return depthCubeTexture; }
ID3D11DepthStencilView* getDepthStencilView() { return depthStencilView; }
ID3D11ShaderResourceView* getShaderResourceView() { return shaderResourceView; }
D3D11_DEPTH_STENCIL_VIEW_DESC getDsvDesc() { return dsvDesc; }
D3D11_SHADER_RESOURCE_VIEW_DESC getSrvDesc() { return srvDesc; }
*/