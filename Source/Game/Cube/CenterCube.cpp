#include "CenterCube.h"

CenterCube::CenterCube(PCWSTR name)
	: name(name)
	, m_rotate(XMMatrixIdentity())
	, m_orbit(XMMatrixIdentity())
	, randNum(0.0f)
	, bSize(true)
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
	else if (name == L"CustomCube")
	{
		if (randNum > 1.5f)
		{
			bSize = false;
			randNum -= deltaTime;
		}
		else if (randNum < 0.1f)
		{
			randNum += deltaTime;
			bSize = true;
		}
		else if (bSize == false)
		{
			randNum -= deltaTime;
		}
		else if (bSize == true)
		{
			randNum += deltaTime;
		}

		m_rotate *= XMMatrixRotationY(-5.0f * deltaTime);
		m_orbit *= XMMatrixRotationZ(1.5f * -1.0f * deltaTime);
		m_world = XMMatrixScaling(randNum, randNum, randNum) * m_rotate * XMMatrixTranslation(0.0f, 2.0f, 0.0f) * m_orbit;
	}
}
