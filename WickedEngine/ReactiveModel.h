#pragma once
#include <type_traits>

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
 
template<class... Responder>
class ReactiveModel {
	using Inputs = decltype((TypeSet{ declval<InspectLambda<Responder>::Input>() } + ...));
	using Outputs = decltype((TypeSet{ declval< InspectLambda<Responder>::Output>() } + ...));
	using State = decltype(TypeSet{ declval<Inputs>() } + TypeSet{ declval<Outputs>() });

	typedef void(*Callback)(State&);
	State _state;
	unordered_map<std::type_index, vector<void*>> listeners;
	unordered_set<std::type_index> unhandled_changes;
	std::mutex change_handling_mutex;

	static const int INFINITE_LOOP_LIMIT = 5;

	void change_handler()
	{
		for (int i = 0; unhandled_changes.size() > 0 && i < INFINITE_LOOP_LIMIT; i++)
		{
			auto existing_unhandled_changes = unordered_set(unhandled_changes);
			unhandled_changes.clear();

			for_each(existing_unhandled_changes.begin(), existing_unhandled_changes.end(), [this](type_index unhandled_change) {
				apply(
					[this, unhandled_change](Responder... responder) {
						return ((TupleHelper<InspectLambda<Responder>::Input>().hasIndex(unhandled_change) && submit(responder(TypeSet{ _state }))), ...);
					}, responders);
				});

			unordered_set<void*> callbacks;
			while (existing_unhandled_changes.size() > 0)
			{
				const auto it = --existing_unhandled_changes.end();
				const auto element = *it;
				const auto listeners = this->listeners[element];

				for_each(listeners.begin(), listeners.end(), [&callbacks](auto listener) { callbacks.insert(listener); });

				existing_unhandled_changes.erase(it);
			}

			for_each(
				callbacks.begin(), callbacks.end(),
				[this](void* callback) {
					Callback callable = (Callback)callback;
					callable(_state);
				});
		}
	}

public:
	using ViewState = State;

	tuple<Responder...> responders;
	ReactiveModel(Responder... responders) : responders{ tuple(responders...) } {};
	template <TypeSetUtils::InTuple<State>... Member>
	void listen(Callback callback)
	{
		(listeners[typeid(Member)].push_back((void*)callback), ...);
	}
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
	State state() {
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
		decltype(model)::ViewState;
		srand(time(NULL));
		model.submit(tuple(LayerA1{ rand() % 1000 / 1000.0f }, LayerB1{ rand() % 1000 / 1000.0f }));

		auto printValue = [](auto... args) {
			cout << (args.value + ...) << endl;
		};
		apply(printValue, model.state());
	}
}
