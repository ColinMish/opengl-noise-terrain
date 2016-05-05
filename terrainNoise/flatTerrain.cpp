#include "baseFlatTerrain.h"
#include "flatTerrain.h"
#include <noise/noise.h>
#include "terrain_object.h"
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"


flatTerrain::flatTerrain()
{
}


flatTerrain::~flatTerrain()
{
}

noise::utils::NoiseMap flatTerrain::generateFlatHeightMap()
{
	//Create our Perlin noise module
	noise::module::ScaleBias flatTerr;
	baseFlatTerrain baseFlatTerrGenerator;
	noise::module::Billow baseFlatTerr;

	//Create a planar noise map builder
	utils::NoiseMap heightMap;

	baseFlatTerr.SetFrequency(0.5);
	flatTerr.SetSourceModule(0, baseFlatTerr);
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
	writer.SetDestFilename("flatheightmap.bmp");
	writer.WriteDestFile();

	return heightMap;
}