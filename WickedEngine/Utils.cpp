#include "Utils.h"

using namespace std;
using namespace wiECS;
using namespace wiScene;

bool utils::IsAncestorOfEntity(const Entity& potentialAncestor, const Entity& entity) {
	auto hierarchyComponent = GetScene().hierarchy.GetComponent(entity);
	if (hierarchyComponent == nullptr) { return false; }
	auto parent = hierarchyComponent->parentID;
	while (parent != INVALID_ENTITY)
	{
		if (potentialAncestor == parent) { return true; }
		hierarchyComponent = GetScene().hierarchy.GetComponent(parent);
		if (hierarchyComponent == nullptr) { return false; }
		parent = hierarchyComponent->parentID;
	}
	return false;
}

[[maybe_unused]]
Entity utils::FindOffspringWithName(const Entity& entity, const string& name) {
	const ComponentManager<NameComponent>& manager = GetScene().GetManager<NameComponent>();
	for (size_t i = 0; i < manager.GetCount(); i++) {
		const auto ent = manager.GetEntity(i);
		if (!IsAncestorOfEntity(entity, ent)) { continue; }
		const auto nameComponent = manager[i];
		if (nameComponent.name == name) { return ent; }
	}
	return INVALID_ENTITY;
}

Entity utils::FindWithName(const string& name) {
	const ComponentManager<NameComponent>& manager = GetScene().GetManager<NameComponent>();
	for (size_t i = 0; i < manager.GetCount(); i++) {
		const auto nameComponent = manager[i];
		if (nameComponent.name == name) { return manager.GetEntity(i); }
	}
	return INVALID_ENTITY;
}

vector<Entity> utils::GetAncestryForEntity(const Entity& child)
{
	vector<Entity> ancestry{};
	Entity next = child;
	while (next != INVALID_ENTITY) {
		ancestry.push_back(next);
		const auto component = GetForEntity<HierarchyComponent>(next);
		if (component == nullptr) { break; }
		next = component->parentID;
	}
	return vector<Entity>(ancestry.rbegin(), ancestry.rend());
}

vector<Entity> utils::GetAncestryForParentChild(const Entity& parent, const Entity& child)
{
	vector<Entity> ancestry{};
	Entity next = child;
	while (next != INVALID_ENTITY && next != parent) {
		ancestry.push_back(next);
		const auto component = GetForEntity<HierarchyComponent>(next);
		if (component == nullptr) { break; }
		next = component->parentID;
	}
	return vector<Entity>(ancestry.rbegin(), ancestry.rend());
}

XMMATRIX utils::LocalToGlobalMatrix(const vector<Entity>& ancestry)
{
	XMMATRIX result = XMMatrixIdentity();
	for (auto ancestor : ancestry) {
		const auto transform = GetForEntity<TransformComponent>(ancestor);
		if (transform == nullptr) continue;
		const XMVECTOR S_local = XMLoadFloat3(&transform->scale_local);
		const XMVECTOR R_local = XMLoadFloat4(&transform->rotation_local);
		const XMVECTOR T_local = XMLoadFloat3(&transform->translation_local);
		const auto matrix =
			XMMatrixScalingFromVector(S_local) *
			XMMatrixRotationQuaternion(R_local) *
			XMMatrixTranslationFromVector(T_local);
		result = matrix * result;
	}
	return result;
}

// Can rely on assumption that hierarchy elements are sorted from ancestors <-> dependence to call `UpdateTransform` on all transform components in the hierarchical order This method demonstrates the current intention of WickedEngine's TransformComponent but my thinking now is to replace this with a more transparent toolkit of functions that lay out a bit better what's happening.

[[maybe_unused]]
void utils::BruteRecalculateAllMatrices()
{
	for (size_t i = 0; i < GetScene().hierarchy.GetCount(); i++) {
		const auto hierarchy = GetScene().hierarchy[i];
		const auto entity = GetScene().hierarchy.GetEntity(i);
		const auto transform = GetScene().transforms.GetComponent(entity);
		const auto parentTransform = GetScene().transforms.GetComponent(hierarchy.parentID);
		if (transform == nullptr)
		{
			continue;
		}
		if (parentTransform == nullptr)
		{
			transform->SetDirty(true);
			transform->UpdateTransform();
			continue;
		}
		transform->UpdateTransform_Parented(*parentTransform);
	}
}
