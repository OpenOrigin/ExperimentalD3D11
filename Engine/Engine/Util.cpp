#include "Engine.h"

namespace Util{

bool CreateDX11Wnd(const D3DInitData &data, HWND *wndHandle, ID3D11Device **device, IDXGISwapChain **swapChain,
	ID3D11DeviceContext **deviceContext, ID3D11RenderTargetView **backBufferView, ID3D11DepthStencilView **depthView){

	*wndHandle = CreateSimpleWindow(data.instance, data.wndName, data.className, data.width, data.height);

	if(wndHandle){
		return InitalizeD3D(*wndHandle, data.width, data.height, data.levelMSAA, device, swapChain, deviceContext, backBufferView, depthView);
	}

	return false;
}

bool CreateVertexShaderFromBinary(ID3D11Device *device, const std::wstring &path, ID3D11VertexShader **shaderObj){
	HANDLE file = CreateFile(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if(file != INVALID_HANDLE_VALUE){
		DWORD bytesRead;
		DWORD size = GetFileSize(file, NULL);
		uint8_t *buffer = new uint8_t[size];

		ReadFile(file, buffer, size, &bytesRead, NULL);

		HRESULT result = device->CreateVertexShader(buffer, bytesRead, NULL, shaderObj);

		delete[] buffer;
		CloseHandle(file);

		return SUCCEEDED(result);
	}

	return false;
}

bool CreatePixelShaderFromBinary(ID3D11Device *device, const std::wstring &path, ID3D11PixelShader **shaderObj){
	HANDLE file = CreateFile(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if(file != INVALID_HANDLE_VALUE){
		DWORD bytesRead;
		DWORD size = GetFileSize(file, NULL);
		uint8_t *buffer = new uint8_t[size];

		ReadFile(file, buffer, size, &bytesRead, NULL);
		HRESULT result = device->CreatePixelShader(buffer, bytesRead, NULL, shaderObj);

		delete[] buffer;
		CloseHandle(file);

		return SUCCEEDED(result);
	}

	return false;
}

bool CreateVertexShaderFromFile(ID3D11Device *device, const std::wstring &path, const std::string &entryPt,
	ID3D11VertexShader **shaderObj){

	HRESULT result;
	ID3DBlob *shaderBlob, *errBlob;

	result = D3DCompileFromFile(path.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPt.c_str(), "vs_5_0", 0, 0, &shaderBlob, &errBlob);

	// Clean up and return error
	if(FAILED(result)){
		DbgOutA((char *)errBlob->GetBufferPointer());
		errBlob->Release();
		return false;
	}

	// Build into shader
	device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, shaderObj);

	if(errBlob) errBlob->Release();

	return true;
}

bool CreatePixelShaderFromFile(ID3D11Device *device, const std::wstring &path, const std::string &entryPt,
	ID3D11PixelShader **shaderObj){

	HRESULT result;
	ID3DBlob *shaderBlob, *errBlob;

	result = D3DCompileFromFile(path.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPt.c_str(), "ps_5_0", 0, 0, &shaderBlob, &errBlob);

	// Clean up and return error
	if(FAILED(result)){
		DbgOutA((char *)errBlob->GetBufferPointer());
		errBlob->Release();
		return false;
	}

	// Build into shader
	device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, shaderObj);

	if(errBlob) errBlob->Release();

	return true;
}

bool CreateVertexIndexBuffer(ID3D11Device *device, const VertexBufferCreationData &data, ID3D11Buffer **vertexBuffer, ID3D11Buffer **indexBuffer,
	D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG access){

	*vertexBuffer = BuildBuffer(device, data.vertexData, data.numVertices * data.vertexElementSize, D3D11_BIND_VERTEX_BUFFER, usage, access);

	if(*vertexBuffer == nullptr) return false;

	*indexBuffer = BuildBuffer(device, data.indexData, data.numIndices * sizeof(uint32_t), D3D11_BIND_INDEX_BUFFER, usage, access);

	if(*indexBuffer == nullptr){
		ReleaseCOM(*vertexBuffer);
		return false;
	}

	return true;
}

bool CreateConstantBuffer(ID3D11Device *device, uint32_t size, ID3D11Buffer **constantBuffer, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG access){
	*constantBuffer = BuildBuffer(device, nullptr, size, D3D11_BIND_CONSTANT_BUFFER, usage, access);

	return *constantBuffer != nullptr;
}

HWND CreateSimpleWindow(HINSTANCE instance, const std::wstring &wndName, const std::wstring &className,
	uint32_t width, uint32_t height){

	// Do window class stuff
	WNDCLASSEX wndClass = {0};

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WindowProc;
	wndClass.hInstance = instance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.lpszClassName = className.c_str();

	if(RegisterClassEx(&wndClass) == 0){
		DbgOut(L"Class registration failed");
		return 0;
	}

	// Create window
	HWND wndHandle = CreateWindowEx(0, className.c_str(), wndName.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, width, height,
		NULL, NULL, instance, NULL);

	if(wndHandle == NULL){
		DbgOut(L"Window creation failed");
		return false;
	}

	// Center window
	RECT centerRect;

	GetWindowRect(wndHandle, &centerRect);

	int32_t xPos = (GetSystemMetrics(SM_CXSCREEN) - centerRect.right) / 2;
	int32_t yPos = (GetSystemMetrics(SM_CYSCREEN) - centerRect.bottom) / 2;

	SetWindowPos(wndHandle, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	return wndHandle;
}

LRESULT CALLBACK WindowProc(HWND wndHandle, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(wndHandle, msg, wParam, lParam);
}

bool InitalizeD3D(HWND wndHandle, uint32_t width, uint32_t height, uint32_t levelMSAA, ID3D11Device **device, IDXGISwapChain **swapChain,
	ID3D11DeviceContext **deviceContext, ID3D11RenderTargetView **backBufferView, ID3D11DepthStencilView **depthView){

	// Fill in swap chain info
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};

	swapChainDesc.BufferCount		= 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.Width	= width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow		= wndHandle;
	swapChainDesc.SampleDesc.Count	= levelMSAA;
	swapChainDesc.Windowed			= TRUE;
	swapChainDesc.Flags				= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Build device and swap chain
	HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION,
		&swapChainDesc, swapChain, device, NULL, deviceContext);

	if(FAILED(result)){
		DbgOut(L"Device/Swap chain creation failed");
		return false;
	}

	// Get a view of the backbuffer and set it as the render target
	ID3D11Texture2D *backBuffer;

	(*swapChain)->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	(*device)->CreateRenderTargetView(backBuffer, NULL, backBufferView);

	// Create depth buffer
	ID3D11Texture2D *depthTexture;
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width				= width;
	depthStencilDesc.Height				= height;
	depthStencilDesc.MipLevels			= 1;
	depthStencilDesc.ArraySize			= 1;
	depthStencilDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count	= levelMSAA;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage				= D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags		= 0;
	depthStencilDesc.MiscFlags			= 0;

	(*device)->CreateTexture2D(&depthStencilDesc, NULL, &depthTexture);
	(*device)->CreateDepthStencilView(depthTexture, NULL, depthView);

	// Use the back buffer address to create the render target along with our depth buffer
	(*deviceContext)->OMSetRenderTargets(1, backBufferView, *depthView);

	backBuffer->Release();

	// Set the viewport
	D3D11_VIEWPORT viewport = {0};

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(width);
	viewport.Height = static_cast<FLOAT>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	(*deviceContext)->RSSetViewports(1, &viewport);

	return true;
}

bool CreateVertexLayoutFromFile(ID3D11Device *device, const std::wstring &path, const std::string &entryPt, ID3D11InputLayout **layout){

	// Compile vertex shader from file first
	HRESULT result;
	ID3DBlob *shaderBlob;

	result = D3DCompileFromFile(path.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPt.c_str(), "vs_5_0", 0, 0, &shaderBlob, NULL);

	// Clean up and return error
	if(FAILED(result)){
		return false;
	}

	// Grab reflection info
	ID3D11ShaderReflection *reflection;

	if(FAILED(D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
		IID_ID3D11ShaderReflection, (void **)&reflection))){

		ReleaseCOM(shaderBlob);

		return false;
	}

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;

	reflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	D3D11_INPUT_ELEMENT_DESC descElement;
	D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

	for(std::uint32_t i = 0; i < shaderDesc.InputParameters; i++){
		reflection->GetInputParameterDesc(i, &paramDesc);

		// Fill in info
		descElement.SemanticName = paramDesc.SemanticName;
		descElement.SemanticIndex = paramDesc.SemanticIndex;
		descElement.InputSlot = 0;
		descElement.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		descElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		descElement.InstanceDataStepRate = 0;

		// Determine DXGI format
		if(paramDesc.Mask == 1){
			if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		descElement.Format = DXGI_FORMAT_R32_UINT;
			else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	descElement.Format = DXGI_FORMAT_R32_SINT;
			else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	descElement.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if(paramDesc.Mask <= 3){
			if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		descElement.Format = DXGI_FORMAT_R32G32_UINT;
			else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	descElement.Format = DXGI_FORMAT_R32G32_SINT;
			else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	descElement.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if(paramDesc.Mask <= 7){
			if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		descElement.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	descElement.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	descElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if(paramDesc.Mask <= 15){
			if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)		descElement.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)	descElement.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)	descElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		inputLayoutDesc.push_back(descElement);
	}

	// Build input layout
	device->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(), layout);

	// Clean up
	reflection->Release();

	return true;
}

ID3D11Buffer *BuildBuffer(ID3D11Device *device, const void *data, uint32_t size, D3D11_BIND_FLAG bindFlag, D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG access){

	// Build buffer
	ID3D11Buffer *buffer;
	D3D11_BUFFER_DESC bufferDesc = {0};
	D3D11_SUBRESOURCE_DATA initalData = {0};

	bufferDesc.ByteWidth = size;
	bufferDesc.BindFlags = bindFlag;
	bufferDesc.Usage = usage;
	bufferDesc.CPUAccessFlags = access;

	initalData.pSysMem = data;

	if(FAILED(device->CreateBuffer(&bufferDesc, data ? &initalData : NULL, &buffer))){
		return nullptr;
	}

	return buffer;
}

}
