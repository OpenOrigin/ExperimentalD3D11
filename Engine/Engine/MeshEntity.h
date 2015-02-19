#pragma once

struct BoxHeader{
	int32_t numVertices;
	int32_t numIndices;
};

struct BoxVertex{
	float x, y, z, w;
	float normX, normY, normZ;
	float u, v;
	float tanX, tanY, tanZ;
};

class MeshEntity{
private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	uint32_t m_numVertices, m_numIndices, m_vertexSize;
	DirectX::XMMATRIX m_world;

public:
	MeshEntity(ID3D11Device *device, const std::wstring &path);
	MeshEntity(ID3D11Device *device, void *vertices, uint32_t *indices, int32_t numVertices, int32_t numIndices, int32_t vertexSize);
	MeshEntity();
	~MeshEntity();

	void reset();

	void translate(const DirectX::XMVECTOR &vector);
	void translate(const DirectX::XMFLOAT3 &vector);

	void rotate(const DirectX::XMVECTOR &axis, float angle);
	void rotate(const DirectX::XMFLOAT3 &axis, float angle);

	void scale(const DirectX::XMVECTOR &vector);
	void scale(const DirectX::XMFLOAT3 &vector);

	uint32_t getNumVertices() const;
	uint32_t getNumIndices() const;
	uint32_t getVertexSize() const;

	ID3D11Buffer *getVertexBuffer() const;
	ID3D11Buffer *getIndexBuffer() const;
	DirectX::XMMATRIX getWorldMatrix() const;

	MeshEntity & operator=(MeshEntity &entity);
};
