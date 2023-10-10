//
// VertexDescription.h
//	Vulkan Vertex-based Add-ons
//
// Describe to Vulkan the layout of our Vertex buffer, name via
//	VkVertexInputAttributeDescription and VkVertexInputBindingDescription
//	especially by automatically/dynamically generating those data structures.
//
// Created 9/22/23 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef VertexDescription_h
#define VertexDescription_h

#include "vulkan/vulkan.h"	// for VkFormat (see vulkan_core.h)
#include "Universal.h"		// for N_ELEMENTS_IN_ARRAY
#include "VertexAbstract.h"


// Create procedurally.
//
template <typename T>
struct VertexDescription : VertexAbstract
{
	VkVertexInputAttributeDescription	attributeDescriptions[N_ELEMENTS_IN_ARRAY(T::layout)];
	VkVertexInputBindingDescription		bindingDescription;

public:
	size_t	 byteSize()				  { return sizeof(T); }

	uint32_t nBindingDescriptions()	  { return 1; }
	uint32_t nAttributeDescriptions() { return N_ELEMENTS_IN_ARRAY(T::layout); }

	const VkVertexInputAttributeDescription* pAttributeDescriptions() {
		return attributeDescriptions;
	}
	const VkVertexInputBindingDescription* pBindingDescriptions() {
		return &bindingDescription;
	}

	VertexDescription() {
		createAttributeDescriptions();
		createBindingDescription();
	}

	void createAttributeDescriptions() {
		int nElements = N_ELEMENTS_IN_ARRAY(T::layout);
		int byteCount = 0;
		for (int iAttr = 0; iAttr < nElements; ++iAttr) {
			int iLayout = T::layout[iAttr];
			attributeDescriptions[iAttr].location =	iAttr;
			attributeDescriptions[iAttr].binding  =	0;
			attributeDescriptions[iAttr].format =	AttributeFormats[iLayout];
			attributeDescriptions[iAttr].offset =	byteCount;
			byteCount += AttributeByteSizes[iLayout];
		}
		assert(byteCount == sizeof(T));
	}

	void createBindingDescription() {
		bindingDescription.binding =	0;
		bindingDescription.stride =		sizeof(T);
		bindingDescription.inputRate =	VK_VERTEX_INPUT_RATE_VERTEX;
	}
};


// If not done procedurally, creating the VkVertexInput<> data structures
//	manually looks something like this example, and you'll need to do this for
//	each and every VertexAbstract type you may need.  This is just an example.
//
struct VertexType3DNormalTexture : Vertex3DNormalTexture, VertexAbstract
{
	const VkVertexInputAttributeDescription attributeDescriptions[3] = {
		{
			.location	= 0,
			.binding	= 0,
			.format		= VK_FORMAT_R32G32B32_SFLOAT,
			.offset		= offsetof(Vertex3DNormalTexture, position)
		}, {
			.location	= 1,
			.binding	= 0,
			.format		= VK_FORMAT_R32G32B32_SFLOAT,
			.offset		= offsetof(Vertex3DNormalTexture, normal)
		}, {
			.location	= 2,
			.binding	= 0,
			.format		= VK_FORMAT_R32G32_SFLOAT,
			.offset		= offsetof(Vertex3DNormalTexture, texCoord)
		}
	};

	const VkVertexInputBindingDescription bindingDescription = {
		.binding	= 0,
		.stride		= sizeof(Vertex3DNormalTexture),
		.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX
	};

public:
	size_t	 byteSize()				  { return sizeof(Vertex3DNormalTexture); }

	uint32_t nBindingDescriptions()	  { return 1; }
	uint32_t nAttributeDescriptions() { return N_ELEMENTS_IN_ARRAY(attributeDescriptions); }

	const VkVertexInputAttributeDescription* pAttributeDescriptions() {
		return attributeDescriptions;
	}
	const VkVertexInputBindingDescription* pBindingDescriptions() {
		return &bindingDescription;
	}
};


#endif	// VertexDescription_h
