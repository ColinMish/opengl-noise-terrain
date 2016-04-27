#pragma once

#include <noise/noise.h>
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"

class baseFlatTerrain
{
public:
	baseFlatTerrain();
	noise::utils::NoiseMap generateBaseFlatHeightMap();
	~baseFlatTerrain();
};

