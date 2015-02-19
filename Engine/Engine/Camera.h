#pragma once

//////////////////
// Camera class //
//////////////////

class Camera{
private:
	DirectX::XMMATRIX m_proj, m_ortho;
	DirectX::XMVECTOR m_pos, m_target, m_up;

public:
	Camera();
	~Camera();

	void setPos(const DirectX::XMFLOAT3 &newPos);
	void setTarget(const DirectX::XMFLOAT3 &newTarget);
	void setUp(const DirectX::XMFLOAT3 &newUp);
	void setProperties(float width, float height, float nearPlane, float farPlane);

	void moveForward(float speed);
	void moveBackward(float speed);
	void moveLeft(float speed);
	void moveRight(float speed);

	void rotate(float pitch, float yaw);

	DirectX::XMVECTOR getPos() const;
	DirectX::XMVECTOR getTarget() const;
	DirectX::XMVECTOR getUp() const;
	DirectX::XMVECTOR getRight() const;

	DirectX::XMMATRIX getViewMatrix() const;
	DirectX::XMMATRIX getProjMatrix() const;
	DirectX::XMMATRIX getOrthoMatrix() const;
};
