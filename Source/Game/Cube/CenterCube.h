#pragma once

#include "BaseCube.h"

class CenterCube : public BaseCube
{
public:
	CenterCube(PCWSTR name);
	~CenterCube() = default;

	virtual void Update(FLOAT deltaTime) override;
private:
	PCWSTR name;
};