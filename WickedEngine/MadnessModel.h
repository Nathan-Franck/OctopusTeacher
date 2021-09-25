#pragma once
#include <type_traits>

template<class... Responder>
class MadnessModel {
	using Inputs = decltype((TypeSet{ declval<Responder>().input } + ...));
	using Outputs = decltype((TypeSet{ declval<Responder>().output } + ...));
public:
	using State = decltype(TypeSet{ declval<Inputs>() } + TypeSet{ declval<Outputs>() });
	State state;
	MadnessModel(Responder... responders) {};
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

namespace MadnessModelTester {


	void Test() {
		auto model = MadnessModel(
			Responder<tuple<Hum, double>, tuple<Hi>>(
				[](auto input) {
					return tuple(Hi{ 0 });
				}),
			Responder<tuple<int, double>, tuple<Ho, double>>(
				[](auto input) {
					return tuple(Ho{}, 0.0);
				})
			).state;
		cout << "hi" << endl;
	}
}
