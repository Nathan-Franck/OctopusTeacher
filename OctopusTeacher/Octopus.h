#pragma once

#include "utils.h"
#include <vector>

using namespace std;
using namespace wiECS;
using namespace wiScene;

struct Octopus {

	std::vector<std::vector<Entity>> limbs;
	Entity grabTarget;
	Entity octopusScene;

	vector<vector<Entity>> FindLimbsFromScene()
	{
		auto isOctopus = [](Entity entity)
		{
			return componentFromEntity<NameComponent>(entity)->name.compare("OctopusRiggedTopo.glb") == 0;
		};
		auto isArmature = [](Entity entity)
		{
			return componentFromEntity<NameComponent>(entity)->name.compare("Armature") == 0;
		};
		auto isArm = [](Entity entity)
		{
			auto name = componentFromEntity<NameComponent>(entity)->name;
			auto prefix = name.substr(0, 3);
			return prefix.compare("Arm") == 0 && name.length() == 4;
		};
		auto namedEntsUnderOctopusScene = getEntitiesForParent<NameComponent>(octopusScene);
		auto octopusEntity = namedEntsUnderOctopusScene | std::views::filter(isOctopus);
		auto namesUnderOctopus = getEntitiesForParent<NameComponent>(octopusEntity.front());
		auto armatureEntity = namesUnderOctopus | std::views::filter(isArmature);
		auto namesUnderArmature = getEntitiesForParent<NameComponent>(armatureEntity.front());
		auto armsEntities = namesUnderArmature | std::views::filter(isArm);
		auto result = armsEntities
			| std::views::transform([](Entity armEnt)
				{
					std::vector<Entity> bones{ }; //armEnt
					Entity parent = armEnt;
					while (true) {
						std::vector<Entity> childEntities = getEntitiesForParent<TransformComponent>(parent);
						if (childEntities.size() == 0) break;
						Entity childEntity = childEntities.front();
						bones.push_back(childEntity);
						parent = childEntity;
					}
					return bones;
				})
			| std::views::common;
				return std::vector(result.begin(), result.end());
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
				auto x = XMQuaternionRotationRollPitchYaw(sin(time + limbIndex + boneIndex * 0.05f) * 10 * 3.14f / 180.0f, 0, 0);
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

		auto bones = limbs[0];
		int boneIndex = 0;
		vector<Segment> segments;
		for (auto bone : bones) {
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
		auto start = componentFromEntity<TransformComponent>(limbs[0][0])->GetPosition();
		float targetDistance = wiMath::Distance(
			XMLoadFloat3(&target),
			XMLoadFloat3(&start)
		);
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
	}

	void Update(float time)
	{
		SimpleDance(time);
		BasicGrasp();
	}
};
