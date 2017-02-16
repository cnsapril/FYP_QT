#pragma once
#include <GL\glew.h>
#include <glm\glm.hpp>

struct ShapeData
{
	ShapeData() :
		vertices(0), numVertices(0),
		indices(0), numIndices(0),
		uvs(0), numUVs(0) {}
	
	glm::vec3* vertices;
	GLuint numVertices;
	GLushort* indices;
	GLuint numIndices;
	glm::vec2* uvs;
	GLuint numUVs;

	GLsizeiptr vertexBufferSize() const
	{
		return numVertices * sizeof(glm::vec3);
	}

	GLsizeiptr indexBufferSize() const
	{
		return numIndices * sizeof(GLushort);
	}

	GLsizeiptr uvBufferSize() const
	{
		return numUVs * sizeof(glm::vec2);
	}

	void cleanup()
	{
		delete[] vertices;
		delete[] indices;
		delete[] uvs;
	}
};