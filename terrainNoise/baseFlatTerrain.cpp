#include "baseFlatTerrain.h"
#include <noise/noise.h>
#include "terrain_object.h"
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"


baseFlatTerrain::baseFlatTerrain()
{
}

noise::utils::NoiseMap baseFlatTerrain::generateBaseFlatHeightMap()
{
	//Create our Perlin noise module
	noise::module::Billow myModule;
	//Create a planar noise map builder
	utils::NoiseMap heightMap;

	myModule.SetFrequency(2.0);

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
	writer.SetDestFilename("baseflatheightmap.bmp");
	writer.WriteDestFile();

	return heightMap;
}

baseFlatTerrain::~baseFlatTerrain()
{
}
