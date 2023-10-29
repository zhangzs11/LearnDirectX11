#include"Light.h"
void Light::CreateCubeTextureAndView(UINT width, UINT height, ID3D11Device*& device) {
	// Create deothCubeTexture
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;  // 6 faces for a cube
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	//ID3D11Texture2D* depthCubeTexture = nullptr;
	device->CreateTexture2D(&texDesc, nullptr, &depthCubeTexture);

	// Depth stencil view description
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.MipSlice = 0;
	dsvDesc.Texture2DArray.ArraySize = 1; // We'll set the first slice index dynamically

	for (int i = 0; i < 6; ++i) {
		dsvDesc.Texture2DArray.FirstArraySlice = i; // Target a specific face
		device->CreateDepthStencilView(depthCubeTexture, &dsvDesc, &depthStencilView[i]);
	}

	// Shader resource view description
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	device->CreateShaderResourceView(depthCubeTexture, &srvDesc, &shaderResourceView);

	this->dsvDesc = dsvDesc;
	this->srvDesc = srvDesc;
	depthCubeTexture->Release();  // If you don't need it anymore

}
void Light::UpdateViewMatrix() {
	static DirectX::XMVECTOR directions[6] = {
		DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), // Right
		DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f), // Left
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), // Up
		DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), // Down
		DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), // Forward
		DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f)  // Back
	};

	static DirectX::XMVECTOR ups[6] = {
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), // Right
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), // Left
		DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), // Up
		DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), // Down
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), // Forward
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)  // Back
	};
	for (int i = 0; i < 6; i++) {
		viewMatrix[i] = DirectX::XMMatrixLookAtLH(lightPosition, directions[i], ups[i]);
	}
}