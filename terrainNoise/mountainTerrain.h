#pragma once

#include <noise/noise.h>
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"

class mountainTerrain
{
public:
	mountainTerrain();
	noise::utils::NoiseMap generateMountainHeightMap();
	~mountainTerrain();
};

