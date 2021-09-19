#pragma once

#include <sstream>
#include <string>
#include <ranges>
#include "utils.h"
#include "Octopus.h"

class Game
{
	OctopusBehaviour octopusBehaviour;
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
		octopusBehaviour = OctopusBehaviour(octopusEntity);
	}
	void Update(float dt)
	{
		time += dt;

		octopusBehaviour.Update(time);
	}
};
