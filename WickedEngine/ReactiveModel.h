#pragma once
#include <type_traits>

template<class... Reactors>
class ReactiveModel
{
	// Helper structs
	template <typename T> struct InspectLambda : InspectLambda<decltype(&T::operator())> {};
	template <typename Out, typename C, typename In>
	struct InspectLambda<Out(C::*)(In) const>
	{
		using Input = In;
		using Output = Out;
	};
	template <typename T> struct TupleHelper : InspectLambda<T> {};
	template <typename... Member>
	struct TupleHelper <tuple<Member...>>
	{
		bool hasIndex(type_index index)
		{
			return ((typeid(Member) == index) || ...);
		}
	};

	// Derivative types
	using _State = decltype((TypeSet{
		TypeSet{
			declval<InspectLambda<Reactors>::Input>()
		}.merge(
			declval<InspectLambda<Reactors>::Output>()
		) } + ...));
	typedef void(*Callback)(const _State&);

	// State
	_State _state;
	tuple<Reactors...> reactors;
	unordered_map<std::type_index, vector<void*>> listeners;
	unordered_set<std::type_index> unhandled_changes;
	std::mutex change_handling_mutex;

	// Reactiveness implentation
	void change_handler()
	{
		static const int INFINITE_LOOP_LIMIT = 5;
		for (int i = 0; unhandled_changes.size() > 0 && i < INFINITE_LOOP_LIMIT; i++)
		{
			auto changed_indices = unordered_set(unhandled_changes);
			unhandled_changes.clear();

			// Reactors
			for_each(
				changed_indices.begin(), changed_indices.end(),
				[this](type_index changed_index) {
					apply(
						[this, changed_index](Reactors... reactor) {
							return ((
								TupleHelper<InspectLambda<Reactors>::Input>().hasIndex(changed_index)
								&& submit(reactor(TypeSet{ _state }))
								), ...);
						}, reactors);
				});

			// Listeners
			for_each(
				changed_indices.begin(), changed_indices.end(),
				[this](type_index changed_index) {
					const auto listeners = this->listeners[changed_index];
					for_each(
						listeners.begin(), listeners.end(),
						[this](auto listener) {
							Callback callback = (Callback)listener;
							callback(_state);
						});
				});
		}
	}

public:
	using State = _State;

	// Provide a series of reactive callbacks, one reactive function may trigger another reactive function, while aggregating new state data
	ReactiveModel(Reactors... reactors) : reactors{ tuple(reactors...) } {};

	// Add new side-effect callback that listens to type(s) in model
	template <TypeSetUtils::InTuple<State>... Member>
	void listen(Callback callback)
	{
		(listeners[typeid(Member)].push_back((void*)callback), ...);
	}

	// Add data to model state, triggers reactive events internally
	template <TypeSetUtils::InTuple<State>... Member>
	bool submit(tuple<Member...> partialState)
	{
		_state = TypeSet{ _state }.merge(partialState);
		(unhandled_changes.insert(typeid(Member)), ...);
		std::unique_lock<std::mutex> lock(change_handling_mutex, std::try_to_lock);
		if (lock.owns_lock())
		{
			change_handler();
		}
		return true;
	}

	State state()
	{
		return _state;
	}
};

struct LayerA1 {
	float value;
};

struct LayerB1 {
	float value;
};

struct LayerA2 {
	float value;
};

struct LayerB2 {
	float value;
};

struct LayerA3 {
	float value;
};

namespace ReactiveModelTester {


	void Test() {
		auto model = ReactiveModel(
			[](tuple<LayerA1> input) {
				auto [layerA1] = input;
				return tuple(LayerA2{ layerA1.value / 1234.0f });
			},
			[](tuple<LayerA2> input) {
				auto [layerA2] = input;
				return tuple(LayerA3{ layerA2.value / 2345.0f });
			},
			[](tuple<LayerB1> input) {
				auto [layerB1] = input;
				return tuple(LayerB2{ layerB1.value / 3456.0f });
			}
		);
		decltype(model)::State;
		srand(time(NULL));
		model.submit(tuple(LayerA1{ rand() % 1000 / 1000.0f }, LayerB1{ rand() % 1000 / 1000.0f }));

		auto printValue = [](auto... args) {
			cout << (args.value + ...) << endl;
		};
		apply(printValue, model.state());
	}
}
