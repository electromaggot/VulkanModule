//
// VertexType.h
//	Vulkan Core Class
//
// Abstract Vertex Descriptor, for:
//	- deriving from, to define multivariate vertices
//	- passing around as a Liskov-substitutable
//
// Created 6/14/19 by Tadd Jensen
//	© 2112 (uncopyrighted; use at will)
//
#ifndef VertexType_h
#define VertexType_h

#include "VulkanMath.h"


struct VertexType	// abstraction
{
public:
	virtual size_t	 byteSize() = 0;

	virtual uint32_t nBindingDescriptions()	  = 0;
	virtual uint32_t nAttributeDescriptions() = 0;

	virtual const VkVertexInputBindingDescription*	 pBindingDescriptions()	  = 0;
	virtual const VkVertexInputAttributeDescription* pAttributeDescriptions() = 0;
};



enum MeshIndexType {
	MESH_SMALL_INDEX = 0,
	MESH_LARGE_INDEX = 1
};	// ↑ ≡ ↓
const VkIndexType VkIndexTypes[] {
	VK_INDEX_TYPE_UINT16,	// MESH_SMALL_INDEX
	VK_INDEX_TYPE_UINT32	// MESH_LARGE_INDEX
};	// ↑ ≡ ↓
const int MeshIndexByteSizes[] {
	sizeof(uint16_t),		// MESH_SMALL_INDEX
	sizeof(uint32_t)		// MESH_LARGE_INDEX
};


// IndexBuffer typedef main purpose is for mesh objects pre-defined in data (which
//	typically aren't big, so choose the smaller, hence more memory efficient, type).
// However if the mesh loads procedurally, the index buffer could be any size,
//	possibly huge.  In that case, the appropriate VkIndexType is chosen at runtime.

typedef uint16_t	IndexBufferSmallIndexType;		// 16-bit unsigned ≡ 0 - 65535 indices
					//    ↓   ≡  ↑  ≡           ↓
const MeshIndexType	MeshSmallIndexType = MESH_SMALL_INDEX;

typedef uint32_t	IndexBufferLargeIndexType;		// 32-bit ≡ 0 - 4.3 billion indices
					//    ↓   ≡  ↑  ≡           ↓
const MeshIndexType	MeshLargeIndexType = MESH_LARGE_INDEX;


typedef IndexBufferSmallIndexType	IndexBufferDefaultIndexType;	// Beware! 65535 indices max!
				//   ↑                ≡               ↓
const	MeshIndexType	MeshDefaultIndexType = MESH_SMALL_INDEX;


#endif // VertexType_h
