#pragma once
#include "WickedEngine.h"

constexpr int RESOLVE_CAP = 4;

class ParticleLocomotion
{
public:
	float radius;
	XMVECTOR position{ 0, 0, 0, 1 };
	const XMFLOAT3 MoveTowards(XMVECTOR direction);
};
