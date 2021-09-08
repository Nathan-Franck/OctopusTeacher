#pragma once
#include <LUA/lmathlib.c>

float easeOutBack(float x){
	x = clamp(x, 0.0f, 1.0f);
	const auto c1 = 1.70158;
	const auto c3 = c1 + 1;

	return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
}

float easeOutCirc(float x){
	x = clamp(x, 0.0f, 1.0f);
	return sqrt(1 - pow(x - 1, 2));
}
