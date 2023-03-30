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


typedef uint16_t	IndexBufferIndexType;
	//  ^^^^^^^^-----(these two must jive!)----vvvvvvvvvvvvvvvvvvvv
const VkIndexType	IndexBufferIndexTypeEnum = VK_INDEX_TYPE_UINT16;


struct VertexType	// abstraction
{
public:
	virtual size_t	 byteSize() = 0;

	virtual uint32_t nBindingDescriptions()	  = 0;
	virtual uint32_t nAttributeDescriptions() = 0;

	virtual const VkVertexInputBindingDescription*	 pBindingDescriptions()	  = 0;
	virtual const VkVertexInputAttributeDescription* pAttributeDescriptions() = 0;
};

#endif // VertexType_h
