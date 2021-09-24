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

	//template<typename First, typename... Rest, typename Other>
	//struct subtract_type {
	//	static constexpr
	//};

	//template<typename First, typename... Rest, typename FirstOther, typename... RestOther>
	//struct subtract_types {
	//	static constexpr 
	//};
}

template<typename... T>
class TupleEntity {
private:
public:
	tuple<T...> components;

	explicit TupleEntity(T... args) : components{ std::make_tuple(args...) } {}
	explicit TupleEntity(std::tuple<T...> arg) : components{ arg } {}

	template<typename Component>
	Component get() {
		constexpr std::size_t index =
			EntityHelper::tuple_element_index_v<Component, std::tuple<T...>>;
		return std::get<index>(components);
	}

	template<typename... Component>
	std::tuple<Component...> get_many() {
		return std::make_tuple(get<Component>()...);
	}

	template<typename Component>
	auto set(Component component) {
		constexpr std::size_t index =
			EntityHelper::tuple_element_index_v<Component, std::tuple<T...>>;
		std::get<index>(components) = component;
		return components;
	}

	template<typename... Component>
	auto set_many(Component... component) {
		std::tuple<T...> result = components;
		((result = TupleEntity{ result }.set<Component>(component)), ...);
		return result;
	}

	template<EntityHelper::InTuple<tuple<T...>> Component>
	auto append(Component component) {
		return set(component);
	}

	template<EntityHelper::OutOfTuple<tuple<T...>> Component>
	auto append(Component component) {
		return tuple_cat(components, make_tuple(component));
	}

	template<EntityHelper::OutOfTuple<tuple<T...>>... Component>
	auto append(Component... component) {
		return tuple_cat(components, make_tuple(component...));
	}

	// Sub?

	template<EntityHelper::InTuple<tuple<T...>>... Component>
	tuple<> subtract(tuple<Component...> other) {
		return {};
	};

	//template<EntityHelper::InTuple<tuple<T...>>... Match, typename... After>
	//tuple<After...> subtract(tuple<Match..., After...> other) {
	//	return { };
	//};

	//template<typename... Component>
	//tuple<Component...> subtract(tuple<Component...> component) {
	//	return { component... };
	//};

	//template<typename... Component>
	//auto subtract(tuple<Component...> component) {
	//	return { component };
	//};

	//template<typename... First, typename... Other>
	//auto subtract(tuple<First...> first, tuple<Other...> other) {
	//	return tuple_cat(first, other);
	//};

	template<typename... Component>
	auto merge(Component... component) {
		return std::tuple_cat(components, std::tuple_cat(std::conditional_t<EntityHelper::OutOfTuple<Component, tuple<T...>>,
			std::tuple<Component>,
			std::tuple<>>{}...));
	}
};

namespace ConstexprMadness
{
	template <int T>
	struct num {
		static constexpr int value = T;
	};

	constexpr std::tuple tup = {
		num<1> {},
		num<3> {},
		num<4> {},
		num<5> {}
	};
	constexpr auto result = std::apply([](auto...ts) {
		return std::tuple_cat(std::conditional_t<(decltype(ts)::value > 3),
			std::tuple<decltype(ts)>,
			std::tuple<>>{}...);
		}, tup);
}

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

	template<typename... T>
	auto get_position_x(tuple<T...> entity) {
		const auto thing = TupleEntity{ entity }.get<Physics>();
		return thing.position.x;
	}

	void test() {
		auto entity = std::make_tuple(
			Physics{XMFLOAT2{0, 0}, XMFLOAT2{10, 10}},
			Health{100, 100}
		);
		auto entity3 = std::make_tuple(
			Glutes{ 2 }
		);
		auto entity2 = TupleEntity{
			tuple_cat(entity, entity3)
		};
		auto physics = TupleEntity{ entity }.get<Physics>();
		physics.velocity = XMFLOAT2{ 32, 32 };
		TupleEntity{ entity }.set(physics);

		const auto x = get_position_x(entity);
		std::cout << x << std::endl;

		{
			auto [physics, health] = TupleEntity{ entity }.get_many<Physics, Health>();
			health.current -= 10;
			//entity = TupleEntity{ entity }.set_many(physics, health);
			auto [physics2, health2] = TupleEntity{ entity }.get_many<Physics, Health>();

			auto result = TupleEntity{ entity }.append(health);
			auto result2 = TupleEntity{ entity }.append(Glutes{ 20 });
			//auto result3 = TupleEntity{ entity }.merge(Glutes{ 20 }, health);
			auto result4 = TupleEntity{ make_tuple(1, 1.0f) }.merge("hi", 2, 1.0);
			constexpr bool what = EntityHelper::OutOfTuple<int, tuple<int>>;

			std::cout << physics2.position.x << std::endl;
			std::cout << physics2.velocity.x << std::endl;
			std::cout << health2.current << std::endl;
		}
	}
}
