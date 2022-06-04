#include "Cube/RotatingCube.h"

RotatingCube::RotatingCube(const XMFLOAT4& outputColor)
    : BaseCube(outputColor)
{
}

void RotatingCube::Update(_In_ FLOAT deltaTime)
{
    static FLOAT t = 0.0f;
    t += deltaTime;

    XMMATRIX mOrbit = XMMatrixRotationY(-t * 2.0f);
    XMMATRIX mTranslate = XMMatrixTranslation(0.0f, 30.0f, -50.0f);
    
    m_world = mTranslate * mOrbit;
}