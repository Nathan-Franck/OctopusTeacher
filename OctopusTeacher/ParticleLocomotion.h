#pragma once

class ParticleLocomotion
{
public:
	float radius;
	XMVECTOR position{ 0, 0, 0, 1 };

	XMFLOAT3 MoveTowards(XMVECTOR direction) {
		const auto pickResult = wiScene::Pick(
			RAY(position, direction),
			RENDERTYPE_OPAQUE,
			1U << 32 - 1);
		XMFLOAT3 result;
		if (pickResult.distance * pickResult.distance > XMVectorGetX(XMVector3LengthSq(direction))) {
			XMStoreFloat3(&result, position + direction);
			return result;
		}
		XMStoreFloat3(&result, position);
		return result;
	}
};
