#include "Engine.h"

ShadowMapper::ShadowMapper(ID3D11Device *device, uint32_t width, uint32_t height, ID3D11VertexShader *vertexShader, 
	ID3D11InputLayout *layout) : m_vertexShader(vertexShader), m_layout(layout){

	D3D11_TEXTURE2D_DESC depthDesc = {0};
	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC depthShaderViewDesc;

	depthDesc.Width					= width;
	depthDesc.Height				= height;
	depthDesc.MipLevels				= 1;
	depthDesc.ArraySize				= 1;
	depthDesc.Format				= DXGI_FORMAT_R24G8_TYPELESS;
	depthDesc.SampleDesc.Count		= 1;
	depthDesc.SampleDesc.Quality	= 0;
	depthDesc.Usage					= D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags				= D3D11_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags		= 0;
	depthDesc.MiscFlags				= 0;

	ZeroMemory(&depthViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthViewDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthViewDesc.ViewDimension			= D3D11_DSV_DIMENSION_TEXTURE2D;
	depthViewDesc.Texture2D.MipSlice	= 0;

	ZeroMemory(&depthShaderViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	depthShaderViewDesc.ViewDimension		= D3D11_SRV_DIMENSION_TEXTURE2D;
	depthShaderViewDesc.Format				= DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthShaderViewDesc.Texture2D.MipLevels = 1;

	// Create the 2D-texture for writing depth to, create the shader-view and depth-view
	// to be able to manipulate this texture
	if(FAILED(device->CreateTexture2D(&depthDesc, NULL, &m_texture))) return;

	if(FAILED(device->CreateDepthStencilView(m_texture, &depthViewDesc, &m_depthView))){
		ReleaseCOM(m_texture);
		return;
	}

	if(FAILED(device->CreateShaderResourceView(m_texture, &depthShaderViewDesc, &m_shaderView))){
		ReleaseCOM(m_texture);
		ReleaseCOM(m_depthView);
	}

	m_cbData = (ShadowMapConstantBufferData *)_aligned_malloc(sizeof(ShadowMapConstantBufferData), 16);

	Util::CreateConstantBuffer(device, sizeof(ShadowMapConstantBufferData), &m_constantBuffer, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

ShadowMapper::~ShadowMapper(){

}

void ShadowMapper::startShadowRender(ID3D11DeviceContext *context, const Camera &light){

	// Clear depth texture and set rendering to only target depth
	context->OMSetRenderTargets(0, nullptr, m_depthView);
	context->ClearDepthStencilView(m_depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Set triangle topology
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set input layout
	context->IASetInputLayout(m_layout);

	// Set/update constant buffer then update it
	D3D11_MAPPED_SUBRESOURCE mappedSubRsrc;

	context->Map(m_constantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubRsrc);

	m_cbData->world = DirectX::XMMatrixIdentity();
	m_cbData->view = light.getViewMatrix();
	m_cbData->proj = light.getProjMatrix();
	memcpy(mappedSubRsrc.pData, m_cbData, sizeof(ShadowMapConstantBufferData));

	context->Unmap(m_constantBuffer, NULL);
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer);

	// Set vertex shader and null pixel shader
	context->VSSetShader(m_vertexShader, 0, 0);
	context->PSSetShader(nullptr, nullptr, 0);
}

void ShadowMapper::setWorldMatrix(ID3D11DeviceContext *context, const DirectX::XMMATRIX &world){
	
	// Update constant buffer with new world matrix
	D3D11_MAPPED_SUBRESOURCE mappedSubRsrc;

	context->Map(m_constantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubRsrc);

	m_cbData->world = world;
	memcpy(mappedSubRsrc.pData, m_cbData, sizeof(ShadowMapConstantBufferData));
}

ID3D11ShaderResourceView * ShadowMapper::getShadowTextureView() const{
	return m_shaderView;
}

