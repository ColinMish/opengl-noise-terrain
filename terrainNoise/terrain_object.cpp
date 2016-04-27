/* terrain_object.cpp
Example class to show how to render a height map
Iain Martin November 2014
*/

#include <noise/noise.h>
#include "terrain_object.h"
#include <glm/gtc/noise.hpp>
#include "noiseutils.h"
#include "SOIL.h"
#include "mountainTerrain.h"
#include "baseFlatTerrain.h"
#include "flatTerrain.h"

// Size of the procedurally generated texture
const int TEXTURE_SIZE = 256;
GLuint texture[1];

// Creates the color gradients for the texture.
void CreateTextureColor(utils::RendererImage& renderer);

// Given a noise module, this function renders a flat square texture map and
// writes it to a Windows bitmap (*.bmp) file.  Because the texture map is
// square, its width is equal to its height.  The texture map can be seamless
// (tileable) or non-seamless. 
void CreatePlanarTexture(const noise::module::Module& noiseModule, bool seamless,
	int height, const char* filename);

// Given a noise module, this function renders a spherical texture map and
// writes it to a Windows bitmap (*.bmp) file.  The texture map's width is
// double its height.
void CreateSphericalTexture(const noise::module::Module& noiseModule, int height,
	const char* filename);

// Given a noise map, this function renders a texture map and writes it to a
// Windows bitmap (*.bmp) file.
void RenderTexture(const noise::utils::NoiseMap& noiseMap,
	const char* filename);


/* Define the vertex attributes for vertex positions and normals. 
   Make these match your application and vertex shader
   You might also want to add colours and texture coordinates */
terrain_object::terrain_object(int octaves, GLfloat freq, GLfloat scale)
{
	attribute_v_coord = 0;
	attribute_v_normal = 2;
	xsize = 0;	// Set to zero because we haven't created the heightfield array yet
	zsize = 0;	
	perlin_octaves = octaves;
	perlin_freq = freq;
	perlin_scale = scale;
	height_scale = 1.f;
	
}


terrain_object::~terrain_object()
{
	/* tidy up */
	if (vertices) delete[] vertices;
	if (normals) delete[] normals;
}

// Generates a texture using coherent noise
void terrain_object::generateTexture()
{
	noise::module::Billow groundTexture;
	groundTexture.SetSeed(0);
	groundTexture.SetFrequency(6.0);
	groundTexture.SetPersistence(0.625);
	groundTexture.SetLacunarity(2.18359375);
	groundTexture.SetOctaveCount(6);
	groundTexture.SetNoiseQuality(QUALITY_STD);

	// Use Voronoi polygons to produce the small grains for the granite texture.
	module::Voronoi baseGrains;
	baseGrains.SetSeed(1);
	baseGrains.SetFrequency(16.0);
	baseGrains.EnableDistance(true);

	// Scale the small grain values so that they may be added to the base
	// granite texture.  Voronoi polygons normally generate pits, so apply a
	// negative scaling factor to produce bumps instead.
	module::ScaleBias scaledGrains;
	scaledGrains.SetSourceModule(0, baseGrains);
	scaledGrains.SetScale(-0.5);
	scaledGrains.SetBias(0.0);

	// Combine the primary granite texture with the small grain texture.
	module::Add combinedGranite;
	combinedGranite.SetSourceModule(0, groundTexture);
	combinedGranite.SetSourceModule(1, scaledGrains);

	// Finally, perturb the granite texture to add realism.
	module::Turbulence finalGranite;
	finalGranite.SetSourceModule(0, combinedGranite);
	finalGranite.SetSeed(2);
	finalGranite.SetFrequency(4.0);
	finalGranite.SetPower(1.0 / 8.0);
	finalGranite.SetRoughness(6);

	// Given the granite noise module, create a non-seamless texture map, a
	// seamless texture map, and a spherical texture map.
	CreatePlanarTexture(finalGranite, false, TEXTURE_SIZE,
		"textureplane.bmp");
	CreatePlanarTexture(finalGranite, true, TEXTURE_SIZE,
		"textureseamless.bmp");
	CreateSphericalTexture(finalGranite, TEXTURE_SIZE	,
		"texturesphere.bmp");
}

GLint loadGLTexture( char *filename)
{
	GLuint SOIL_response;
	SOIL_response = SOIL_load_OGL_texture(filename, 0, 0, SOIL_FLAG_POWER_OF_TWO);
	if (!SOIL_response)
	{
		printf("Texture could not be loaded");
		exit(1);
	}
	return (SOIL_response);
}

void CreateTextureColor(utils::RendererImage& renderer)
{
	// Create a gray granite palette.  Black and pink appear at either ends of
	// the palette; those colors provide the charactistic black and pink flecks
	// in granite.
	renderer.ClearGradient();
	renderer.AddGradientPoint(-1.0000, utils::Color(0, 0, 0, 255));
	renderer.AddGradientPoint(-0.9375, utils::Color(0, 0, 0, 255));
	renderer.AddGradientPoint(-0.8750, utils::Color(216, 216, 242, 255));
	renderer.AddGradientPoint(0.0000, utils::Color(191, 191, 191, 255));
	renderer.AddGradientPoint(0.5000, utils::Color(210, 116, 125, 255));
	renderer.AddGradientPoint(0.7500, utils::Color(210, 113, 98, 255));
	renderer.AddGradientPoint(1.0000, utils::Color(255, 176, 192, 255));
}

void CreatePlanarTexture(const module::Module& noiseModule, bool seamless,
	int height, const char* filename)
{
	// Map the output values from the noise module onto a plane.  This will
	// create a two-dimensional noise map which can be rendered as a flat
	// texture map.
	utils::NoiseMapBuilderPlane plane;
	utils::NoiseMap noiseMap;
	plane.SetBounds(-1.0, 1.0, -1.0, 1.0);
	plane.SetDestSize(height, height);
	plane.SetSourceModule(noiseModule);
	plane.SetDestNoiseMap(noiseMap);
	plane.EnableSeamless(seamless);
	plane.Build();

	RenderTexture(noiseMap, filename);
}

void CreateSphericalTexture(const module::Module& noiseModule, int height,
	const char* filename)
{
	// Map the output values from the noise module onto a sphere.  This will
	// create a two-dimensional noise map which can be rendered as a spherical
	// texture map.
	utils::NoiseMapBuilderSphere sphere;
	utils::NoiseMap noiseMap;
	sphere.SetBounds(-90.0, 90.0, -180.0, 180.0); // degrees
	sphere.SetDestSize(height * 2, height);
	sphere.SetSourceModule(noiseModule);
	sphere.SetDestNoiseMap(noiseMap);
	sphere.Build();

	RenderTexture(noiseMap, filename);
}

void RenderTexture(const utils::NoiseMap& noiseMap, const char* filename)
{
	// Create the color gradients for the texture.
	utils::RendererImage textureRenderer;
	CreateTextureColor(textureRenderer);

	// Set up us the texture renderer and pass the noise map to it.
	utils::Image destTexture;
	textureRenderer.SetSourceNoiseMap(noiseMap);
	textureRenderer.SetDestImage(destTexture);
	textureRenderer.EnableLight(true);
	textureRenderer.SetLightAzimuth(135.0);
	textureRenderer.SetLightElev(60.0);
	textureRenderer.SetLightContrast(2.0);
	textureRenderer.SetLightColor(utils::Color(255, 255, 255, 0));

	// Render the texture.
	textureRenderer.Render();

	// Write the texture as a Windows bitmap file (*.bmp).
	utils::WriterBMP textureWriter;
	textureWriter.SetSourceImage(destTexture);
	textureWriter.SetDestFilename(filename);
	textureWriter.WriteDestFile();
}

/* Copy the vertices, normals and element indices into vertex buffers */
void terrain_object::createObject()
{
	generateTexture();
	/* Generate the vertex buffer object */
	glGenBuffers(1, &vbo_mesh_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh_vertices);
	glBufferData(GL_ARRAY_BUFFER, xsize * zsize  * sizeof(glm::vec3), &(vertices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the normals in a buffer object */
	glGenBuffers(1, &vbo_mesh_normals);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh_normals);
	glBufferData(GL_ARRAY_BUFFER, xsize * zsize * sizeof(glm::vec3), &(normals[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Generate a buffer for the indices
	glGenBuffers(1, &ibo_mesh_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_mesh_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size()* sizeof(GLuint), &(elements[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

/* Enable vertex attributes and draw object
Could improve efficiency by moving the vertex attribute pointer functions to the
create object but this method is more general 
This code is almost untouched fomr the tutorial code except that I changed the
number of elements per vertex from 4 to 3*/
void terrain_object::drawObject(int drawmode)
{
	int size;	// Used to get the byte size of the element (vertex index) array

	// Describe our vertices array to OpenGL (it can't guess its format automatically)
	glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh_vertices);
	glVertexAttribPointer(
		attribute_v_coord,  // attribute index
		3,                  // number of elements per vertex, here (x,y,z)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh_normals);
	glVertexAttribPointer(
		attribute_v_normal, // attribute
		3,                  // number of elements per vertex, here (x,y,z)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
		);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_mesh_elements); 
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

	// Enable this line to show model in wireframe
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/* Draw the triangle strips */
	for (int i = 0; i < xsize - 1; i++)
	{
		GLuint location = sizeof(GLuint) * (i * zsize * 2);
		glDrawElements(GL_TRIANGLE_STRIP, zsize * 2, GL_UNSIGNED_INT, (GLvoid*)(location));
	}
}

/* Define the terrian heights */
/* Uses code adapted from OpenGL Shading Language Cookbook: Chapter 8 */
/*
This function is as it is in the lab start, it is being commented out and a similar function is used instead to generate
a heightmap from coherent noise
*/
void terrain_object::calculateNoise()
{

	//TODO: clean up unused terrains
	mountainTerrain mountainTerr;
	baseFlatTerrain baseFlatTerr;
	flatTerrain flatTerr;

	/* Create the array to store the noise values */
	/* The size is the number of vertices * number of octaves */
	noise = new GLfloat[xsize * zsize * perlin_octaves];
	for (int i = 0; i < (xsize*zsize*perlin_octaves); i++) noise[i] = 0;

	GLfloat xfactor = 1.f / (xsize - 1);
	GLfloat zfactor = 1.f / (zsize - 1);
	GLfloat freq = perlin_freq;
	GLfloat scale = perlin_scale;

	//double MaxValueFor;
	//double currentValue;
	//added now
	module::Perlin myModule;
	myModule.SetOctaveCount(6);
	myModule.SetFrequency(1.0);
	myModule.SetPersistence(0.5);
	double value;
	utils::NoiseMap heightMap = flatTerr.generateFlatHeightMap();

	for (int row = 0; row <zsize; row++)
	{
		for (int col = 0; col < xsize; col++)
		{
			GLfloat x = xfactor * col;
			GLfloat z = zfactor * row;
			GLfloat sum = 0;
			GLfloat curent_scale = scale;
			GLfloat current_freq = freq;

			//this
			//for (int oct = 0; oct < 4; oct++)
			//{
			value = heightMap.GetValue(row, col);

			//value = myModule.GetValue(row / scaleForTerrain, col / scaleForTerrain, 0);

			noise[(row * xsize + col) * perlin_octaves] = value*scale;



			//	if (currentValue < value*scaleForTerrain){
			//		MaxValueFor = currentValue;
			//		std::cout << MaxValueFor;
			//	}


		}

	}


}


/* Define the vertex array that specifies the terrain
   (x, y) specifies the pixel dimensions of the heightfield (x * y) vertices
   (xs, ys) specifies the size of the heightfield region in world coords
   */
void terrain_object::createTerrain(GLuint xp, GLuint zp, GLfloat xs, GLfloat zs)
{
	xsize = xp;
	zsize = zp;
	width = xs;
	height = zs;

	/* Scale heights in relation to the terrain size */
	height_scale = xs;

	/* Create array of vertices */
	GLuint numvertices = xsize * zsize;
	vertices = new glm::vec3[numvertices];
	normals  = new glm::vec3[numvertices];

	/* First calculate the noise array which we'll use for our vertex height values */
	calculateNoise();

	/* Define starting (x,z) positions and the step changes */
	GLfloat xpos = -width / 2.f;
	GLfloat xpos_step = width / GLfloat(xp);
	GLfloat zpos_step = height / GLfloat(zp);
	GLfloat zpos_start = -height / 2.f;

	/* Define the vertex positions and the initial normals for a flat surface */
	for (GLuint x = 0; x < xsize; x++)
	{
		GLfloat zpos = zpos_start;
		for (GLuint z = 0; z < zsize; z++)
		{
			GLfloat height = noise[(x*zsize + z) * perlin_octaves + perlin_octaves-1];
			vertices[x*xsize + z] = glm::vec3(xpos, (height-0.5f)*height_scale, zpos);
			normals[x*xsize + z]  = glm::vec3(0, 1.0f, 0);		// Normals for a flat surface
			zpos += zpos_step;
		}
		xpos += xpos_step;
	}

	/* Define vertices for triangle strips */
	for (GLuint x = 0; x < xsize - 1; x++)
	{
		GLuint top    = x * zsize;
		GLuint bottom = top + zsize;
		for (GLuint z = 0; z < zsize; z++)
		{
			elements.push_back(top++);
			elements.push_back(bottom++);
		}
	}

	// Stretch the height values to a defined height range 
	stretchToRange(-(xs / 8.f), (xs / 8.f));

	// Define a sea level by flattening low regions
	defineSea(0);

	// Calculate the normals by averaging cross products for all triangles 
	calculateNormals();
}

/* Calculate normals by using cross products along the triangle strips
   and averaging the normals for each vertex */
void terrain_object::calculateNormals()
{
	GLuint element_pos = 0;
	glm::vec3 AB, AC, cross_product;

	// Loop through each triangle strip  
	for (GLuint x = 0; x < xsize - 1; x++)
	{
		// Loop along the strip
		for (GLuint tri = 0; tri < zsize * 2 - 2; tri++)
		{
			// Extract the vertex indices from the element array 
			GLuint v1 = elements[element_pos];
			GLuint v2 = elements[element_pos+1];
			GLuint v3 = elements[element_pos+2];
			
			// Define the two vectors for the triangle
			AB = vertices[v2] - vertices[v1];
			AC = vertices[v3] - vertices[v1];

			// Calculate the cross product
			cross_product = glm::normalize(glm::cross(AC, AB));

			// Add this normal to the vertex normal for all three vertices in the triangle
			normals[v1] += cross_product;
			normals[v2] += cross_product;
			normals[v3] += cross_product;

			// Move on to the next vertex along the strip
			element_pos++;
		}

		// Jump past the lat two element positions to reach the start of the strip
		element_pos += 2;	
	}

	// Normalise the normals
	for (GLuint v = 0; v < xsize * zsize; v++)
	{
		normals[v] = glm::normalize(normals[v]);
	}
}

/* Stretch the height values to the range min to max */
void terrain_object::stretchToRange(GLfloat min, GLfloat max)
{
	/* Calculate min and max values */
	GLfloat cmin, cmax;
	cmin = cmax = vertices[0].y;
	for (int v = 1; v < xsize*zsize; v++)
	{
		if (vertices[v].y < cmin) cmin = vertices[v].y;
		if (vertices[v].y > cmax) cmax = vertices[v].y;
	}

	// Calculate stretch factor
	GLfloat stretch_factor = (max - min) / (cmax - cmin);
	GLfloat stretch_diff = cmin - min;

	/* Rescale the vertices */
	for (int v = 0; v < xsize*zsize; v++)
	{
		vertices[v].y = (vertices[v].y - stretch_diff) * stretch_factor;
	}
}

/* Define a sea level in the terrain */
void terrain_object::defineSea(GLfloat sealevel)
{
	for (int v = 0; v < xsize*zsize; v++)
	{
		if (vertices[v].y < sealevel)
		{
			vertices[v].y = sealevel;
		}
	}
}

