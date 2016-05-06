#include "typeTerrain.h"
#include <noise/noise.h>
#include "terrain_object.h"
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"


typeTerrain::typeTerrain()
{
}


typeTerrain::~typeTerrain()
{
}


noise::utils::NoiseMap typeTerrain::generateTypeTerrainMap()
{
	//Create our Perlin noise module
	noise::module::Perlin terrainType;
	//Create a planar noise map builder
	utils::NoiseMap heightMap;
	terrainType.SetFrequency(0.7);
	terrainType.SetPersistence(0.5);
	

	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(terrainType);
	heightMapBuilder.SetDestNoiseMap(heightMap);
	heightMapBuilder.SetDestSize(256, 256);

	//Set boundaries for the heightmap on the z and x axis
	heightMapBuilder.SetBounds(5.0, 9.0, 4.0, 8.0);
	heightMapBuilder.Build();

	utils::RendererImage renderer;
	utils::Image image;
	renderer.SetSourceNoiseMap(heightMap);
	renderer.SetDestImage(image);
	renderer.Render();

	utils::WriterBMP writer;
	writer.SetSourceImage(image);
	writer.SetDestFilename("terraintypeheightmap.bmp");
	writer.WriteDestFile();

	return heightMap;
}