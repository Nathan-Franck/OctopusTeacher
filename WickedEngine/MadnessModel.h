#pragma once
#include <type_traits>

template<class... Responder>
class MadnessModel {
public:
	MadnessModel(Responder... responder) {}

	using Inputs = decltype((TypeSet{ declval<Responder>().input } + ...));
	using Outputs = decltype((TypeSet{ declval<Responder>().output } + ...));
	using State = decltype(TypeSet{ declval<Inputs>() } + TypeSet{ declval<Outputs>() });
};

template<class Input, class Output>
class Responder {
public:
	Responder(Output(*lambder)(Input)) {}
	Input input;
	Output output;
};

struct Hi {
	int what;
};

struct Ho {

};

struct Hum {

};

using Thing = decltype(MadnessModel(
	Responder<tuple<Hum, double>, tuple<Hi>>(
		[](auto input) {
			return tuple(Hi{ 0 });
		}),
	Responder<tuple<int, double>, tuple<Ho, double>>(
		[](auto input) {
			return tuple(Ho{}, 0.0);
		})
			))::State;

namespace MadnessModelTester {


	void Test() {
		Thing thing = tuple(0.0f, 0, 0.0);
		cout << "hi" << endl;
	}
}
