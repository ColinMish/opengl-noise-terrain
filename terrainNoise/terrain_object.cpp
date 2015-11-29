/* terrain_object.cpp
Example class to show how to render a height map
Iain Martin November 2014
*/

#include "terrain_object.h"
#include <glm/gtc/noise.hpp>

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


/* Copy the vertices, normals and element indices into vertex buffers */
void terrain_object::createObject()
{
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
void terrain_object::calculateNoise()
{
	/* Create the array to store the noise values */
	/* The size is the number of vertices * number of octaves */
	noise = new GLfloat[xsize * zsize * perlin_octaves];
	for (int i = 0; i < (xsize*zsize*perlin_octaves); i++) noise[i] = 0;

	GLfloat xfactor = 1.f / (xsize - 1);
	GLfloat zfactor = 1.f / (zsize - 1);
	GLfloat freq = perlin_freq;
	GLfloat scale = perlin_scale;

	for (int row = 0; row < zsize; row++)
	{
		for (int col = 0; col < xsize; col++)
		{
			GLfloat x = xfactor * col;
			GLfloat z = zfactor * row;
			GLfloat sum = 0;
			GLfloat curent_scale = scale;
			GLfloat current_freq = freq;

			// Compute the sum for each octave
			for (int oct = 0; oct < perlin_octaves; oct++)
			{
				glm::vec2 p(x*current_freq, z*current_freq);
				GLfloat val = glm::perlin(p) / curent_scale;
				sum += val;
				GLfloat result = (sum + 1.f) / 2.f;

				// Store the noise value in our noise array
				noise[(row * xsize + col) * perlin_octaves + oct] = result;
				
				// Move to the next frequency and scale
				current_freq *= 2.f;
				curent_scale *= scale;
			}
			
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

