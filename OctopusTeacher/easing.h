#pragma once
#include <LUA/lmathlib.c>

float easeOutBack(float x)
{
	x = clamp(x, 0.0f, 1.0f);
	const float c1 = 1.70158f;
	const float c3 = c1 + 1;

	return 1.0f + c3 * (float)pow(x - 1, 3) + c1 * (float)pow(x - 1, 2);
}

float easeOutCirc(float x)
{
	x = clamp(x, 0.0f, 1.0f);
	return (float)sqrt(1 - pow(x - 1, 2));
}
