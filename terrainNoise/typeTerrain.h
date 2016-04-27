#pragma once
#include <noise/noise.h>
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"
class typeTerrain
{
public:
	typeTerrain();
	~typeTerrain();
	noise::utils::NoiseMap generateTypeTerrainMap();
};

