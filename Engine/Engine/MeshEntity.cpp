#include "Engine.h"

MeshEntity::MeshEntity(ID3D11Device *device, const std::wstring &path){
	m_vertexBuffer	= nullptr;
	m_indexBuffer	= nullptr;
	m_vertexSize	= sizeof(BoxVertex);
	m_world			= DirectX::XMMatrixIdentity();

	// Open up the model
	HANDLE file = CreateFile(path.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if(file == INVALID_HANDLE_VALUE) return;

	// Read in header
	DWORD bytesRead;
	BoxHeader header;
	uint32_t vertexBufferSize, indexBufferSize;

	ReadFile(file, &header, sizeof(BoxHeader), &bytesRead, NULL);

	// Fill in some data
	m_numVertices		= header.numVertices;
	m_numIndices		= header.numIndices;
	vertexBufferSize	= m_numVertices * m_vertexSize;
	indexBufferSize		= header.numIndices * sizeof(uint32_t);

	// Allocate buffers
	void *vertices		= new uint8_t[vertexBufferSize]();
	uint32_t *indices	= new uint32_t[header.numIndices]();

	// Read in vertices first, then indices
	ReadFile(file, vertices, vertexBufferSize, &bytesRead, NULL);
	ReadFile(file, indices, indexBufferSize, &bytesRead, NULL);

	// Create GPU-side buffers
	Util::VertexBufferCreationData data = {vertices, indices, m_numVertices, m_numIndices, m_vertexSize};

	Util::CreateVertexIndexBuffer(device, data, &m_vertexBuffer, &m_indexBuffer, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

	// Clean up
	delete[] vertices;
	delete[] indices;

	CloseHandle(file);
}

MeshEntity::MeshEntity(ID3D11Device *device, void *vertices, uint32_t *indices, int32_t numVertices, int32_t numIndices, int32_t vertexSize){
	m_vertexSize	= vertexSize;
	m_world			= DirectX::XMMatrixIdentity();

	// Fill in some data
	m_numVertices				= numVertices;
	m_numIndices				= numIndices;
	
	// Create GPU-side buffers
	Util::VertexBufferCreationData data = {vertices, indices, m_numVertices, m_numIndices, m_vertexSize};

	Util::CreateVertexIndexBuffer(device, data, &m_vertexBuffer, &m_indexBuffer, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

MeshEntity::MeshEntity(){
	m_vertexBuffer	= nullptr;
	m_indexBuffer	= nullptr;
	m_numVertices	= 0;
	m_numIndices	= 0;
	m_vertexSize	= 0;
	m_world			= DirectX::XMMatrixIdentity();
}

MeshEntity::~MeshEntity(){
	ReleaseCOM(m_vertexBuffer);
	ReleaseCOM(m_indexBuffer);
}

void MeshEntity::reset(){
	m_world = DirectX::XMMatrixIdentity();
}

void MeshEntity::translate(const DirectX::XMVECTOR &vector){
	DirectX::XMMATRIX translate = DirectX::XMMatrixTranslationFromVector(vector);

	m_world = DirectX::XMMatrixMultiply(m_world, translate);
}

void MeshEntity::translate(const DirectX::XMFLOAT3 &vector){
	translate(DirectX::XMLoadFloat3(&vector));
}

void MeshEntity::rotate(const DirectX::XMVECTOR &axis, float angle){
	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationAxis(axis, angle * DirectX::XM_PI / 180);

	m_world = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(m_world, rotation));
}

void MeshEntity::rotate(const DirectX::XMFLOAT3 &axis, float angle){
	rotate(DirectX::XMLoadFloat3(&axis), angle);
}

void MeshEntity::scale(const DirectX::XMVECTOR &vector){
	DirectX::XMMATRIX mat = DirectX::XMMatrixScalingFromVector(vector);

	m_world = DirectX::XMMatrixMultiply(m_world, mat);
}

void MeshEntity::scale(const DirectX::XMFLOAT3 &vector){
	scale(DirectX::XMLoadFloat3(&vector));
}

uint32_t MeshEntity::getNumVertices() const{
	return m_numVertices;
}

uint32_t MeshEntity::getNumIndices() const{
	return m_numIndices;
}

uint32_t MeshEntity::getVertexSize() const{
	return m_vertexSize;
}

ID3D11Buffer *MeshEntity::getVertexBuffer() const{
	return m_vertexBuffer;
}

ID3D11Buffer *MeshEntity::getIndexBuffer() const{
	return m_indexBuffer;
}

DirectX::XMMATRIX MeshEntity::getWorldMatrix() const{
	return DirectX::XMMatrixTranspose(m_world);
}

MeshEntity &MeshEntity::operator=(MeshEntity &entity){

	// Necessary to override this since the buffers are a shared pointer
	m_vertexBuffer	= entity.m_vertexBuffer;
	m_indexBuffer	= entity.m_indexBuffer;

	m_numVertices	= entity.m_numVertices;
	m_numIndices	= entity.m_numIndices;
	m_vertexSize	= entity.m_vertexSize;
	
	m_world			= entity.m_world;

	// Only the new class owns the buffers
	entity.m_vertexBuffer	= nullptr;
	entity.m_indexBuffer	= nullptr;

	return *this;
}
