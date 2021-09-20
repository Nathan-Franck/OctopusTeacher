#pragma once

#include <sstream>
#include <string>
#include <ranges>
#include "utils.h"
#include "OctopusComponent.h"
#include "ParticleLocomotion.h"

class Game
{
	OctopusComponent octopusComponent;
	ParticleLocomotion particle { .radius = 10, .position = { 0, 0, 5, 1 } };
	Entity testTarget; 
	float time = 0.0f;
private:

public:
	Game()
	{
		// 📷 Update the camera transform to have a nice top-down (orthographic-like) view
		{
			TransformComponent transform;
			XMStoreFloat4(&transform.rotation_local, XMQuaternionRotationRollPitchYaw(3.14 * .25, 0, 0));
			XMStoreFloat3(&transform.translation_local, { 0, 10, 0 });
			transform.UpdateTransform();
			wiScene::GetCamera().TransformCamera(transform);
		}

		const auto octopusScene = LoadModel("../CustomContent/Game.wiscene", XMMatrixTranslation(0, 0, 10), true);

		testTarget = GetScene().Entity_CreateObject("Tentacle Target");
		const auto transform = mutableComponentFromEntity<TransformComponent>(testTarget);
		transform->translation_local = { 0, 0, 15 };
		transform->UpdateTransform();

		const auto getOctopus = [](const vector<Entity>& entities)
		{
			for (const auto entity : entities)
				if (componentFromEntity<NameComponent>(entity)->name == "OctopusRiggedTopo.glb")
					return entity;
			return INVALID_ENTITY;
		};
		const auto octopusEntity = getOctopus(getEntitiesForParent<NameComponent>(octopusScene));
		octopusComponent = OctopusComponent(octopusEntity);

	}
	void Update(float dt)
	{
		time += dt;

		octopusComponent.Update(time);

		TransformComponent* transform = mutableComponentFromEntity<TransformComponent>(octopusComponent.octopusScene);
		const Entity parentEnt = componentFromEntity<HierarchyComponent>(octopusComponent.octopusScene)->parentID;
		const TransformComponent* parentTransform = componentFromEntity<TransformComponent>(parentEnt);
		const XMMATRIX localToGlobal = localToGlobalMatrix(getAncestryForEntity(parentEnt));
		const XMMATRIX globalToLocal = XMMatrixInverse(nullptr, localToGlobal);
		const XMFLOAT3 local = transform->translation_local;
		const XMVECTOR position = XMVector4Transform({ local.x, local.y, local.z, 1 }, localToGlobal);
		const XMVECTOR direction = position - particle.position;
		const XMFLOAT3 resFl = particle.MoveTowards(direction);
		const XMVECTOR result = { resFl.x, resFl.y, resFl.z, 1 };
		particle.position = result;
		XMStoreFloat3(&transform->translation_local, XMVector4Transform(result, globalToLocal));
		transform->UpdateTransform_Parented(*parentTransform);
	}
};
