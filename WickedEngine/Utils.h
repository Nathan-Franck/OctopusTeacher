#pragma once

#include "WickedEngine.h"

#include <ranges>
#include <tuple>
#include <type_traits>
#include <string>
#include <iostream>

using namespace std;
using namespace wiECS;
using namespace wiScene;

namespace utils
{

	template<class T>
	vector<Entity> Children(const Entity& parent)
	{
		vector<Entity> entities;
		for (size_t i = 0; i < GetScene().hierarchy.GetCount(); i++) {
			const auto hierarchyComponent = GetScene().hierarchy[i];
			const auto entity = GetScene().hierarchy.GetEntity(i);
			if (hierarchyComponent.parentID == parent) {
				const ComponentManager<T>& manager = GetScene().GetManager<T>();
				const auto component = manager.GetComponent(entity);
				if (component != nullptr) {
					entities.push_back(entity);
				}
			}
		}
		return entities;
	}

	template<class T>
	const T* GetForEntity(const Entity& ent) {
		return GetScene().GetManager<T>().GetComponent(ent);
	}

	template<class T>
	T* GetMutableForEntity(const Entity& ent) {
		auto component = GetScene().GetManager<T>().GetComponent(ent);
		Scene::WhenMutable(*component);
		return component;
	}

	bool IsAncestorOfEntity(const Entity& potentialAncestor, const Entity& entity);

	[[maybe_unused]] Entity FindOffspringWithName(const Entity& entity, const string& name);

	Entity FindWithName(const string& name);

	vector<Entity> GetAncestryForEntity(const Entity& child);

	vector<Entity> GetAncestryForParentChild(const Entity& parent, const Entity& child);

	XMMATRIX LocalToGlobalMatrix(const vector<Entity>& ancestry);

	// Can rely on assumption that hierarchy elements are sorted from ancestors <-> dependence to call `UpdateTransform` on all transform components in the hierarchical order This method demonstrates the current intention of WickedEngine's TransformComponent but my thinking now is to replace this with a more transparent toolkit of functions that lay out a bit better what's happening.
	[[maybe_unused]] void BruteRecalculateAllMatrices();
};
