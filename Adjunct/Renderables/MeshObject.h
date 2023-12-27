//
// MeshObject.h
//	Vulkan Vertex-based Add-on
//
// "Mesh" encapsulates collection of Vertex Buffer and optional Index Buffer.
// Represent a Vertex Array by an abstract Descriptor of vertex type,
//	a pointer to all the vertices, and the number of them.
// To keep the vertex array's initialization as simple as possible, and to
//	not complicate this struct with myriad individual custom constructors:
//   while individual vertices are typed, their structure must be trivial/
//	plain-old-data, and especially, not inherited, like from an abstraction.
//	So unfortunately, the so-called "abstract type" has to be a void pointer.
//
// Created 6/14/19 by Tadd Jensen
//	© 2112 (uncopyrighted; use at will)
//
#ifndef MeshObject_h
#define MeshObject_h

#include "VulkanPlatform.h"

#include "VertexAbstract.h"

class PrimitiveBuffer;


struct MeshObject
{
	VertexAbstract&	vertexType;

	void*			vertices	= nullptr;
	uint32_t		vertexCount	= 0;

	// (fyi, if you're not using indexes, you don't necessarily have
	//	to include anything below here in your pre-initializer code)

	void*			indices		= nullptr;
	uint32_t		indexCount	= 0;

	MeshIndexType	indexType	= MeshDefaultIndexType;

	uint32_t		firstVertex	= 0;		// (and the following can almost always
											//	be left with these default values)
	uint32_t		firstIndex	  = 0;
	int32_t			vertexOffset  = 0;

	uint32_t		instanceCount = 1;		// (while these are shared between
	uint32_t		firstInstance = 0;		//	Vertex Buffer and Index Buffer)


	VkDeviceSize vertexBufferSize() {
		return vertexCount * vertexType.byteSize();
	}

	VkDeviceSize indexBufferSize() {
		return indexCount * MeshIndexByteSizes[indexType];
	}

	bool isUndefined() {
		return vertices == nullptr || vertexCount == 0;
	}
};

#endif // MeshObject_h
