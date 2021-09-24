#pragma once

#include "WickedEngine.h"

using namespace std;
using namespace wiECS;
using namespace wiScene;

const static float strideLength = 0.75;
const static float smoothSpeed = 1;
constexpr int LIMB_COUNT = 8;
constexpr int LIMB_SEGMENTS = 8;

struct OctopusComponent {

	array<array<Entity, LIMB_SEGMENTS>, LIMB_COUNT> limbs;
	vector<XMFLOAT3> relativeTargetGoals;
	XMMATRIX previousOctopusMatrix;

	struct Target {
		XMFLOAT3 position;
		float time;
	};

	vector<vector<Target>> targets;

	Entity octopusScene;

	OctopusComponent() {}

	void Initialize(Entity octopus);

	OctopusComponent(Entity octopus) {
		Initialize(octopus);
	}

	void Retargetting(float time);

	static auto GetRelativeTarget(XMFLOAT3 target, Entity bone);

	static auto GetSegments(array<Entity, LIMB_SEGMENTS> bones);

	static void CurlTips(const array<Entity, LIMB_SEGMENTS>& bones, XMVECTOR relativeTarget);

	void BasicWalk(float time);

	void Update(float time);
};
