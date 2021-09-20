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

class Utils
{

public:

	template<class T>
	static vector<Entity> children(const Entity& parent);

	template <class T>
	static const T* getForEntity(const Entity& ent);

	template <class T>
	static T* getMutableForEntity(const Entity& ent);

	static bool isAncestorOfEntity(const Entity& potentialAncestor, const Entity& entity);

	[[maybe_unused]] static Entity findOffspringWithName(const Entity& entity, const string& name);

	static Entity findWithName(const string& name);

	static vector<Entity> getAncestryForEntity(const Entity& child);

	static vector<Entity> getAncestryForParentChild(const Entity& parent, const Entity& child);

	static XMMATRIX localToGlobalMatrix(const vector<Entity>& ancestry);

	// Can rely on assumption that hierarchy elements are sorted from ancestors <-> dependence to call `UpdateTransform` on all transform components in the hierarchical order This method demonstrates the current intention of WickedEngine's TransformComponent but my thinking now is to replace this with a more transparent toolkit of functions that lay out a bit better what's happening.
	[[maybe_unused]] static void bruteRecalculateAllMatrices();
};
