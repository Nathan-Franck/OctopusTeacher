#pragma once

#include "WickedEngine.h"

class ParticleLocomotion
{
public:
	float radius;
	XMVECTOR position{ 0, 0, 0, 1 };
	const XMFLOAT3 MoveTowards(XMVECTOR direction);
};
