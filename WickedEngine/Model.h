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
	tuple<T...> state;
	unordered_map<std::type_index, vector<void*>> listeners;
	unordered_set<std::type_index> unhandled_changes;
	std::mutex change_handling_mutex;

	void change_handler()
	{
		while (unhandled_changes.size() > 0)
		{
			const auto it = --unhandled_changes.end();
			const auto element = *it;



			unhandled_changes.erase(it);
		}
	}
public:

	Model(T... args) : state{ std::make_tuple(args...) } {}
	Model(std::tuple<T...> arg) : state{ arg } {}

	template <TypeSetUtils::InTuple<tuple<T...>>... Member>
	void listen(void (*callback)(tuple<T...>&))
	{
		(listeners[typeid(Member)].push_back((void*)callback), ...);
	}
	const tuple<T...>& State = state;
	template <TypeSetUtils::InTuple<tuple<T...>>... Member>
	void merge(tuple<Member...> partialState)
	{
		state = TypeSet{ state }.merge(partialState);
		(unhandled_changes.insert(typeid(Member)), ...);
		std::unique_lock<std::mutex> lock(change_handling_mutex, std::try_to_lock);
		if (lock.owns_lock())
		{
			change_handler();
		}
	}
};

namespace ModelTester
{
	void Test()
	{
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

		TypeSet{ model.State }.get<Chunkin>().chunkalicious;
		model.merge(tuple(Chunkin{ 1.0 }));
		const auto state = model.State;
		model.listen<Chunkin, Lumpy>(
			[](decltype(initialState)& components) {
				const auto what = TypeSet{ components }.get<Chunkin>().chunkalicious;
				what;
			});

		cout << "Done!" << endl;
	}	
}
