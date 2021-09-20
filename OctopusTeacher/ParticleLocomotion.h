#pragma once

constexpr int RESOLVE_CAP = 4;

class ParticleLocomotion
{
public:
	float radius;
	XMVECTOR position{ 0, 0, 0, 1 };

	const XMFLOAT3 MoveTowards(XMVECTOR direction) {
		XMFLOAT3 result;
		XMVECTOR position = this->position;
		for (int i = 0; i < RESOLVE_CAP; i++) {
			const auto pickResult = wiScene::Pick(
				RAY(position, direction),
				RENDERTYPE_OPAQUE,
				1U << 0);
			const auto length = XMVectorGetX(XMVector3Length(direction));
			if (pickResult.distance > length + radius) {
				XMStoreFloat3(&result, position + direction);
				return result;
			}
			position = XMLoadFloat3(&pickResult.position) - (direction * radius / length);
			position.m128_f32[4] = 1;
			direction = XMLoadFloat3(&pickResult.normal);
		}
		XMStoreFloat3(&result, position);
		return result;
	}
};
