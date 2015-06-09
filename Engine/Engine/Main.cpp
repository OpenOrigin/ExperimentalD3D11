#include "Engine.h"

namespace Global{

// Device stuff
HWND WndHandle;
ID3D11Device *Device;
IDXGISwapChain *SwapChain;
ID3D11DeviceContext *DeviceContext;
ID3D11RenderTargetView *BackBufferView;
ID3D11DepthStencilView *DepthView;

// Utilites
Camera UserCamera;
Timer GameTimer;

// Shader stuff
ID3D11SamplerState *SimpleSampler;

// Constants
static const uint32_t Width				= 800;
static const uint32_t Height			= 600;

static const float CameraMoveSpeed		= 2.0f;
static const float CameraRotateSpeed	= 0.005f;

}

struct MaterialConstantBufferData{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX viewProj;
	DirectX::XMMATRIX lightView;
	DirectX::XMVECTOR lightDir;
	DirectX::XMVECTOR cameraDir;
	DirectX::XMVECTOR mode;
};

// Shaders and vertex layouts
ID3D11VertexShader *g_materialVS, *g_shadowVS, *g_passthruVS;
ID3D11PixelShader *g_materialPS, *g_texToQuadPS;
ID3D11InputLayout *g_materialVertLayout, *g_shadowVertLayout, *g_passthruVertLayout;

// Buffers
ID3D11Buffer *g_materialConstantBuffer;

// Textures
ID3D11Texture2D *g_diffuseTexture, *g_normalTexture;
ID3D11ShaderResourceView *g_diffuseTextureView, *g_normalTextureView;

// CPU-side constant buffer data for shaders
MaterialConstantBufferData g_materialCbData;

// Timestamps
TimeStamp g_timeStart, g_timeCurrent;

// Shadow mapper
ShadowMapper *g_shadowMapper;

// Geometric entities
MeshEntity g_masterChief, g_crate, g_sphere, g_plane, g_quad;

// Cameras
Camera g_lightCamera;

void UpdateConstantBuffer(){
	D3D11_MAPPED_SUBRESOURCE mappedSubRsrc;

	Global::DeviceContext->Map(g_materialConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubRsrc);
	memcpy(mappedSubRsrc.pData, &g_materialCbData, sizeof(MaterialConstantBufferData));
	Global::DeviceContext->Unmap(g_materialConstantBuffer, NULL);
}

bool LoadShaders(){
	int ret = 0;

	// Create vertex/pixel shaders
	if(Util::CreateVertexShaderFromBinary(Global::Device, L"..\\Debug\\Material_VS.cso", &g_materialVS))	ret++;
	if(Util::CreateVertexShaderFromBinary(Global::Device, L"..\\Debug\\Shadow_VS.cso", &g_shadowVS))		ret++;
	if(Util::CreateVertexShaderFromBinary(Global::Device, L"..\\Debug\\Passthru_VS.cso", &g_passthruVS))	ret++;
	
	if(Util::CreatePixelShaderFromBinary(Global::Device, L"..\\Debug\\Material_PS.cso", &g_materialPS))		ret++;
	if(Util::CreatePixelShaderFromBinary(Global::Device, L"..\\Debug\\TexToQuad_PS.cso", &g_texToQuadPS))	ret++;

	return (ret == 5);
}

bool LoadLayouts(){
	int ret = 0;

	// Create input layouts
	if(Util::CreateVertexLayoutFromFile(Global::Device, L"..\\Engine\\Material_VS.hlsl", "V_Shader", &g_materialVertLayout))	ret++;
	if(Util::CreateVertexLayoutFromFile(Global::Device, L"..\\Engine\\Shadow_VS.hlsl", "V_Shader", &g_shadowVertLayout))		ret++;
	if(Util::CreateVertexLayoutFromFile(Global::Device, L"..\\Engine\\Passthru_VS.hlsl", "V_Shader", &g_passthruVertLayout))	ret++;

	return (ret == 3);
}

bool LoadEntities(){
	int ret = 0;

	float g_planeRawVertices[] = {
		-0.5f, -0.5f, 0.5f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,  -0.707f, 0.0f, 0.707f,
		-0.5f, 0.5f, 0.5f, 1.0f,   0.0f, 0.0f, -1.0f,  0.0f, 1.0f,  -0.707f, 0.0f, -0.707f,
		0.5f, 0.5f, 0.5f, 1.0f,    0.0f, 0.0f, -1.0f,  1.0f, 0.0f,   0.707f, 0.0f, -0.707f,
		0.5f, -0.5f, 0.5f, 1.0f,   0.0f, 0.0f, -1.0f,  1.0f, 1.0f,   0.707f, 0.0f, 0.707f,
	};

	float g_quadRawVertices[] = {
		0.0f, -1.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // x = 0, y = -1 (bottom left)
		0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,  // x = 0, y = 0 (top left)
		1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 0.0f,  // x = 1, y = 0 (top right)
		1.0f, -1.0f, 0.0f, 1.0f,   1.0f, 1.0f,  // x = 1, y = -1 (bottom right)
	};

	uint32_t g_planeRawIndices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	// Load models into renderable entities
	// TODO: Change boxer to use only a single file extension (.BOX), so that git doesn't commit it
	if(LoadMeshFromFile(Global::Device, L"..\\..\\Models\\chief.box", g_masterChief))								ret++;
	if(LoadMeshFromFile(Global::Device, L"..\\..\\Models\\crate.box", g_crate))										ret++;
	if(LoadMeshFromFile(Global::Device, L"..\\..\\Models\\sphere.box", g_sphere))									ret++;
	if(LoadMeshFromFile(Global::Device, g_planeRawVertices, g_planeRawIndices, 4, 6, sizeof(BoxVertex), g_plane))	ret++;
	if(LoadMeshFromFile(Global::Device, g_quadRawVertices, g_planeRawIndices, 4, 6, sizeof(float) * 6, g_quad))		ret++;
	//LoadMeshFromFile(Global::Device, L"..\\..\\Models\\wall.box", g_plane);

	return (ret == 5);
}

bool LoadTexturesAndSampler(){
	int ret = 0;
	
	// Load textures
	if(SUCCEEDED(DirectX::CreateDDSTextureFromFile(Global::Device, Global::DeviceContext, L"..\\..\\Textures\\chief_techsuit_d.DDS",
		reinterpret_cast<ID3D11Resource **>(&g_diffuseTexture), &g_diffuseTextureView))) ret++;
	if(SUCCEEDED(DirectX::CreateDDSTextureFromFile(Global::Device, Global::DeviceContext, L"..\\..\\Textures\\chief_techsuit_n.DDS",
		reinterpret_cast<ID3D11Resource **>(&g_normalTexture), &g_normalTextureView))) ret++;

	// Define simple sampler
	D3D11_SAMPLER_DESC SamplerStateTexture = {
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0, 1,
		D3D11_COMPARISON_NEVER,
		0, 0, 0, 0, 0,
		D3D11_FLOAT32_MAX
	};

	Global::Device->CreateSamplerState(&SamplerStateTexture, &Global::SimpleSampler);

	return (ret == 2);
}

void SetResources(){
	if(!LoadShaders()){
		MessageBox(0, L"Error loading shaders", L"Error", 0);
		exit(-1);
	}

	if(!LoadLayouts()){
		MessageBox(0, L"Error loading vertex layouts from shaders", L"Error", 0);
		exit(-1);
	}

	if(!LoadEntities()){
		MessageBox(0, L"Error loading geometry from files", L"Error", 0);
		exit(-1);
	}

	if(!LoadTexturesAndSampler()){
		MessageBox(0, L"Error loading textures", L"Error", 0);
		exit(-1);
	}

	// Setup the material shader's constant buffer
	Util::CreateConstantBuffer(Global::Device, sizeof(MaterialConstantBufferData), &g_materialConstantBuffer, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	g_materialCbData.world = DirectX::XMMatrixIdentity();

	// Setup cameras
	Global::UserCamera.setProperties(static_cast<float>(Global::Width), static_cast<float>(Global::Height), 0.1f, 1000.0f);
	g_lightCamera.setProperties(static_cast<float>(Global::Width), static_cast<float>(Global::Height), 0.1f, 1000.0f);

	Global::UserCamera.setPos(DirectX::XMFLOAT3(0, 0, 0));

	// Setup mode for toggling
	g_materialCbData.mode = DirectX::XMVectorSet(0, 0, 0, 0);

	// Setup shadow-mapping
	g_shadowMapper = new ShadowMapper(Global::Device, Global::Width, Global::Height, g_shadowVS, g_shadowVertLayout);
}

void HandleKeyInput(uint32_t vKey){
	switch(vKey){
	case 'W': Global::UserCamera.moveForward(Global::CameraMoveSpeed);		break;
		case 'S': Global::UserCamera.moveBackward(Global::CameraMoveSpeed);	break;
		case 'A': Global::UserCamera.moveLeft(Global::CameraMoveSpeed);		break;
		case 'D': Global::UserCamera.moveRight(Global::CameraMoveSpeed);	break;
		case 'Z': g_materialCbData.mode = DirectX::XMVectorSet(0, 0, 0, 0);			break;
		case 'X': g_materialCbData.mode = DirectX::XMVectorSet(1, 1, 1, 1);			break;
		case 'C': g_materialCbData.mode = DirectX::XMVectorSet(2, 2, 2, 2);			break;
		case 'V': g_materialCbData.mode = DirectX::XMVectorSet(3, 3, 3, 3);			break;
		case 'B': g_materialCbData.mode = DirectX::XMVectorSet(4, 4, 4, 4);			break;
	}
}

void HandleMouseMove(uint32_t rButton, uint16_t newXPos, uint16_t newYPos){
	static uint16_t oldXPos = newXPos, oldYPos = newYPos;

	float pitch = static_cast<float>(newYPos - oldYPos), yaw = static_cast<float>(newXPos - oldXPos);

	if(rButton) Global::UserCamera.rotate(pitch * Global::CameraRotateSpeed, yaw * Global::CameraRotateSpeed);

	oldXPos = newXPos;
	oldYPos = newYPos;
}

void GenerateShadowMap(){

	// Send only position (float4), adjust offset to make up for difference
	//UINT stride = sizeof(float) * 4, offset = sizeof(float) * 8;
	UINT stride = g_masterChief.getVertexSize(), offset = 0;
	
	g_shadowMapper->startShadowRender(Global::DeviceContext, g_lightCamera);

	// Prepare shadow caster
	ID3D11Buffer *vertexBuffer = g_masterChief.getVertexBuffer();
	ID3D11Buffer *indexBuffer = g_masterChief.getIndexBuffer();

	// Transform
	DirectX::XMFLOAT3 axis(-1, 0, 0);
	
	g_masterChief.reset();
	g_masterChief.rotate(axis, 90);

	g_shadowMapper->setWorldMatrix(Global::DeviceContext, g_masterChief.getWorldMatrix());

	// Attach buffers and render
	Global::DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	Global::DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, offset);
	Global::DeviceContext->DrawIndexed(g_masterChief.getNumIndices(), 0, 0);
}

void RenderScene(){
	float clearColor[] = {.3f, .5f, 1.0f, 1.0f};
	auto shadowTextureView = g_shadowMapper->getShadowTextureView();

	// Re-apply backbuffer and depth buffer as render targets
	Global::DeviceContext->OMSetRenderTargets(1, &Global::BackBufferView, Global::DepthView);

	// Clear backbuffer, depth view and apply topology
	Global::DeviceContext->ClearDepthStencilView(Global::DepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	Global::DeviceContext->ClearRenderTargetView(Global::BackBufferView, clearColor);
	Global::DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind input layout
	Global::DeviceContext->IASetInputLayout(g_materialVertLayout);

	// Bind material vertex/pixel shaders
	Global::DeviceContext->VSSetShader(g_materialVS, 0, 0);
	Global::DeviceContext->PSSetShader(g_materialPS, 0, 0);

	// Bind textures
	Global::DeviceContext->PSSetShaderResources(0, 1, &g_diffuseTextureView);
	Global::DeviceContext->PSSetShaderResources(1, 1, &g_normalTextureView);
	Global::DeviceContext->PSSetShaderResources(2, 1, &shadowTextureView);

	// Bind texture-sampler
	Global::DeviceContext->PSSetSamplers(0, 1, &Global::SimpleSampler);

	// Bind constant buffer
	Global::DeviceContext->VSSetConstantBuffers(0, 1, &g_materialConstantBuffer);
	Global::DeviceContext->PSSetConstantBuffers(0, 1, &g_materialConstantBuffer);

	// Fill constant buffer
	g_materialCbData.viewProj = Global::UserCamera.getProjMatrix() * Global::UserCamera.getViewMatrix();
	g_materialCbData.lightView = g_lightCamera.getViewMatrix() * Global::UserCamera.getProjMatrix();
	g_materialCbData.cameraDir = DirectX::XMVectorNegate(Global::UserCamera.getTarget());
	g_materialCbData.lightDir = g_lightCamera.getPos();

	// Render each object in a loop
	DirectX::XMFLOAT3 axis(-1, 0, 0);
	float rotations[] = {90, 0, 90};
	DirectX::XMFLOAT3 scales[] = {DirectX::XMFLOAT3(1, 1, 1), DirectX::XMFLOAT3(1, 1, 1), DirectX::XMFLOAT3(100, 100, 100)};
	DirectX::XMFLOAT3 translations[] = {DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0, 50, 0)};

	MeshEntity *entities[] = {&g_masterChief, &g_sphere, &g_plane};

	ID3D11Buffer *vertexBuffers[] = {entities[0]->getVertexBuffer(), entities[1]->getVertexBuffer(), entities[2]->getVertexBuffer()};
	ID3D11Buffer *indexBuffers[] = {entities[0]->getIndexBuffer(), entities[1]->getIndexBuffer(), entities[2]->getIndexBuffer()};
	UINT stride = g_masterChief.getVertexSize(), offset = 0;

	// Add in the sphere translation
	DirectX::XMStoreFloat3(&translations[1], g_lightCamera.getPos());

	for(int i = 0; i < 3; i++){

		// Transform
		entities[i]->reset();
		entities[i]->rotate(axis, rotations[i]);
		entities[i]->scale(scales[i]);
		entities[i]->translate(translations[i]);

		g_materialCbData.world = entities[i]->getWorldMatrix();

		UpdateConstantBuffer();

		// Attach buffers and render
		Global::DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffers[i], &stride, &offset);
		Global::DeviceContext->IASetIndexBuffer(entities[i]->getIndexBuffer(), DXGI_FORMAT_R32_UINT, offset);
		Global::DeviceContext->DrawIndexed(entities[i]->getNumIndices(), 0, 0);
	}
}

void RenderFromTexture(){
	auto shadowTextureView = g_shadowMapper->getShadowTextureView();

	// Bind input layout
	Global::DeviceContext->IASetInputLayout(g_passthruVertLayout);

	// Bind material vertex/pixel shaders
	Global::DeviceContext->VSSetShader(g_passthruVS, 0, 0);
	Global::DeviceContext->PSSetShader(g_texToQuadPS, 0, 0);

	// Bind texture
	Global::DeviceContext->PSSetShaderResources(0, 1, &shadowTextureView);

	// Bind texture-sampler
	Global::DeviceContext->PSSetSamplers(0, 1, &Global::SimpleSampler);

	// Render quad
	auto vertexBuffer = g_quad.getVertexBuffer();
	UINT stride = g_quad.getVertexSize(), offset = 0;

	Global::DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	Global::DeviceContext->IASetIndexBuffer(g_quad.getIndexBuffer(), DXGI_FORMAT_R32_UINT, offset);
	Global::DeviceContext->DrawIndexed(g_quad.getNumIndices(), 0, 0);
}

void Render(){
	const float LightSpeed = .75f;
	DirectX::XMFLOAT3 lightOrigin(0, 30, -15), lightProtrusion(40, 0, 40);
	float time = (float)Global::GameTimer.getDeltaTime(g_timeStart, g_timeCurrent);
	
	// Update light(s)
	g_lightCamera.setPos(
		DirectX::XMFLOAT3(
			cos(time * LightSpeed) * sin(time * LightSpeed) * lightProtrusion.x + lightOrigin.x,
			cos(time * LightSpeed) * lightProtrusion.y + lightOrigin.y,
			sin(time * LightSpeed) * sin(time * LightSpeed) * lightProtrusion.z + lightOrigin.z
		)
	);

	// Calculate new light camera target (flip position)
	auto vector = DirectX::XMVector3Normalize(DirectX::XMVectorNegate(g_lightCamera.getPos()));
	DirectX::XMFLOAT3 target;

	DirectX::XMStoreFloat3(&target, vector);

	g_lightCamera.setTarget(target);

	GenerateShadowMap();
	RenderScene();
	RenderFromTexture();
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int numCmdShow){
	CoInitialize(NULL);

	Util::D3DInitData data = {instance, L"Wnd", L"DX_Wnd", Global::Width, Global::Height, 1};

	CreateDX11Wnd(data, &Global::WndHandle, &Global::Device, &Global::SwapChain, &Global::DeviceContext, &Global::BackBufferView, &Global::DepthView);

	SetResources();

	MSG msg;

	Global::GameTimer.createTimeStamp(g_timeStart);
	g_timeCurrent = g_timeStart;

	while(true){
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(msg.message == WM_KEYDOWN){
				HandleKeyInput(msg.wParam);
			}
			else if(msg.message == WM_MOUSEMOVE){
				HandleMouseMove(msg.wParam & MK_RBUTTON, LOWORD(msg.lParam), HIWORD(msg.lParam));
			}

			if(msg.message == WM_QUIT) break;
		}

		Global::GameTimer.createTimeStamp(g_timeCurrent);

		Render();

		Global::SwapChain->Present(0, 0);
	}

	return 0;
}