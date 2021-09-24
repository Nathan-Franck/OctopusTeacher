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
}

template<typename... T>
class TupleEntity : std::tuple<T...> {
private:
public:
	explicit TupleEntity(T... args) : std::tuple<T...>{ std::make_tuple(args...) } {}
	explicit TupleEntity(std::tuple<T...> tuple) : std::tuple<T...>{ tuple } {}

	template<typename Component>
	Component get() {
		constexpr std::size_t index =
			EntityHelper::tuple_element_index_v<Component, std::tuple<T...>>;
		return std::get<index>(*this);
	}

	template<typename... Component>
	std::tuple<Component...> get_many() {
		return std::make_tuple(get<Component>()...);
	}

	template<typename Component>
	void set(Component component) {
		constexpr std::size_t index =
			EntityHelper::tuple_element_index_v<Component, std::tuple<T...>>;
		std::get<index>(*this) = component;
	}

	template<typename... Component>
	void set_many(Component... component) {
		(set<Component>(component), ...);
	}

	tuple<T...> tuple = *this;
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

	template<typename... T>
	auto get_position_x(TupleEntity<T...> entity) {
		const auto thing = entity.get<Physics>();
		return thing.position.x;
	}


	void test() {
		auto entity = TupleEntity{
			Physics{XMFLOAT2{0, 0}, XMFLOAT2{10, 10}},
			Health{100, 100},
		};
		auto entity3 = TupleEntity{
			Glutes{ 2 },
		};
		auto entity2 = TupleEntity{
			tuple_cat(entity.tuple, entity3.tuple)
		};
		auto physics = entity.get<Physics>();
		physics.velocity = XMFLOAT2{ 32, 32 };
		entity.set(physics);

		const auto x = get_position_x(entity);
		std::cout << x << std::endl;

		{
			auto [physics, health] = entity.get_many<Physics, Health>();
			health.current -= 10;
			entity.set_many(physics, health);
			auto [physics2, health2] = entity.get_many<Physics, Health>();
			std::cout << physics2.position.x << std::endl;
			std::cout << physics2.velocity.x << std::endl;
			std::cout << health2.current << std::endl;
		}
	}
}
