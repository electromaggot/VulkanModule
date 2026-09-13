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
#include <algorithm>	// for std::max, comparing capacity against contents below

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

	// How much to ALLOCATE, when that must exceed what this mesh currently HOLDS.  Zero (the default)
	//	means "exactly the counts above" - the right answer for static geometry, which never grows.
	// DYNAMIC geometry is the reason these exist.  Its buffers are written in place for the life of the renderable,
	//	so they must be allocated for the largest mesh that will ever be uploaded - which is NOT knowable from the mesh
	//	that happens to exist at creation time.  Leaving it to be inferred from that mesh makes capacity an accident of
	//	whatever data was loaded first: LevelEdit sized terrain buffers from a 101-row window because a short heightmap
	//	was current, then overran them when a longer song was swapped in (Sep 2026).  An owner that knows its own worst
	//	case - rows × columns, maximum glyph count - states it here instead, and PrimitiveBuffer enforces it.
	uint32_t		vertexCapacity = 0;		// In VERTICES, not bytes (as vertexCount).
	uint32_t		indexCapacity  = 0;		// In INDICES, not bytes (as indexCount).


	VkDeviceSize vertexBufferSize() {
		return vertexCount * vertexType.byteSize();
	}

	VkDeviceSize indexBufferSize() {
		return indexCount * MeshIndexByteSizes[indexType];
	}

	// What to allocate: the stated capacity, or the current contents when none was stated.  Never less
	//	than the contents, so a capacity mistakenly set too small cannot truncate the initial upload.
	VkDeviceSize vertexAllocationSize() {
		return std::max(vertexCapacity, vertexCount) * vertexType.byteSize();
	}

	VkDeviceSize indexAllocationSize() {
		return std::max(indexCapacity, indexCount) * MeshIndexByteSizes[indexType];
	}

	bool isUndefined() {
		return vertices == nullptr || vertexCount == 0;
	}
};

#endif // MeshObject_h
