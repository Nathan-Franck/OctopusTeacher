#pragma once

#include <ranges>

using namespace std;
using namespace wiECS;
using namespace wiScene;

template<class T>
std::vector<Entity> getEntitiesForParent(Entity parent)
{
	std::vector<Entity> entities;
	for (int i = 0; i < GetScene().hierarchy.GetCount(); i++) {
		auto heirarchyComponent = GetScene().hierarchy[i];
		auto entity = GetScene().hierarchy.GetEntity(i);
		if (heirarchyComponent.parentID == parent) {
			auto manager = GetScene().GetManager<T>();
			auto component = manager->GetComponent(entity);
			if (component != nullptr) {
				entities.push_back(entity);
			}
		}
	}
	return entities;
}

template <class T>
T* componentFromEntity(Entity ent) {
	return GetScene().GetManager<T>()->GetComponent(ent);
}

bool isAncestorOfEntity(Entity entity, Entity ancestor) {
	auto heirarchyComponent = GetScene().hierarchy.GetComponent(ancestor);
	if (heirarchyComponent == nullptr) { return false; }
	Entity parent = heirarchyComponent->parentID;
	while (parent != wiECS::INVALID_ENTITY)
	{
		if (entity == parent) { return true; }
		ancestor = parent;
		auto heirarchyComponent = GetScene().hierarchy.GetComponent(ancestor);
		if (heirarchyComponent == nullptr) { return false; }
		parent = heirarchyComponent->parentID;
	}
	return false;
}

Entity findOffspringWithName(Entity entity, string name) {
	auto manager = GetScene().GetManager<NameComponent>();
	for (int i = 0; i < manager->GetCount(); i++) {
		auto ent = manager->GetEntity(i);
		if (!isAncestorOfEntity(entity, ent)) { continue; }
		auto nameComponent = (*manager)[i];
		if (nameComponent.name.compare(name) == 0) { return ent;  }
	}
	return wiECS::INVALID_ENTITY;
}

Entity findWithName(string name) {
	auto manager = GetScene().GetManager<NameComponent>();
	for (int i = 0; i < manager->GetCount(); i++) {
		auto nameComponent = (*manager)[i];
		if (nameComponent.name.compare(name) == 0) { return manager->GetEntity(i); }
	}
	return wiECS::INVALID_ENTITY;
}

vector<vector<Entity>> getLimbsForOctopusScene(Entity octopusScene)
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
