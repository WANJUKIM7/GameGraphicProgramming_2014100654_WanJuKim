#include "CenterCube.h"

CenterCube::CenterCube(PCWSTR name)
	: name(name)
{
	m_world = XMMatrixIdentity();
	if (name == L"Center")
	{
		m_world *= XMMatrixTranslation(0.0f, 0.0f, 0.0f);
		m_world *= XMMatrixScaling(1.0f, 1.0f, 1.0f);
		m_world *= XMMatrixRotationY(0.0f);
	}
	else if (name == L"Orbit")
	{
		m_world *= XMMatrixScaling(0.3f, 0.3f, 0.3f);
		m_world *= XMMatrixTranslation(4.0f, 0.0f, 0.0f);
	}
}

void CenterCube::Update(FLOAT deltaTime)
{
	if (name == L"Center")
	{
		RotateY(deltaTime);
	}
	else if (name == L"Orbit")
	{
		m_world *= XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
		RotateZ(-1.0f * deltaTime);
		m_world *= XMMatrixTranslation(4.0f, 0.0f, 0.0f);
		RotateY(2.0f * -1.0f * deltaTime);
	}
}
