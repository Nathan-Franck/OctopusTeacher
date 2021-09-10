#pragma once

#include "utils.h"
#include "easing.h"
#include <vector>
#include <algorithm>

using namespace std;
using namespace wiECS;
using namespace wiScene;

const static float strideLength = 2;
const static float smoothSpeed = 1;
constexpr int LIMB_COUNT = 8;
constexpr int LIMB_SEGMENTS = 8;

static array<array<Entity, LIMB_SEGMENTS>, LIMB_COUNT> findLimbsFromScene(Entity octopusScene)
{
    const auto getOctopus = [](const vector<Entity>& entities)
    {
        for (const auto entity : entities)
            if (componentFromEntity<NameComponent>(entity)->name == "OctopusRiggedTopo.glb")
                return entity;
		return INVALID_ENTITY;
    };
    const auto getArmature = [](const vector<Entity>& entities)
    {
        for (const auto entity : entities)
            if (componentFromEntity<NameComponent>(entity)->name == "Armature")
                return entity;
		return INVALID_ENTITY;
    };
    const auto getArms = [](const vector<Entity>& entities)
    {
        vector<Entity> arms;
        for (const auto entity : entities)
        {
            auto &name = componentFromEntity<NameComponent>(entity)->name;
            auto prefix = name.substr(0, 3);
            if (prefix == "Arm" && name.length() == 4)
                arms.push_back(entity);
        }
        return arms;
    };
    const auto getLimbs = [](vector<Entity> arms)
    {
		array<array<Entity, LIMB_SEGMENTS>, LIMB_COUNT> limbs;
		transform(arms.begin(), arms.end(), limbs.begin(),
			[](auto entity) {
				array<Entity, LIMB_SEGMENTS> bones{};
				Entity parent = entity;
				for (size_t i = 0; i < LIMB_SEGMENTS; i++)
				{
					vector<Entity> childEntities = getEntitiesForParent<TransformComponent>(parent);
					if (childEntities.empty()) break;
					Entity childEntity = childEntities.front();
					bones[i] = childEntity;
					parent = childEntity;
				}
				return bones;
			});
        return limbs;
    };
    const auto namedEntitiesUnderOctopusScene = getEntitiesForParent<NameComponent>(octopusScene);
    const auto octopus = getOctopus(namedEntitiesUnderOctopusScene);
    const auto namesUnderOctopus = getEntitiesForParent<NameComponent>(octopus);
    const auto armature = getArmature(namesUnderOctopus);
    const auto namesUnderArmature = getEntitiesForParent<NameComponent>(armature);
    const auto armsEntities = getArms(namesUnderArmature);
    return getLimbs(armsEntities);
}

struct Octopus {

	array<array<Entity, LIMB_SEGMENTS>, LIMB_COUNT> limbs;
	vector<XMFLOAT3> relativeTargetGoals;
	XMMATRIX previousOctopusMatrix;

	struct Target {
		XMFLOAT3 position;
		float time;
	};

	vector<vector<Target>> targets;

	Entity octopusScene;

	Octopus() {

	}

	Octopus(
		Entity octopusScene
	): octopusScene(octopusScene) {
		limbs = findLimbsFromScene(octopusScene);
        previousOctopusMatrix = localToGlobalMatrix(getAncestryForEntity(octopusScene));

		// Size limb bones down closer to the tips
		int limbIndex = 0;
		for (const auto& limb : limbs)
		{

			int boneIndex = 0;
			for (auto boneEnt : limb)
			{
				const auto bone = mutableComponentFromEntity<TransformComponent>(boneEnt);
				bone->SetDirty();
				auto scale = XMLoadFloat3(&bone->scale_local);
				scale = XMVectorScale(scale, 1.0f / ((float)boneIndex * 0.05f + 1));
				XMStoreFloat3(&bone->scale_local, scale);
				boneIndex++;
			}
			limbIndex++;
		}

		// Initialize targets to tips
		const auto octopusMatrix = localToGlobalMatrix(getAncestryForEntity(octopusScene));
		for (const auto& limb : limbs)
		{
			const Entity lastBone = limb[limb.size() - 1];
			const auto matrix = localToGlobalMatrix(getAncestryForParentChild(octopusScene, lastBone));
			XMFLOAT3 goal{};
			XMStoreFloat3(&goal, XMVector4Transform({ 0, 0, 0, 1 }, matrix));
			goal.x *= 0.75;
			goal.y = -0.5;
			goal.z *= 0.75;
			relativeTargetGoals.push_back(goal);
			XMFLOAT3 target{};
			XMStoreFloat3(&target, XMVector4Transform({ goal.x, goal.y, goal.z, 1 }, octopusMatrix));
			targets.push_back({ { target, 0 } });
		}

		// Hard-coding octopus coloration for now
		GetScene().materials.Update(findWithName("Material"), {
			.baseColor = XMFLOAT4(153 / 255.0f, 164 / 255.0f, 255 / 255.0f, 1.0f),
			.subsurfaceScattering = XMFLOAT4(255 / 255.0f, 111 / 255.0f, 0 / 255.0f, 0.25f)
			});

	}

	void SimpleDance(float time)
	{
		int limbIndex = 0;
		for (const auto& limb : limbs)
		{
			int boneIndex = 0;
			for (const auto boneEnt : limb)
			{
				const auto bone = mutableComponentFromEntity<TransformComponent>(boneEnt);
				XMVECTOR quat{ 0, 0, 0, 1 };
				
				const float angle = sin(time + limbIndex + boneIndex * 0.05f) * 0 * 3.14f / 180.0f;
				const auto x = XMQuaternionRotationNormal({ 1, 0, 0 }, angle);
				quat = XMQuaternionMultiply(x, quat);
				quat = XMQuaternionNormalize(quat);
				XMStoreFloat4(&bone->rotation_local, quat);
				boneIndex++;
			}
			limbIndex++;
		}
	}


	void retargetting(float time)
	{
		const auto octopusMatrix = localToGlobalMatrix(getAncestryForEntity(octopusScene));

		struct Data {
			XMVECTOR newTarget;
			XMVECTOR targetDelta;
			XMVECTOR historyDelta;
			float targetHistoryDot;
		};
		array<Data, LIMB_COUNT> data{};
		for (size_t i = 0; i < LIMB_COUNT; i++)
		{
			const auto& limb = limbs[i];
			const auto& goal = relativeTargetGoals[i];
			const Entity lastBone = limb[limb.size() - 1];
			const auto matrix = localToGlobalMatrix(getAncestryForParentChild(octopusScene, lastBone));
			const auto newTarget = XMVector4Transform({ goal.x, goal.y, goal.z, 1 }, octopusMatrix);
			const auto previousTarget = XMVector4Transform({ goal.x, goal.y, goal.z, 1 }, previousOctopusMatrix);
			const auto historyDelta = newTarget - previousTarget;
			const auto targetDelta = newTarget - XMLoadFloat3(&targets[i][targets[i].size() - 1].position);
			const auto targetHistoryDot = XMVectorGetX(XMVector3Dot(historyDelta, targetDelta)); // If dot is positive, then we are on the end of a stride, if this number is negative we are at the start
			data[i] = {
				newTarget,
				targetDelta,
				historyDelta,
				targetHistoryDot,
			};
		}
		for (size_t i = 0; i < LIMB_COUNT; i++)
		{
			const auto neighborIndex = (i + 1) % LIMB_COUNT;
			const auto dominant = i % 2 == 0;
			const auto historyDistance = XMVectorGetX(XMVector3Length(data[i].historyDelta));
			const auto targetDistance = XMVectorGetX(XMVector3Length(data[i].targetDelta));
			if (data[neighborIndex].targetHistoryDot >= 0
				&& data[i].targetHistoryDot > 0
				&& (data[i].targetHistoryDot > data[neighborIndex].targetHistoryDot
					|| dominant && abs(data[i].targetHistoryDot - data[neighborIndex].targetHistoryDot) < FLT_EPSILON)
				&& targetDistance > FLT_EPSILON)
			{
				const auto &nextTarget = data[i].newTarget;
				const auto usedDelta = historyDistance > FLT_EPSILON
					? data[i].historyDelta / historyDistance
					: data[i].targetDelta / targetDistance;
				XMFLOAT3 position{};
				XMStoreFloat3(&position, nextTarget + usedDelta * strideLength);
				targets[i].push_back({ position, time });
			}
		}

		previousOctopusMatrix = octopusMatrix;
	}

    struct Segment {
        float length;
    };

    static auto getRelativeTarget(XMFLOAT3 target, Entity bone) {
        const auto matrix = localToGlobalMatrix(getAncestryForEntity(bone));
        XMVECTOR S, R, start;
        XMMatrixDecompose(&S, &R, &start, matrix);
        return XMVECTOR{ target.x, target.y, target.z, 1 } - start;
    }

    static auto getSegments(array<Entity, LIMB_SEGMENTS> bones) {
        size_t boneIndex = 0;
		array<Segment, LIMB_SEGMENTS> segments{};
        for (size_t i = 0; i < bones.size(); i ++)
        {
			const auto& bone = bones[i];
            if (boneIndex >= bones.size() - 1)
            {
                segments[i] = { .01f };
                break;
            }
            const auto first = componentFromEntity<TransformComponent>(bone)->GetPosition();
            const auto second = componentFromEntity<TransformComponent>(bones[boneIndex + 1])->GetPosition();
            const auto firstPosition = XMLoadFloat3(&first);
            const auto secondPosition = XMLoadFloat3(&second);
            const auto length = wiMath::Distance(firstPosition, secondPosition);
			segments[i] = { length };
            boneIndex++;
        }
        return segments;
    }

    static void curlTips(const array<Entity, LIMB_SEGMENTS>& bones, XMVECTOR relativeTarget) {
        const auto segments = getSegments(bones);
        const float targetDistance = XMVectorGetX(XMVector3Length(relativeTarget));
        float distanceTravelled = 0;
        int boneIndex = 0;
        for (const auto boneEnt : bones)
        {
            const auto bone = mutableComponentFromEntity<TransformComponent>(boneEnt);
            const auto rotation = min(max(distanceTravelled - targetDistance, 0.f), 1.f);
            XMVECTOR rotation_local = XMQuaternionRotationRollPitchYaw(rotation * 60 * 3.14f / 180.f, 0, 0);
            XMStoreFloat4(&bone->rotation_local, rotation_local);
            distanceTravelled += segments[boneIndex].length;
            boneIndex++;
        }
    }

    void basicWalk(float time)
	{
		const auto octopusMatrix = localToGlobalMatrix(getAncestryForEntity(octopusScene));

		int boneIndex = 0;
		for (auto &bones : limbs)
		{
			auto getProgress = [time](Target target) {
				return (time - target.time) * smoothSpeed;
			};

			// Cull old unused targets for limb
			{
				const auto result = find_if(targets[boneIndex].rbegin(), targets[boneIndex].rend(),
					[getProgress](Target what) {
						return getProgress(what) >= 1;
					});
				if (result != targets[boneIndex].rend() - 1)
				{
					targets[boneIndex].erase(targets[boneIndex].begin(), (result + 1).base());
				}
			}

			// Calculate smoothed version of targets for limb
			XMVECTOR smoothed = XMLoadFloat3(&targets[boneIndex][0].position);
			const auto octopusPosition = XMVector3Transform({ 0, 0, 0, 1 }, octopusMatrix);
			for (int i = 1; i < targets[boneIndex].size(); i++)
			{
				const auto &target = targets[boneIndex][i];
				const auto progress = easeOutBack(getProgress(target));
				smoothed = XMVectorLerp(smoothed, XMLoadFloat3(&target.position), progress); // Target influence
				smoothed = XMVectorLerp(smoothed, octopusPosition + XMVECTOR{ 0, strideLength * .25f, 0}, easeOutCirc(1 - abs(progress - 0.5f) * 2) * .5f); // Pull-in influence
			}
			XMFLOAT3 smoothTarget{};
			XMStoreFloat3(&smoothTarget, smoothed);	 
			auto relativeTarget = getRelativeTarget(smoothTarget, bones[0]);

            curlTips(bones, relativeTarget);

			// Rotate base of limbs to targets
			{
				const Entity parentEnt = componentFromEntity<HierarchyComponent>(bones[0])->parentID;
				const XMMATRIX localToGlobal = localToGlobalMatrix(getAncestryForEntity(parentEnt));
				const XMMATRIX globalToLocal = XMMatrixInverse(nullptr, localToGlobal);
				const XMVECTOR localStartDirection{ 0, 1, 0 };
				const XMVECTOR localTargetDirection = XMVector3Normalize(XMVector4Transform(relativeTarget, globalToLocal));
				const XMVECTOR axis = XMVector3Normalize(XMVector3Cross(localStartDirection, localTargetDirection));
				const float angle = XMScalarACos(XMVectorGetX(XMVector3Dot(localStartDirection, localTargetDirection)));
				const XMVECTOR localRotationTowardsTarget = XMQuaternionRotationNormal(axis, angle);
				auto bone = mutableComponentFromEntity<TransformComponent>(bones[0]);
				XMStoreFloat4(&bone->rotation_local, localRotationTowardsTarget);
			}

			boneIndex++;
		}
	}

	void Update(float time)
	{
        retargetting(time);
		basicWalk(time);
	}
};
