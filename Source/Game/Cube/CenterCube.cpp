#include "CenterCube.h"

CenterCube::CenterCube(PCWSTR name)
	: name(name)
	, m_rotate(XMMatrixIdentity())
	, m_orbit(XMMatrixIdentity())
{
}

void CenterCube::Update(FLOAT deltaTime)
{
	if (name == L"Center")
	{
		RotateY(deltaTime);
	}
	else if (name == L"Orbit")
	{
		m_rotate *= XMMatrixRotationZ(-1.0f * deltaTime);
		m_orbit *= XMMatrixRotationY(2.0f * -1.0f * deltaTime);
		m_world = XMMatrixScaling(0.3f, 0.3f, 0.3f) * m_rotate * XMMatrixTranslation(-4.0f, 0.0f, 0.0f) * m_orbit;
	}
}
