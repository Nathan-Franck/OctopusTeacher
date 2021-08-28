#pragma once

#include "utils.h"
#include <vector>
#include <algorithm>

using namespace std;
using namespace wiECS;
using namespace wiScene;

struct Octopus {

	std::vector<std::vector<Entity>> limbs;
	Entity grabTarget;
	Entity octopusScene;
	Entity armature;

	vector<vector<Entity>> FindLimbsFromScene()
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
		auto octopusEntity = getOctopus(namedEntsUnderOctopusScene);
		auto namesUnderOctopus = getEntitiesForParent<NameComponent>(octopusEntity);
		armature = getArmature(namesUnderOctopus);
		auto namesUnderArmature = getEntitiesForParent<NameComponent>(armature);
		auto armsEntities = getArms(namesUnderArmature);
		return getLimbs(armsEntities);
	}

	Octopus() {

	}

	Octopus(
		Entity grabTarget,
		Entity octopusScene
	) {
		this->grabTarget = grabTarget;
		this->octopusScene = octopusScene;
		limbs = FindLimbsFromScene();

		// Size limb bones down closer to the tips
		int limbIndex = 0;
		for (auto limb : limbs)
		{
			//auto lastBoneEnt = limb[limb.size() - 1];
			//GetScene().GetManager<InverseKinematicsComponent>()->Create(lastBoneEnt);
			//GetScene().GetManager<InverseKinematicsComponent>()->Update(lastBoneEnt, { .target = grabTarget, .chain_length = (uint)(limb.size() - 1) });

			auto bones = limb | std::views::transform(mutableComponentFromEntity<TransformComponent>);
			int boneIndex = 0;
			for (auto bone : bones)
			{
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
			auto bones = limb | std::views::transform(mutableComponentFromEntity<TransformComponent>);
			int boneIndex = 0;
			for (auto bone : bones)
			{
				XMVECTOR quat{ 0, 0, 0, 1 };
				
				float angle = sin(time + limbIndex + boneIndex * 0.05f) * 10 * 3.14f / 180.0f;
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
		for (auto bones : limbs)
		{
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

			boneIndex = 0;
			float distanceTravelled = 0;
			auto target = componentFromEntity<TransformComponent>(grabTarget)->GetPosition();
			auto ancestry = getAncestryForEntity(bones[0]);
			auto matrix = resolveMatrix(ancestry);
			XMVECTOR S, R, start;
			XMMatrixDecompose(&S, &R, &start, matrix);
			auto relativeTarget = XMLoadFloat3(&target) - start;
			float targetDistance = XMVectorGetX(XMVector3Length(relativeTarget));
			for (auto boneEnt : bones) {
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

			{
				auto trans = mutableComponentFromEntity<TransformComponent>(armature);
				trans->RotateRollPitchYaw(XMFLOAT3{ .001, 0, 0 });
			}

			{
				auto bone = mutableComponentFromEntity<TransformComponent>(bones[0]);
				const Entity parentEnt = componentFromEntity<HierarchyComponent>(bones[0])->parentID;
				auto ancestry = getAncestryForEntity(parentEnt);
				auto matrix = resolveMatrix(ancestry);
				XMVECTOR S, globalRotation, T;
				XMMatrixDecompose(&S, &globalRotation, &T, matrix);
				const XMVECTOR inverseRotation = XMQuaternionInverse(globalRotation);
				const XMVECTOR localStartDirection{ 0, 1, 0 };
				const XMVECTOR localTargetDirection = XMVector3Normalize(XMVector3Rotate(relativeTarget * XMVECTOR{ 1, 1, -1 }, inverseRotation));
				const XMVECTOR axis = XMVector3Normalize(XMVector3Cross(localStartDirection, localTargetDirection));
				const float angle = XMScalarACos(XMVectorGetX(XMVector3Dot(localStartDirection, localTargetDirection)));
				const XMVECTOR localRotationTowardsTarget = XMQuaternionRotationNormal(axis, angle);
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
