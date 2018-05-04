#pragma once
#include <d3d10.h>
#include <d3dx10.h>
#include "ASRenderer.h"

constexpr float g_halfPI = 1.57079632f;

class ASCamera
{
	D3DXVECTOR3		m_position;//private coordinates
	D3DXVECTOR2		m_rotation;

	D3DXVECTOR3		m_vUp, m_vLook, m_vRight; // camera axis
	D3DXMATRIX		m_matView;
	effectMatrixVariable m_mvView;

protected:
	ASCamera();
	~ASCamera();

public:
	const float &x; //Public access to private coordinates, read only
	const float &y; //Public access to private coordinates, read only
	const float &z; //Public access to private coordinates, read only
	const D3DXVECTOR3 &xyz;
	D3DXMATRIX &m; //Public access to private view matrix, read only
	void Reset();

	D3DXVECTOR2 GetRotation();
	void SetPosition(D3DXVECTOR3 m);
	void Move(D3DXVECTOR3 m);
	void SetRotation(D3DXVECTOR2 r);
	void Rotate(D3DXVECTOR2 r);
	void SetPosition(float x, float y, float z);
	void Move(float x, float y, float z);
	void SetRotation(float x, float y);
	void Rotate(float x, float y);
	void Update();
};

D3DXVECTOR2 ASCamera::GetRotation()
{
	return m_rotation;
}

void		ASCamera::SetPosition(D3DXVECTOR3 vPos)
{
	m_position = vPos;
}
void		ASCamera::Move(D3DXVECTOR3 vMove)
{
	Move(vMove.x, vMove.y, vMove.z);
}
void		ASCamera::SetRotation(D3DXVECTOR2 r)
{
	SetRotation(r.x, r.y);
}
void		ASCamera::Rotate(D3DXVECTOR2 r)
{
	Rotate(r.x, r.y);
}

void		ASCamera::SetPosition(float dx, float dy, float dz)
{
	m_position.x = dx;
	m_position.y = dy;
	m_position.z = dz;
}
void		ASCamera::Move(float dx, float dy, float dz)
{
	m_position += m_vRight * dx;
	m_position += m_vUp * dy;
	m_position += m_vLook * dz;
}
void		ASCamera::SetRotation(float dx, float dy)
{
	m_rotation.x = dx;
	m_rotation.y = dy;
}
void		ASCamera::Rotate(float dx, float dy)
{
	m_rotation.x += dx;
	m_rotation.x = (m_rotation.x <  g_halfPI) ? m_rotation.x : g_halfPI;
	m_rotation.x = (m_rotation.x > -g_halfPI) ? m_rotation.x : -g_halfPI;
	m_rotation.y += dy;
}
void ASCamera::Reset()
{
	m_rotation.x = m_rotation.y = 0;
	m_position = D3DXVECTOR3(0, 1, -65);
	// Initialize the view matrix
	D3DXMatrixTranslation(&m_matView, -x, -y, -z);
	m_vUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	m_vLook = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	m_vRight = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
}



void ASCamera::Update()
{
	m_vUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	m_vLook = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	m_vRight = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	float yaw = m_rotation.y;                 // rotation around the y axis
	float pitch = m_rotation.x;               // rotation around the x axis

	D3DXMATRIX mYaw;
	D3DXMatrixRotationAxis(&mYaw, &m_vUp, yaw);
	D3DXVec3TransformCoord(&m_vLook, &m_vLook, &mYaw);
	D3DXVec3TransformCoord(&m_vRight, &m_vRight, &mYaw);

	D3DXMATRIX mPitch;
	D3DXMatrixRotationAxis(&mPitch, &m_vRight, pitch);
	D3DXVec3TransformCoord(&m_vLook, &m_vLook, &mPitch);
	D3DXVec3TransformCoord(&m_vUp, &m_vUp, &mPitch);

	m_matView._11 = m_vRight.x; m_matView._12 = m_vUp.x; m_matView._13 = m_vLook.x;
	m_matView._21 = m_vRight.y; m_matView._22 = m_vUp.y; m_matView._23 = m_vLook.y;
	m_matView._31 = m_vRight.z; m_matView._32 = m_vUp.z; m_matView._33 = m_vLook.z;

	m_matView._41 = -D3DXVec3Dot(&m_position, &m_vRight);
	m_matView._42 = -D3DXVec3Dot(&m_position, &m_vUp);
	m_matView._43 = -D3DXVec3Dot(&m_position, &m_vLook);

	m_mvView.Push(m);
}

ASCamera::ASCamera() : x(m_position.x), y(m_position.y), z(m_position.z), xyz(m_position), m(m_matView)
{
	m_mvView = effectMatrixVariable("matView");
	Reset();
}
ASCamera::~ASCamera() {}
