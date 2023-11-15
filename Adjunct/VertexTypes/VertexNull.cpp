//
// VertexNull.cpp
//	Vulkan Vertex-based Add-ons
//
// Instantiate "empty" type and mesh descriptor when no vertex is needed.
//
// Created 6/14/19 by Tadd Jensen
//	© 2112 (uncopyrighted; use at will)
//
#include "MeshObject.h"
#include "VertexNull.h"


VertexTypeNull	VertexNull;


MeshObject ShaderSets3Vertices = {	// Initializing this structure is necessary when
	VertexNull, nullptr,			//	using Shaders that define their own vertices,
	3								//	here, THREE of them, so the Drawing code can
};									//	tell Vulkan how many vertices there are.
