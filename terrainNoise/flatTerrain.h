#pragma once
#include <noise/noise.h>
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"
class flatTerrain
{
public:
	flatTerrain();
	~flatTerrain();
	noise::utils::NoiseMap generateFlatHeightMap();
};

