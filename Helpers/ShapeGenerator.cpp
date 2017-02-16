#include "ShapeGenerator.h"
#define NUMBER_ARRAY_ELEMENTS(a) sizeof(a) / sizeof(*a)
#define PI 3.14159265359

ShapeData ShapeGenerator::makeCube()
{
	ShapeData ret;

	glm::vec3 vertexBuffer[] =
	{
		glm::vec3(+1.0f, +1.0f, +0.0f),	// 0
		glm::vec3(+1.0f, -1.0f, +0.0f),	// 1
		glm::vec3(-1.0f, -1.0f, +0.0f),	// 2
		glm::vec3(-1.0f, +1.0f, +0.0f), // 3
	};

	glm::vec2 uvBuffer[] =
	{
		glm::vec2(+1.0f, +0.0f), // 0
		glm::vec2(+1.0f, +1.0f), // 1
		glm::vec2(+0.0f, +1.0f), // 2
		glm::vec2(+0.0f, +0.0f), // 3
	};

	ret.numVertices = NUMBER_ARRAY_ELEMENTS(vertexBuffer);
	ret.vertices = new glm::vec3[ret.numVertices];
	memcpy(ret.vertices, vertexBuffer, sizeof(vertexBuffer));

	ret.numUVs = NUMBER_ARRAY_ELEMENTS(uvBuffer);
	ret.uvs = new glm::vec2[ret.numUVs];
	memcpy(ret.uvs, uvBuffer, sizeof(uvBuffer));

	GLushort stackIndices[] = { 0, 1, 2, 3 };
	ret.numIndices = NUMBER_ARRAY_ELEMENTS(stackIndices);
	ret.indices = new GLushort[ret.numIndices];
	memcpy(ret.indices, stackIndices,
		sizeof(stackIndices));

	return ret;
}

ShapeData ShapeGenerator::makePlaneVerts(GLuint dimensions)
{
	ShapeData ret;
	ret.numVertices = dimensions * dimensions;
	int half = dimensions / 2;
	ret.vertices = new glm::vec3[ret.numVertices];
	for (int i = 0; i < dimensions; i++)
	{
		for (int j = 0; j < dimensions; j++)
		{
			glm::vec3& thisVert = ret.vertices[i * dimensions + j];
			thisVert.x = j - half;
			thisVert.z = i - half;
			thisVert.y = 0;
		}
	}
	return ret;
}

ShapeData ShapeGenerator::makePlaneIndices(GLuint dimensions)
{
	ShapeData ret;
	ret.numIndices = (dimensions - 1) * (dimensions - 1) * 2 * 3; // 2 triangles per square, 3 indices per triangle
	ret.indices = new unsigned short[ret.numIndices];
	int runner = 0;
	for (int row = 0; row < dimensions - 1; row++)
	{
		for (int col = 0; col < dimensions - 1; col++)
		{
			ret.indices[runner++] = dimensions * row + col;
			ret.indices[runner++] = dimensions * row + col + dimensions;
			ret.indices[runner++] = dimensions * row + col + dimensions + 1;

			ret.indices[runner++] = dimensions * row + col;
			ret.indices[runner++] = dimensions * row + col + dimensions + 1;
			ret.indices[runner++] = dimensions * row + col + 1;
		}
	}
	assert(runner = ret.numIndices);
	return ret;
}

ShapeData ShapeGenerator::makeSphere(GLuint tesselation)
{
	ShapeData ret = makePlaneVerts(tesselation);
	ShapeData ret2 = makePlaneIndices(tesselation);
	ret.indices = ret2.indices;
	ret.numIndices = ret2.numIndices;

	GLuint dimensions = tesselation;
	const float RADIUS = 1.0f;
	const double CIRCLE = PI * 2;
	const double SLICE_ANGLE = CIRCLE / (dimensions - 1);
	for (size_t col = 0; col < dimensions; col++)
	{
		double phi = -SLICE_ANGLE * col;
		for (size_t row = 0; row < dimensions; row++)
		{
			double theta = -(SLICE_ANGLE / 2.0) * row;
			size_t vertIndex = col * dimensions + row;
			glm::vec3& v = ret.vertices[vertIndex];
			v.x = RADIUS * cos(phi) * sin(theta);
			v.y = RADIUS * sin(phi) * sin(theta);
			v.z = RADIUS * cos(theta);
		}
	}
	return ret;
}