#pragma once
#include <tuple>
#include <iostream>
#include <algorithm>
#include <list>

namespace TypeSetUtils {

	template<typename T, typename Tuple>
	struct tuple_element_index_helper;

	template<typename T>
	struct tuple_element_index_helper<T, std::tuple<>>
	{
		static constexpr std::size_t value = 0;
	};

	template<typename T, typename... Rest>
	struct tuple_element_index_helper<T, std::tuple<T, Rest...>>
	{
		static constexpr std::size_t value = 0;
		using RestTuple = std::tuple<Rest...>;
		static_assert(
			tuple_element_index_helper<T, RestTuple>::value ==
			std::tuple_size_v<RestTuple>,
			"type appears more than once in tuple");
	};

	template<typename T, typename First, typename... Rest>
	struct tuple_element_index_helper<T, std::tuple<First, Rest...>>
	{
		using RestTuple = std::tuple<Rest...>;
		static constexpr std::size_t value = 1 +
			tuple_element_index_helper<T, RestTuple>::value;
	};

	template<typename T, typename Tuple>
	struct tuple_element_index
	{
		static constexpr std::size_t value =
			tuple_element_index_helper<T, Tuple>::value;
		static_assert(value < std::tuple_size_v<Tuple>,
			"type does not appear in tuple");
	};

	template<typename T, typename Tuple>
	inline constexpr std::size_t tuple_element_index_v
		= tuple_element_index<T, Tuple>::value;

	template<typename T, typename Tuple>
	concept InTuple = TypeSetUtils::tuple_element_index_helper<T, Tuple>::value < std::tuple_size_v<Tuple>;

	template<typename T, typename Tuple>
	concept OutOfTuple = !InTuple<T, Tuple>;
}

template<typename... T>
class TypeSet
{
private:

	template<typename Component>
	tuple<T...> Set(Component component)
	{
		constexpr std::size_t index =
			TypeSetUtils::tuple_element_index_v<Component, std::tuple<T...>>;
		std::get<index>(components) = component;
		return components;
	}

	template<typename... Component>
	tuple<T...> SetMany(tuple<Component...> toSet)
	{
		return std::apply([this](auto... component)
			{
				std::tuple<T...> result = components;
				((result = TypeSet{ result }.Set<Component>(component)), ...);
				return result;
			}, toSet);
	}

public:
	tuple<T...> components;

	explicit TypeSet(T... args) : components{ std::make_tuple(args...) } {}
	explicit TypeSet(std::tuple<T...> arg) : components{ arg } {}

	template<typename... Component>
	auto Merge(tuple<Component...> toMerge)
	{
		return std::apply([this](auto... component) {
				auto toSet =
					std::tuple_cat(
						std::get<TypeSetUtils::InTuple<Component, decltype(components)> ? 0 : 1>(
							std::make_tuple(
								std::make_tuple(component),
								std::make_tuple()))...);
				return std::tuple_cat(
					SetMany(toSet),
					std::tuple_cat(
						std::get<TypeSetUtils::OutOfTuple<Component, decltype(components)> ? 0 : 1>(
							std::make_tuple(
								std::make_tuple(component),
								std::make_tuple()))...));
			}, toMerge);
	}

	template<typename... Component>
	auto Merge(Component... toMerge)
	{
		return Merge(make_tuple(toMerge...));
	}

	template<TypeSetUtils::InTuple<tuple<T...>> Component>
	Component Get()
	{
		constexpr std::size_t index =
			TypeSetUtils::tuple_element_index_v<Component, std::tuple<T...>>;
		return std::get<index>(components);
	}

	template<TypeSetUtils::InTuple<tuple<T...>>... Component>
	std::tuple<Component...> Pick()
	{
		return std::make_tuple(Get<Component>()...);
	}

	template<TypeSetUtils::InTuple<tuple<T...>>... Components>
	operator tuple<Components...>()
	{
		return Pick<Components...>();
	}
};

namespace TypeSetTester
{

	struct Physics
	{
		XMFLOAT2 position;
		XMFLOAT2 velocity;
	};

	struct Health
	{
		int max;
		int current;
	};

	struct Glutes
	{
		int special;
	};

	int GetHealthCurrent(tuple<Health> components)
	{
		const auto [health] = components;
		return health.current;
	}

	auto GetPositionX(tuple<Physics> components)
	{
		const auto [physics] = components;
		return physics.position;
	}

	void Test()
	{
		tuple components3(
			Glutes{ 2 }
		);
		auto components = TypeSet
		{
			Physics{ {0, 0}, {10, 10} },
			Health{ 100, 100 }
		}.Merge(components3);
		auto physics = TypeSet{ components }.Get<Physics>();
		physics.velocity = { 32, 32 };
		components = TypeSet{ components }.Merge(physics);

		const auto x = GetHealthCurrent(TypeSet{ components });
		const auto y = GetPositionX(TypeSet{ components });
		std::cout << x << std::endl;
		{
			auto [physics, health] = TypeSet{ components }.Pick<Physics, Health>();
			health.current -= 10;
			auto result2 = TypeSet{ components }.Merge(Glutes{ 20 }, health);
			auto result4 = TypeSet{ result2 }.Merge(Glutes{ 30 }, 2, 3.14);
			auto [physics2, health2, glutes2] = TypeSet{ result4 }.Pick<Physics, Health, Glutes>();

			std::cout << physics2.position.x << std::endl;
			std::cout << physics2.velocity.x << std::endl;
			std::cout << health2.current << std::endl;
			std::cout << glutes2.special << std::endl;
		}
	}
}
