#pragma once
#include <noise/noise.h>
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"
#include "typeTerrain.h"
#include "mountainTerrain.h"
#include "flatTerrain.h"
class finalTerrain
{
public:
	finalTerrain();
	~finalTerrain();
	noise::utils::NoiseMap generateFinalHeightMap();
};

