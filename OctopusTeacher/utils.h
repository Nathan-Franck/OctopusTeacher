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
const T* componentFromEntity(Entity ent) {
	return GetScene().GetManager<T>()->GetComponent(ent);
}

template <class T>
T* mutableComponentFromEntity(Entity ent) {
	auto component = GetScene().GetManager<T>()->GetComponent(ent);
	GetScene().WhenMutable(component);
	return component;
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
