//
// Quad2DColored.cpp
//	Vulkan Convenience 3D Objects
//
// A hard-coded 2D shape for test, demo,
//	example, or any other purpose.
// Uses colored vertices: red, white, blue, green.
// Specifies an Index Buffer, so the Quad
//	may define 4 vertices instead of 6.
//
// Winding is counter-clockwise, which by our default
//	should make the fronts/normals face outward.
// Per Vulkan defaults, negative-Y is up.
//
// Created 6/14/19 by Tadd Jensen
//	© 2112 (uncopyrighted; use at will)
//
#include "MeshObject.h"
#include "Vertex2DColored.h"


const Vertex2DColored QuadVertices[] = {

	{ {	-0.5f, -0.5f },	{ 1.0f, 0.0f, 0.0f } },
	{ {	-0.5f,  0.5f },	{ 1.0f, 1.0f, 1.0f } },
	{ {	 0.5f,  0.5f },	{ 0.0f, 0.0f, 1.0f } },
	{ {	 0.5f, -0.5f },	{ 0.0f, 1.0f, 0.0f } }
};

const IndexBufferDefaultIndexType QuadIndices[] = {

	0, 1, 2, 2, 3, 0
};

MeshObject Quad2DColored = {

	VertexDescriptor2DColored,
	(void*) QuadVertices,
	N_ELEMENTS_IN_ARRAY(QuadVertices),

	(void*) QuadIndices,
	N_ELEMENTS_IN_ARRAY(QuadIndices)
};
