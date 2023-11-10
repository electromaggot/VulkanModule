//
// VertexTypes.cpp
//	Vulkan Vertex-based Add-ons
//
// Instantiate Vertex Descriptors.
//
// Created 6/14/19 by Tadd Jensen
//	© 2112 (uncopyrighted; use at will)
//
#include "VulkanPlatform.h"

#include "Vertex2D.h"
#include "Vertex2DTextured.h"
#include "Vertex2DTextureTinted.h"
#include "Vertex2DColored.h"
#include "VertexNull.h"


VertexType2D				VertexDescriptor2D;

VertexType2DTextured		VertexDescriptor2DTextured;

VertexType2DTextureTinted	VertexDescriptor2DTextureTinted;

VertexType2DColored 		VertexDescriptor2DColored;

VertexTypeNull				VertexNull;


MeshObject ShaderSets3Vertices = {	// Initializing this structure is necessary when
	VertexNull, nullptr,			//	using Shaders that define their own vertices,
	3								//	here, THREE of them, so the Drawing code can
};									//	tell Vulkan how many vertices there are.
