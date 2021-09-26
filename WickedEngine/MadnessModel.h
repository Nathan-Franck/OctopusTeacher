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
class MadnessModel {
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
	MadnessModel(Responder... responders) : responders{ tuple(responders...) } {};
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
};

struct LayerA1 {
	int value;
};

struct LayerB1 {
	int value;
};

struct LayerA2 {
	int value;
};

struct LayerB2 {
	int value;
};

struct LayerA3 {
	int value;
};

namespace MadnessModelTester {


	void Test() {
		auto model = MadnessModel(
			[](tuple<LayerA1> input) {
				return tuple(LayerA2{ 12 });
			},
			[](tuple<LayerA2> input) {
				return tuple(LayerA3{ 13 });
			},
			[](tuple<LayerB1> input) {
				return tuple(LayerB2{ 22 });
			}
		);
		decltype(model)::ViewState;
		model.submit(tuple(LayerA1{ 11 }, LayerB1{ 21 }));
		cout << "hi" << endl;
	}
}
