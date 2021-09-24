#pragma once

#include <sstream>
#include <string>
#include <ranges>
#include "WickedEngine.h"
#include "utils.h"
#include "OctopusComponent.h"
#include "ParticleLocomotion.h"

class Game
{
public:
	OctopusComponent octopusComponent;
	ParticleLocomotion particle { .radius = 0.01f, .position = { 0, 0, 5, 1 } };
	Entity testTarget; 
	float time = 0.0f;
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
		const auto transform = Utils::GetMutableForEntity<TransformComponent>(testTarget);
		transform->translation_local = { 0, 0, 15 };
		transform->UpdateTransform();

		const auto getOctopus = [](const vector<Entity>& entities)
		{
			for (const auto entity : entities)
				if (Utils::GetForEntity<NameComponent>(entity)->name == "OctopusRiggedTopo.glb")
					return entity;
			return INVALID_ENTITY;
		};
		const auto octopusEntity = getOctopus(Utils::Children<NameComponent>(octopusScene));
		octopusComponent = OctopusComponent(octopusEntity);

	}
	void Update(float dt);
	
};
