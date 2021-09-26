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
	tuple<T...> set(Component component)
	{
		constexpr std::size_t index =
			TypeSetUtils::tuple_element_index_v<Component, std::tuple<T...>>;
		std::get<index>(components) = component;
		return components;
	}

	template<typename... Component>
	tuple<T...> set_many(tuple<Component...> toSet)
	{
		return std::apply([this](auto... component)
			{
				std::tuple<T...> result = components;
				((result = TypeSet{ result }.set<Component>(component)), ...);
				return result;
			}, toSet);
	}

public:
	tuple<T...> components;

	explicit TypeSet(T... args) : components{ std::tuple(args...) } {}
	explicit TypeSet(std::tuple<T...> arg) : components{ arg } {}

	template<typename... Component>
	auto merge(tuple<Component...> toMerge)
	{
		return std::apply([this](auto... component) {
				auto toSet =
					std::tuple_cat(
						std::get<TypeSetUtils::InTuple<Component, decltype(components)> ? 0 : 1>(
							std::tuple(
								std::tuple(component),
								std::tuple()))...);
				return std::tuple_cat(
					set_many(toSet),
					std::tuple_cat(
						std::get<TypeSetUtils::OutOfTuple<Component, decltype(components)> ? 0 : 1>(
							std::tuple(
								std::tuple(component),
								std::tuple()))...));
			}, toMerge);
	}

	template<typename... Component>
	auto merge(Component... toMerge)
	{
		return merge(make_tuple(toMerge...));
	}

	template<TypeSetUtils::InTuple<tuple<T...>> Component>
	const Component get()
	{
		constexpr std::size_t index =
			TypeSetUtils::tuple_element_index_v<Component, std::tuple<T...>>;
		return std::get<index>(components);
	}

	template<TypeSetUtils::InTuple<tuple<T...>>... Component>
	const std::tuple<Component...> pick()
	{
		return std::make_tuple(get<Component>()...);
	}

	template<TypeSetUtils::InTuple<tuple<T...>>... Components>
	operator tuple<Components...> const ()
	{
		return pick<Components...>();
	}

	template<typename... Components>
	auto operator+(TypeSet<Components...> rhs)
	{
		return merge(rhs.components);
	}

	template<typename... Components>
	friend auto operator+(tuple<Components...> lhs, TypeSet<T...> rhs)
	{
		TypeSet<Components...> typeSet(lhs);
		return typeSet.merge(rhs);
	}

	template<typename... Components>
	auto operator+(tuple<Components...> rhs)
	{
		return merge(rhs);
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
		}.merge(components3);
		auto physics = TypeSet{ components }.get<Physics>();
		physics.velocity = { 32, 32 };
		components = TypeSet{ components }.merge(physics);

		const auto x = GetHealthCurrent(TypeSet{ components });
		const auto y = GetPositionX(TypeSet{ components });
		std::cout << x << std::endl;
		{
			auto [physics, health] = TypeSet{ components }.pick<Physics, Health>();
			health.current -= 10;
			auto result2 = TypeSet{ components }.merge(Glutes{ 20 }, health);
			auto result4 = TypeSet{ result2 }.merge(Glutes{ 30 }, 2, 3.14);
			auto [physics2, health2, glutes2] = TypeSet{ result4 }.pick<Physics, Health, Glutes>();

			std::cout << physics2.position.x << std::endl;
			std::cout << physics2.velocity.x << std::endl;
			std::cout << health2.current << std::endl;
			std::cout << glutes2.special << std::endl;
		}

		auto result = TypeSet{ Physics {} } + (TypeSet{ Health{} } + TypeSet{ Physics{} });
	}
}
