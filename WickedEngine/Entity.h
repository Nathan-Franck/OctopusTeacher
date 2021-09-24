#pragma once
#include <tuple>
#include <iostream>
#include <algorithm>
#include <list>

namespace EntityHelper {

	template<typename T, typename Tuple>
	struct tuple_element_index_helper;

	template<typename T>
	struct tuple_element_index_helper<T, std::tuple<>> {
		static constexpr std::size_t value = 0;
	};

	template<typename T, typename... Rest>
	struct tuple_element_index_helper<T, std::tuple<T, Rest...>> {
		static constexpr std::size_t value = 0;
		using RestTuple = std::tuple<Rest...>;
		static_assert(
			tuple_element_index_helper<T, RestTuple>::value ==
			std::tuple_size_v<RestTuple>,
			"type appears more than once in tuple");
	};

	template<typename T, typename First, typename... Rest>
	struct tuple_element_index_helper<T, std::tuple<First, Rest...>> {
		using RestTuple = std::tuple<Rest...>;
		static constexpr std::size_t value = 1 +
			tuple_element_index_helper<T, RestTuple>::value;
	};

	template<typename T, typename Tuple>
	struct tuple_element_index {
		static constexpr std::size_t value =
			tuple_element_index_helper<T, Tuple>::value;
		static_assert(value < std::tuple_size_v<Tuple>,
			"type does not appear in tuple");
	};

	template<typename T, typename Tuple>
	inline constexpr std::size_t tuple_element_index_v
		= tuple_element_index<T, Tuple>::value;

	template<typename T, typename Tuple>
	concept InTuple = EntityHelper::tuple_element_index_helper<T, Tuple>::value < std::tuple_size_v<Tuple>;

	template<typename T, typename Tuple>
	concept OutOfTuple = !InTuple<T, Tuple>;
}

template<typename... T>
class Ent {
private:

	template<EntityHelper::InTuple<tuple<T...>> Component>
	auto set(Component component) {
		constexpr std::size_t index =
			EntityHelper::tuple_element_index_v<Component, std::tuple<T...>>;
		std::get<index>(components) = component;
		return components;
	}

	template<EntityHelper::InTuple<tuple<T...>>... Component>
	auto set_many(tuple<Component...> toSet) {
		return std::apply([this](auto... component) {
			std::tuple<T...> result = components;
			((result = Ent{ result }.set<Component>(component)), ...);
			return result;
			}, toSet);
	}

public:
	tuple<T...> components;

	explicit Ent(T... args) : components{ std::make_tuple(args...) } {}
	explicit Ent(std::tuple<T...> arg) : components{ arg } {}

	template<typename... Component>
	auto merge(tuple<Component...> toMerge) {
		return std::apply([this](auto... component) {
			auto toSet =
				std::tuple_cat(
					std::get<EntityHelper::InTuple<Component, decltype(components)> ? 0 : 1>(
						std::make_tuple(
							std::make_tuple(component),
							std::make_tuple()))...);
			return std::tuple_cat(
				set_many(toSet),
				std::tuple_cat(
					std::get<EntityHelper::OutOfTuple<Component, decltype(components)> ? 0 : 1>(
						std::make_tuple(
							std::make_tuple(component),
							std::make_tuple()))...));
			}, toMerge);
	}

	template<typename... Component>
	auto merge(Component... toMerge) {
		return merge(make_tuple(toMerge...));
	}

	template<typename Component>
	Component get() {
		constexpr std::size_t index =
			EntityHelper::tuple_element_index_v<Component, std::tuple<T...>>;
		return std::get<index>(components);
	}

	template<typename... Component>
	std::tuple<Component...> prune() {
		return std::make_tuple(get<Component>()...);
	}

	template<typename... Components>
	operator tuple<Components...>() {
		return prune<Components...>();
	}
};

namespace EntityTester
{

	struct Physics {
		XMFLOAT2 position;
		XMFLOAT2 velocity;
	};

	struct Health {
		int max;
		int current;
	};

	struct Glutes {
		int special;
	};

	enum class DomainSpec {
		First,
		Second,
		Nice
	};

	template<auto DomainInfo>
	struct Lats {
	};

	int getHealthCurrent(tuple<Health, Physics> components) {
		const auto [health, physics] = components;
		return health.current;
	}

	void test() {
		tuple components3(
			Glutes{ 2 }
		);
		auto components = Ent{ tuple(
			Physics{ {0, 0}, {10, 10} },
			Health{ 100, 100 }
		) }.merge(components3);
		auto physics = Ent{ components }.get<Physics>();
		physics.velocity = { 32, 32 };
		components = Ent{ components }.merge(physics);

		const auto x = getHealthCurrent(Ent{ components });
		std::cout << x << std::endl;

		{
			auto [physics, health] = Ent{ components }.prune<Physics, Health>();
			health.current -= 10;
			auto result2 = Ent{ components }.merge(Glutes{ 20 }, health);
			auto result4 = Ent{ result2 }.merge(Glutes{ 30 }, 2, 3.14);
			auto [physics2, health2, glutes2] = Ent{ result4 }.prune<Physics, Health, Glutes>();

			std::cout << physics2.position.x << std::endl;
			std::cout << physics2.velocity.x << std::endl;
			std::cout << health2.current << std::endl;
			std::cout << glutes2.special << std::endl;
		}
	}
}
