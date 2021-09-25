#pragma once

#include <TypeSet.h>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <list>
#include <typeindex>

using namespace std;

/**
--- Model Listen/Respond System ---
Tool to architect reactive design without a lot of chaos
Able to construct entire reactive flow from single managable/refactorable file
State of program kept in single memory block
Recommended to stay away from references in state members, so external systems can manage history of state
**/
template <typename... T>
class Model
{
private:
	typedef void(*Callback)(Model<T...>&);
	tuple<T...> _state;
	unordered_map<std::type_index, vector<void*>> listeners;
	unordered_set<std::type_index> unhandled_changes;
	std::mutex change_handling_mutex;

	static const int INFINITE_LOOP_LIMIT = 5;

	void change_handler()
	{
		for (int i = 0; unhandled_changes.size() > 0 && i < INFINITE_LOOP_LIMIT; i ++)
		{
			unordered_set<void*> callbacks;
			while (unhandled_changes.size() > 0)
			{
				const auto it = --unhandled_changes.end();
				const auto element = *it;
				const auto listeners = this->listeners[element];

				for_each(listeners.begin(), listeners.end(), [&callbacks](auto listener) { callbacks.insert(listener); });

				unhandled_changes.erase(it);
			}

			for_each(
				callbacks.begin(), callbacks.end(),
				[this](void* callback) {
					Callback callable = (Callback)callback;
					callable(*this);
				});
		}
	}
public:

	Model(T... args) : _state{ std::make_tuple(args...) } {}
	Model(std::tuple<T...> arg) : _state{ arg } {}

	template <TypeSetUtils::InTuple<tuple<T...>>... Member>
	void listen(Callback callback)
	{
		(listeners[typeid(Member)].push_back((void*)callback), ...);
	}

	template <TypeSetUtils::InTuple<tuple<T...>>... Member>
	void submit(tuple<Member...> partialState)
	{
		_state = TypeSet{ _state }.merge(partialState);
		(unhandled_changes.insert(typeid(Member)), ...);
		std::unique_lock<std::mutex> lock(change_handling_mutex, std::try_to_lock);
		if (lock.owns_lock())
		{
			change_handler();
		}
	}
	TypeSet<T...> state() {
		return TypeSet{ _state };
	}
};

namespace ModelTester
{

	int hi = 0;
	struct Chunkin
	{
		float chunkalicious;
	};
	struct Lumpy
	{
		int type = 2;
	};
	tuple initialState(
		Chunkin{ 0 },
		Lumpy{ 1 }
	);
	Model model{ initialState };

	void Test()
	{
		model.state().get<Chunkin>().chunkalicious;
		model.listen<Chunkin>(
			[](decltype(model)& model) {
				const auto [chunkin] = model.state().pick<Chunkin>();

				model.submit(tuple(Lumpy{ (int)chunkin.chunkalicious }));
			});
		model.listen<Lumpy>(
			[](decltype(model)& model) {
				const auto [lumpy] = model.state().pick<Lumpy>();
				cout << lumpy.type << endl;
			});

		model.submit(tuple(Chunkin{ 1.0 }));

		cout << "Done!" << endl;
	}	
}
