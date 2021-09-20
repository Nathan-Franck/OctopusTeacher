#pragma once

class ParticleLocomotion
{
public:
	float radius;
	XMVECTOR position;

	XMFLOAT3 MoveTowards(XMVECTOR direction) {
		const auto result = wiScene::Pick(
			RAY(position, direction),
			RENDERTYPE_OPAQUE,
			1U << 32 - 1);
		return result.position;
	}
};
