#pragma once

#define DbgOut(x) OutputDebugString(x)
#define DbgOutA(x) OutputDebugStringA(x)
#define DbgOutW(x) OutputDebugStringW(x)

#define ReleaseCOM(x) if(x) (x)->Release(); (x) = nullptr;

namespace Util{

struct D3DInitData{
	HINSTANCE instance;

	std::wstring wndName;
	std::wstring className;
	int32_t width;
	int32_t height;

	uint32_t levelMSAA;
};

struct VertexBufferCreationData{
	void *vertexData;
	uint32_t *indexData;

	uint32_t numVertices;
	uint32_t numIndices;

	uint32_t vertexElementSize;
};

//////////////////////
// Client functions //
//////////////////////

// Creates a D3D11 window
bool CreateDX11Wnd(const D3DInitData &data, HWND *wndHandle, ID3D11Device **device, IDXGISwapChain **swapChain,
	ID3D11DeviceContext **deviceContext, ID3D11RenderTargetView **backBufferView, ID3D11DepthStencilView **depthView);

// Creates shaders from .CSO binary files
bool CreateVertexShaderFromBinary(ID3D11Device *device, const std::wstring &path, ID3D11VertexShader **shaderObj);
bool CreatePixelShaderFromBinary(ID3D11Device *device, const std::wstring &path, ID3D11PixelShader **shaderObj);

// Compiles .HLSL files and creates shaders from them
bool CreateVertexShaderFromFile(ID3D11Device *device, const std::wstring &path, const std::string &entryPt,
	ID3D11VertexShader **shaderObj);
bool CreatePixelShaderFromFile(ID3D11Device *device, const std::wstring &path, const std::string &entryPt,
	ID3D11PixelShader **shaderObj);

// Creates a vertex input layout from a vertex shader file (.HLSL)
bool CreateVertexLayoutFromFile(ID3D11Device *device, const std::wstring &path, const std::string &entryPt, ID3D11InputLayout **layout);

// Creates vertex/index/constant buffers
bool CreateVertexIndexBuffer(ID3D11Device *device, const VertexBufferCreationData &data, ID3D11Buffer **vertexBuffer, ID3D11Buffer **indexBuffer, 
	D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG access);
bool CreateConstantBuffer(ID3D11Device *device, uint32_t size, ID3D11Buffer **constantBuffer, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG access);

//////////////////////
// Helper functions //
//////////////////////

// Creates a window to be able to render D3D scenes
HWND CreateSimpleWindow(HINSTANCE instance, const std::wstring &wndName, const std::wstring &className, uint32_t width, uint32_t height);

// A generic window callback
LRESULT CALLBACK WindowProc(HWND wndHandle, UINT msg, WPARAM wParam, LPARAM lParam);

// Sets up D3D and associated contexts
bool InitalizeD3D(HWND wndHandle, uint32_t width, uint32_t height, uint32_t levelMSAA, ID3D11Device **device, IDXGISwapChain **swapChain,
	ID3D11DeviceContext **deviceContext, ID3D11RenderTargetView **backBufferView, ID3D11DepthStencilView **depthView);

// Creates a buffer from input
ID3D11Buffer *BuildBuffer(ID3D11Device *device, const void *data, uint32_t size, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage,
	D3D11_CPU_ACCESS_FLAG access);

}
