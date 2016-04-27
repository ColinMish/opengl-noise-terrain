#include "mountainTerrain.h"
#include <noise/noise.h>
#include "terrain_object.h"
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"

mountainTerrain::mountainTerrain()
{

}

noise::utils::NoiseMap mountainTerrain::generateMountainHeightMap()
{
	//Create our Perlin noise module
	noise::module::RidgedMulti myModule;
	//Create a planar noise map builder
	utils::NoiseMap heightMap;


	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(myModule);
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
	writer.SetDestFilename("mountainheightmap.bmp");
	writer.WriteDestFile();

	return heightMap;
}


mountainTerrain::~mountainTerrain()
{
}
