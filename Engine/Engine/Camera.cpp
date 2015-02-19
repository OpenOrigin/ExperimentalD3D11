#include "Engine.h"

Camera::Camera(){
	m_pos		= DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
	m_target	= DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	m_up		= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	m_proj		= DirectX::XMMatrixIdentity();
	m_ortho		= DirectX::XMMatrixIdentity();
}

Camera::~Camera(){

}

void Camera::setPos(const DirectX::XMFLOAT3 &newPos){
	m_pos = DirectX::XMLoadFloat3(&newPos);
}

void Camera::setTarget(const DirectX::XMFLOAT3 &newTarget){
	m_target = DirectX::XMLoadFloat3(&newTarget);
}

void Camera::setUp(const DirectX::XMFLOAT3 &newUp){
	m_up = DirectX::XMLoadFloat3(&newUp);
}

void Camera::setProperties(float width, float height, float nearPlane, float farPlane){
	m_proj	= DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, (width / height), nearPlane, farPlane);
	m_ortho = DirectX::XMMatrixOrthographicLH(width, height, nearPlane, farPlane);
}

void Camera::moveForward(float speed){
	m_pos = DirectX::XMVectorAdd(m_pos, DirectX::XMVectorScale(m_target, speed));
}

void Camera::moveBackward(float speed){
	m_pos = DirectX::XMVectorAdd(m_pos, DirectX::XMVectorScale(m_target, -speed));
}

void Camera::moveLeft(float speed){
	DirectX::XMVECTOR left = DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVector3Cross(m_target, m_up)), speed);

	m_pos = DirectX::XMVectorAdd(m_pos, DirectX::XMVectorScale(left, speed));
}

void Camera::moveRight(float speed){
	DirectX::XMVECTOR right = DirectX::XMVectorScale(DirectX::XMVector3Normalize(DirectX::XMVector3Cross(m_up, m_target)), speed);

	m_pos = DirectX::XMVectorAdd(m_pos, DirectX::XMVectorScale(right, speed));
}

void Camera::rotate(float pitch, float yaw){
	DirectX::XMVECTOR right		= getRight();
	DirectX::XMMATRIX rotation	= DirectX::XMMatrixRotationAxis(right, -pitch);

	m_up		= DirectX::XMVector3TransformNormal(m_up, rotation);
	m_target	= DirectX::XMVector3TransformNormal(m_target, rotation);

	rotation	= DirectX::XMMatrixRotationY(yaw);

	right		= DirectX::XMVector3TransformNormal(right, rotation);
	m_up		= DirectX::XMVector3TransformNormal(m_up, rotation);
	m_target	= DirectX::XMVector3TransformNormal(m_target, rotation);
}

DirectX::XMVECTOR Camera::getPos() const{
	return m_pos;
}

DirectX::XMVECTOR Camera::getTarget() const{
	return m_target;
}

DirectX::XMVECTOR Camera::getUp() const{
	return m_up;
}

DirectX::XMVECTOR Camera::getRight() const{
	return DirectX::XMVector3Normalize(DirectX::XMVector3Cross(m_target, m_up));
}

DirectX::XMMATRIX Camera::getViewMatrix() const{
	return DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(m_pos, DirectX::XMVectorAdd(m_target, m_pos), m_up));
}

DirectX::XMMATRIX Camera::getProjMatrix() const{
	return DirectX::XMMatrixTranspose(m_proj);
}

DirectX::XMMATRIX Camera::getOrthoMatrix() const{
	return DirectX::XMMatrixTranspose(m_ortho);
}

