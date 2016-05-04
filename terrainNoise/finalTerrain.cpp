#include "finalTerrain.h"
#include <noise/noise.h>
#include "terrain_object.h"
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"
#include "typeTerrain.h"


finalTerrain::finalTerrain()
{
}


finalTerrain::~finalTerrain()
{
}

noise::utils::NoiseMap finalTerrain::generateFinalHeightMap()
{
	//Create our Perlin noise module
	noise::module::Select finalTerr;
	noise::module::ScaleBias flatTerr;
	noise::module::RidgedMulti mountainTerr;
	noise::module::Perlin typeTerr;

	

	//Create a planar noise map builder
	utils::NoiseMap heightMap;

	// flatten the bumps to a lower height
	flatTerr.SetScale(0.125);
	flatTerr.SetBias(-0.75);

	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(flatTerr);
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
	writer.SetDestFilename("baseflatheightmap.bmp");
	writer.WriteDestFile();

	return heightMap;
}