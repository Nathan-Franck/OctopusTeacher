#include "ParticleComponent.h"

constexpr int RESOLVE_CAP = 4;

using namespace wiScene;

const XMFLOAT3 ParticleLocomotion::MoveTowards(XMVECTOR intendedDirection)
{
	XMVECTOR direction = intendedDirection;
	float directionLength = XMVectorGetX(XMVector3Length(intendedDirection));
	XMFLOAT3 result;
	XMVECTOR position = this->position;
	for (int i = 0; i < RESOLVE_CAP; i++) {
		// Move in direction
		XMVECTOR bounceDirection;
		PickResult bonkResult;
		{
			bonkResult = Pick(
				RAY(position, direction),
				RENDERTYPE_OPAQUE,
				1U << 0);
			if (bonkResult.distance > directionLength + radius)
			{
				XMStoreFloat3(&result, position + direction);
				return result;
			}
			// *BONK*
			const float distanceTravelled = bonkResult.distance - radius;
			position += direction * distanceTravelled / directionLength;
			bounceDirection = -XMLoadFloat3(&bonkResult.normal);
			directionLength -= distanceTravelled;
		}
		// Bounce off obstacle
		{
			const PickResult bounceResult = Pick(
				RAY(position, bounceDirection),
				RENDERTYPE_OPAQUE,
				1U << 0);
			const float bounceLength = XMVectorGetX(XMVector3Length(bounceDirection));
			if (bounceResult.distance > radius)
			{
				position += bounceDirection * radius / bounceLength;
			}
			else
			{
				const float distanceTravelled = fmax(bonkResult.distance - radius, 0.0f);
				position += bounceDirection * distanceTravelled / bounceLength;
			}
			// Continue in same direction perpendicular to surface normal
			const XMVECTOR normal = -XMLoadFloat3(&bonkResult.normal);
			const XMVECTOR cross = XMVector3Cross(intendedDirection, normal);
			direction = XMVector3Normalize(XMVector3Cross(cross, normal)) * directionLength;
			if (XMVectorGetX(XMVector3Dot(direction, intendedDirection)) < 0)
			{
				direction *= -1;
			}
		}
	}
	XMStoreFloat3(&result, position);
	return result;
}
