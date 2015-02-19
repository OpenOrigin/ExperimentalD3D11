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

static const float CameraMoveSpeed		= 1.5f;
static const float CameraRotateSpeed	= 0.005f;

}

struct ConstantBufferData{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;
	DirectX::XMVECTOR lightDir;
	DirectX::XMVECTOR cameraDir;
	DirectX::XMVECTOR mode;
};

// Shaders
ID3D11VertexShader *g_vertexShader;
ID3D11PixelShader *g_pixelShader;
ID3D11InputLayout *g_layout;

// Buffers
ID3D11Buffer *g_constantBuffer;

// Textures
ID3D11Texture2D *g_diffuseTexture, *g_normalTexture;
ID3D11ShaderResourceView *g_diffuseTextureView, *g_normalTextureView;

// CPU constant buffer data for shader
ConstantBufferData g_cbData;

// Timestamps
TimeStamp g_timeStart, g_timeCurrent;

// Entities
MeshEntity g_masterChief, g_crate, g_sphere;

// Test stuff
const float g_lightSpeed = .75f;
DirectX::XMFLOAT3 g_lightOrigin(0, 50, -10);
DirectX::XMFLOAT3 g_lightProtrusion(30, 30, -30);

//float test_vertices[] = {
//	-0.5f, -0.5f, 0.5f, 1.0f, 1.0f, .5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//	-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//	0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//	0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//};
//
//int32_t test_indices[] = {
//	0, 1, 2,
//	0, 2, 3,
//};

//VertexBufferData g_dataVert = {test_vertices, 4, sizeof(BoxVertex), test_indices, 6};
//
//LightSource g_light(DirectX::XMFLOAT3(0, 45, 0), DirectX::XMFLOAT3(16, 0, -16), 1.5);

void UpdateConstantBuffer(){
	D3D11_MAPPED_SUBRESOURCE mappedSubRsrc;

	Global::DeviceContext->Map(g_constantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubRsrc);
	memcpy(mappedSubRsrc.pData, &g_cbData, sizeof(ConstantBufferData));
	Global::DeviceContext->Unmap(g_constantBuffer, NULL);
}

void SetResources(){

	// TODO: Fix to get relative path working with CreateFile()...
	std::wstring path;
	wchar_t directory[300];

	GetModuleFileName(GetModuleHandle(NULL), directory, 300);
	PathRemoveFileSpec(directory);
	path = directory;

	// Create vertex/pixel shaders
	Util::CreateVertexShaderFromBinary(Global::Device, path + L"\\VertexShader.cso", &g_vertexShader);
	Util::CreatePixelShaderFromBinary(Global::Device, path + L"\\PixelShader.cso", &g_pixelShader);

	// Create input layout
	Util::CreateVertexLayoutFromFile(Global::Device, L"..\\Engine\\VertexShader.hlsl", "V_Shader", &g_layout);

	// Load models into renderable entities
	// TODO: Change boxer to use only a single file extension (.BOX), so that git doesn't commit it
	g_masterChief	= MeshEntity(Global::Device, L"..\\..\\Models\\chief.box");
	g_crate			= MeshEntity(Global::Device, L"..\\..\\Models\\crate.box");
	g_sphere		= MeshEntity(Global::Device, L"..\\..\\Models\\sphere.box");

	// Set models' orientation 
	DirectX::XMFLOAT3 axis(-1, 0, 0);

	g_masterChief.rotate(axis, 90);

	// Setup the shader's buffer
	Util::CreateConstantBuffer(Global::Device, sizeof(ConstantBufferData), &g_constantBuffer, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	// Setup world
	g_cbData.world = DirectX::XMMatrixIdentity();

	// Setup textures
	DirectX::CreateDDSTextureFromFile(Global::Device, Global::DeviceContext, L"..\\..\\Textures\\chief_techsuit_d.DDS", reinterpret_cast<ID3D11Resource **>(&g_diffuseTexture), &g_diffuseTextureView);
	DirectX::CreateDDSTextureFromFile(Global::Device, Global::DeviceContext, L"..\\..\\Textures\\chief_techsuit_n.DDS", reinterpret_cast<ID3D11Resource **>(&g_normalTexture), &g_normalTextureView);

	// Setup camera
	Global::UserCamera.setProperties(static_cast<float>(Global::Width), static_cast<float>(Global::Height), 0.1f, 1000.0f);
	Global::UserCamera.setPos(DirectX::XMFLOAT3(0, 0, 0));

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

	// Setup mode for toggling
	g_cbData.mode = DirectX::XMVectorSet(0, 0, 0, 0);
}

void HandleKeyInput(uint32_t vKey){
	switch(vKey){
	case 'W': Global::UserCamera.moveForward(Global::CameraMoveSpeed);		break;
		case 'S': Global::UserCamera.moveBackward(Global::CameraMoveSpeed);	break;
		case 'A': Global::UserCamera.moveLeft(Global::CameraMoveSpeed);		break;
		case 'D': Global::UserCamera.moveRight(Global::CameraMoveSpeed);	break;
		case 'Z': g_cbData.mode = DirectX::XMVectorSet(0, 0, 0, 0);			break;
		case 'X': g_cbData.mode = DirectX::XMVectorSet(1, 1, 1, 1);			break;
		case 'C': g_cbData.mode = DirectX::XMVectorSet(2, 2, 2, 2);			break;
		case 'V': g_cbData.mode = DirectX::XMVectorSet(3, 3, 3, 3);			break;
		case 'B': g_cbData.mode = DirectX::XMVectorSet(4, 4, 4, 4);			break;
	}
}

void HandleMouseMove(uint32_t rButton, uint16_t newXPos, uint16_t newYPos){
	static uint16_t oldXPos = newXPos, oldYPos = newYPos;

	float pitch = static_cast<float>(newYPos - oldYPos), yaw = static_cast<float>(newXPos - oldXPos);

	if(rButton) Global::UserCamera.rotate(pitch * Global::CameraRotateSpeed, yaw * Global::CameraRotateSpeed);

	oldXPos = newXPos;
	oldYPos = newYPos;
}

void Render(){
	float clearColor[] = {.3f, .5f, 1.0f, 1.0f};
	UINT stride = g_masterChief.getVertexSize(), offset = 0;

	// Get time stuff
	double time = Global::GameTimer.getDeltaTime(g_timeStart, g_timeCurrent);

	// Get camera matrices and direction
	g_cbData.proj = Global::UserCamera.getProjMatrix();
	g_cbData.view = Global::UserCamera.getViewMatrix();
	g_cbData.world = g_masterChief.getWorldMatrix();
	g_cbData.cameraDir = DirectX::XMVectorNegate(Global::UserCamera.getTarget());

	// Update light(s)
	g_cbData.lightDir = DirectX::XMVectorSet(
		cos(time * g_lightSpeed) * sin(time * g_lightSpeed) * g_lightProtrusion.x + g_lightOrigin.x,
		cos(time * g_lightSpeed) * g_lightProtrusion.y + g_lightOrigin.y,
		sin(time * g_lightSpeed) * sin(time * g_lightSpeed) * g_lightProtrusion.z + g_lightOrigin.z,
		1
	);

	// Update constant buffer
	UpdateConstantBuffer();

	// Clear depth view
	Global::DeviceContext->ClearDepthStencilView(Global::DepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Clear to color
	Global::DeviceContext->ClearRenderTargetView(Global::BackBufferView, clearColor);

	// Set topology
	Global::DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind buffers
	auto buffer = g_masterChief.getVertexBuffer();

	Global::DeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	Global::DeviceContext->IASetIndexBuffer(g_masterChief.getIndexBuffer(), DXGI_FORMAT_R32_UINT, offset);
	Global::DeviceContext->VSSetConstantBuffers(0, 1, &g_constantBuffer);
	Global::DeviceContext->PSSetConstantBuffers(0, 1, &g_constantBuffer);

	// Bind input
	Global::DeviceContext->IASetInputLayout(g_layout);

	// Bind shaders
	Global::DeviceContext->VSSetShader(g_vertexShader, 0, 0);
	Global::DeviceContext->PSSetShader(g_pixelShader, 0, 0);

	// Bind texture
	Global::DeviceContext->VSSetShaderResources(0, 1, &g_diffuseTextureView);
	Global::DeviceContext->PSSetShaderResources(0, 1, &g_diffuseTextureView);
	Global::DeviceContext->PSSetShaderResources(1, 1, &g_normalTextureView);

	// Bind sampler
	Global::DeviceContext->PSSetSamplers(0, 1, &Global::SimpleSampler);

	// Render geometry
	Global::DeviceContext->DrawIndexed(g_masterChief.getNumIndices(), 0, 0);


	// Render sphere/light source

	// Setup translations and update buffer
	g_sphere.reset();
	g_sphere.translate(g_cbData.lightDir);
	g_cbData.world = g_sphere.getWorldMatrix();

	UpdateConstantBuffer();

	// Bind sphere buffers
	buffer = g_sphere.getVertexBuffer();

	Global::DeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	Global::DeviceContext->IASetIndexBuffer(g_sphere.getIndexBuffer(), DXGI_FORMAT_R32_UINT, offset);

	// Render geometry 2
	Global::DeviceContext->DrawIndexed(g_sphere.getNumIndices(), 0, 0);
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