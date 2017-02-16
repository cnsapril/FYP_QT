#pragma once
#include <glm\glm.hpp>
#include "ShapeData.h"

class ShapeGenerator
{
public:
	static ShapeData makeCube();
	static ShapeData makeSphere(GLuint tesselation = 20);

protected:
	static ShapeData makePlaneVerts(GLuint dimensions);
	static ShapeData makePlaneIndices(GLuint dimensions);
};