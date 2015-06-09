#pragma once

struct ShadowMapConstantBufferData{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;
};

class ShadowMapper{
private:
	
	// Texture and its views
	ID3D11Texture2D *m_texture;
	ID3D11DepthStencilView *m_depthView;
	ID3D11ShaderResourceView *m_shaderView;

	// Shader values
	ID3D11InputLayout *m_layout;
	ID3D11VertexShader *m_vertexShader;

	// GPU and CPU constant buffer
	ID3D11Buffer *m_constantBuffer;
	ShadowMapConstantBufferData *m_cbData;

public:
	ShadowMapper(ID3D11Device *device, uint32_t width, uint32_t height, ID3D11VertexShader *vertexShader, 
		ID3D11InputLayout *layout);
	~ShadowMapper();

	void startShadowRender(ID3D11DeviceContext *context, const Camera &light);
	void setWorldMatrix(ID3D11DeviceContext *context, const DirectX::XMMATRIX &world);
	
	ID3D11ShaderResourceView * getShadowTextureView() const;
};
