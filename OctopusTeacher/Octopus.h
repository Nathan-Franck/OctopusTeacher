#pragma once

#include "utils.h"
#include <vector>
#include <algorithm>

using namespace std;
using namespace wiECS;
using namespace wiScene;

struct Octopus {

	std::vector<std::vector<Entity>> limbs;
	Entity octopusScene;
	Entity grabTarget;

	vector<vector<Entity>> FindLimbsFromScene(Entity octopusScene)
	{
		auto getOctopus = [](vector<Entity> entities)
		{
			for (auto entity : entities)
				if (componentFromEntity<NameComponent>(entity)->name.compare("OctopusRiggedTopo.glb") == 0)
					return entity;
		};
		auto getArmature = [](vector<Entity> entities)
		{
			for (auto entity : entities)
				if (componentFromEntity<NameComponent>(entity)->name.compare("Armature") == 0)
					return entity;
		};
		auto getArms = [](vector<Entity> entities)
		{
			vector<Entity> arms;
			for (auto entity : entities)
			{
				auto name = componentFromEntity<NameComponent>(entity)->name;
				auto prefix = name.substr(0, 3);
				if (prefix.compare("Arm") == 0 && name.length() == 4)
					arms.push_back(entity);
			}
			return arms;
		};
		auto getLimbs = [](vector<Entity> arms)
		{
			vector<vector<Entity>> limbs;
			std::transform(arms.begin(), arms.end(), std::back_inserter(limbs), [](auto entity){
				vector<Entity> bones;
				Entity parent = entity;
				while (true)
				{
					vector<Entity> childEntities = getEntitiesForParent<TransformComponent>(parent);
					if (childEntities.size() == 0) break;
					Entity childEntity = childEntities.front();
					bones.push_back(childEntity);
					parent = childEntity;
				}
				return bones;
			});
			return limbs;
		};
		auto namedEntsUnderOctopusScene = getEntitiesForParent<NameComponent>(octopusScene);
		auto octopus = getOctopus(namedEntsUnderOctopusScene);
		auto namesUnderOctopus = getEntitiesForParent<NameComponent>(octopus);
		auto armature = getArmature(namesUnderOctopus);
		auto namesUnderArmature = getEntitiesForParent<NameComponent>(armature);
		auto armsEntities = getArms(namesUnderArmature);
		return getLimbs(armsEntities);
	}

	Octopus() {

	}

	Octopus(
		Entity grabTarget,
		Entity octopusScene
	): octopusScene(octopusScene) {
		this->grabTarget = grabTarget;
		limbs = FindLimbsFromScene(octopusScene);

		// Size limb bones down closer to the tips
		int limbIndex = 0;
		for (auto limb : limbs)
		{

			int boneIndex = 0;
			for (auto boneEnt : limb)
			{
				auto bone = mutableComponentFromEntity<TransformComponent>(boneEnt);
				bone->SetDirty();
				auto scale = XMLoadFloat3(&bone->scale_local);
				scale = XMVectorScale(scale, 1.0 / (boneIndex * 0.05f + 1));
				XMStoreFloat3(&bone->scale_local, scale);
				boneIndex++;
			}
			limbIndex++;
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
		for (auto limb : limbs)
		{
			int boneIndex = 0;
			for (auto boneEnt : limb)
			{
				auto bone = mutableComponentFromEntity<TransformComponent>(boneEnt);
				XMVECTOR quat{ 0, 0, 0, 1 };
				
				float angle = sin(time + limbIndex + boneIndex * 0.05f) * 0 * 3.14f / 180.0f;
				auto x = XMQuaternionRotationNormal({ 1, 0, 0 }, angle);
				quat = XMQuaternionMultiply(x, quat);
				quat = XMQuaternionNormalize(quat);
				XMStoreFloat4(&bone->rotation_local, quat);
				boneIndex++;
			}
			limbIndex++;
		}
	}

	void BasicGrasp()
	{

		struct Segment {
			float length;
		};

		auto getRelativeTarget = [](Entity grabTarget, Entity bone) {
			auto target = componentFromEntity<TransformComponent>(grabTarget)->GetPosition();
			auto ancestry = getAncestryForEntity(bone);
			auto matrix = localToGlobalMatrix(ancestry);
			XMVECTOR S, R, start;
			XMMatrixDecompose(&S, &R, &start, matrix);
			return XMVECTOR{ target.x, target.y, target.z, 1 } - start;
		};

		auto getSegments = [](vector<Entity> bones) {
			int boneIndex = 0;
			vector<Segment> segments;
			for (auto bone : bones)
			{
				if (boneIndex >= bones.size() - 1)
				{
					segments.push_back({ .01f });
					break;
				}
				auto first = componentFromEntity<TransformComponent>(bone)->GetPosition();
				auto second = componentFromEntity<TransformComponent>(bones[boneIndex + 1])->GetPosition();
				auto firstPosition = XMLoadFloat3(&first);
				auto secondPosition = XMLoadFloat3(&second);
				auto length = wiMath::Distance(firstPosition, secondPosition);
				segments.push_back({ length });
				boneIndex++;
			}
			return segments;
		};

		for (auto bones : limbs)
		{
			auto relativeTarget = getRelativeTarget(grabTarget, bones[0]);

			{
				auto segments = getSegments(bones);
				float targetDistance = XMVectorGetX(XMVector3Length(relativeTarget));
				float distanceTravelled = 0;
				int boneIndex = 0;
				for (auto boneEnt : bones)
				{
					auto bone = mutableComponentFromEntity<TransformComponent>(boneEnt);
					XMVECTOR quat{ 0, 0, 0, 1 };
					XMVECTOR x;
					if (distanceTravelled > targetDistance)
					{
						x = XMQuaternionRotationRollPitchYaw(60 * 3.14f / 180.0f, 0, 0);
					}
					else
					{
						x = XMQuaternionRotationRollPitchYaw(0, 0, 0);
					}
					quat = XMQuaternionMultiply(x, quat);
					quat = XMQuaternionNormalize(quat);
					XMStoreFloat4(&bone->rotation_local, quat);
					distanceTravelled += segments[boneIndex].length;
					boneIndex++;
				}
			}

			{
				const Entity parentEnt = componentFromEntity<HierarchyComponent>(bones[0])->parentID;
				auto ancestry = getAncestryForEntity(parentEnt);
				auto localToGlobal = localToGlobalMatrix(ancestry);
				const XMMATRIX globalToLocal = XMMatrixInverse(nullptr, localToGlobal);
				const XMVECTOR localStartDirection{ 0, 1, 0 };
				const XMVECTOR localTargetDirection = XMVector3Normalize(XMVector4Transform(relativeTarget, globalToLocal));
				const XMVECTOR axis = XMVector3Normalize(XMVector3Cross(localStartDirection, localTargetDirection));
				const float angle = XMScalarACos(XMVectorGetX(XMVector3Dot(localStartDirection, localTargetDirection)));
				const XMVECTOR localRotationTowardsTarget = XMQuaternionRotationNormal(axis, angle);
				auto bone = mutableComponentFromEntity<TransformComponent>(bones[0]);
				XMStoreFloat4(&bone->rotation_local, localRotationTowardsTarget);
			}
		}
	}

	void Update(float time)
	{
		SimpleDance(time);
		BasicGrasp();
	}
};
