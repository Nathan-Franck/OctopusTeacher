#pragma once

#include "ParticleLocomotion.h"

class ParticleComponent
{
public:
	XMVECTOR previousPosition;
	ParticleLocomotion locomotion;

	ParticleComponent(ParticleLocomotion locomotion) : locomotion(locomotion)
	{
		previousPosition = locomotion.position;
	}
};
