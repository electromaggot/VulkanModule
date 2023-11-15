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
#include <bitset>			// (to build on Linux side)


struct VertexDescriptionDynamic : VertexAbstract
{
	int		numAttributes = 0;
	int		numBytesPerVertex = 0;
	AttributeBits	attribits = 0;
	VertexAttribute*	pLayout = nullptr;

	VkVertexInputBindingDescription		bindingDescription;
	VkVertexInputAttributeDescription*	attributeDescriptions = nullptr;

public:
	size_t	 byteSize()				  override	{ return numBytesPerVertex; }

	uint32_t nBindingDescriptions()	  override	{ return 1; }
	uint32_t nAttributeDescriptions() override	{ return numAttributes; }

	const VkVertexInputAttributeDescription* pAttributeDescriptions() override {
		return attributeDescriptions;
	}
	const VkVertexInputBindingDescription* pBindingDescriptions() override {
		return &bindingDescription;
	}

	VertexDescriptionDynamic() { }

	void initialize(VertexAttribute* pAttrs, int nAttrs, int nBytesVertex) override {	// obsolete
		pLayout = pAttrs;
		numAttributes = nAttrs;
		numBytesPerVertex = nBytesVertex;
		attributeDescriptions = new VkVertexInputAttributeDescription[numAttributes];

		createAttributeDescriptions(pLayout);
		createBindingDescription();
	}
	void initialize(AttributeBits bits) override {
		attribits = bits;
		std::bitset<8 * sizeof(AttributeBits)> onebits(bits);
		numAttributes = (int) onebits.count();
		numBytesPerVertex = numBytesGivenAttributes(bits);
		attributeDescriptions = new VkVertexInputAttributeDescription[numAttributes];

		createAttributeDescriptions(bits);
		createBindingDescription();
	}

	~VertexDescriptionDynamic() {
		if (attributeDescriptions)
			delete attributeDescriptions;
	}


	void createAttributeDescriptions(VertexAttribute* attrEnums) {
		int byteCount = 0;
		for (int iAttr = 0; iAttr < numAttributes; ++iAttr) {
			int iLayout = attrEnums[iAttr];
			attributeDescriptions[iAttr].location =	iAttr;
			attributeDescriptions[iAttr].binding  =	0;
			attributeDescriptions[iAttr].format =	AttributeFormats[iLayout];
			attributeDescriptions[iAttr].offset =	byteCount;
			byteCount += AttributeByteSizes[iLayout];
		}
		assert(byteCount == numBytesPerVertex);
	}
	void createAttributeDescriptions(AttributeBits attrbits) {
		int iAttr = 0, iDesc = 0, byteCount = 0;
		for (AttributeBits bit = 0b0001; bit < PastLastBit; bit <<= 1) {
			if (bit & attrbits) {
				attributeDescriptions[iDesc].location =	iDesc;
				attributeDescriptions[iDesc].binding  =	0;
				attributeDescriptions[iDesc].format =	AttributeFormats[iAttr];
				attributeDescriptions[iDesc].offset =	byteCount;
				byteCount += AttributeByteSizes[iAttr];
				++iDesc;
			}
			++iAttr;
		}
		assert(byteCount == numBytesPerVertex);
	}

	void createBindingDescription() {
		bindingDescription.binding =	0;
		bindingDescription.stride =		numBytesPerVertex;
		bindingDescription.inputRate =	VK_VERTEX_INPUT_RATE_VERTEX;
	}
};


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
struct VertexType2DTextureColor : Vertex2DTextureColor, VertexAbstract
{
	const VkVertexInputAttributeDescription attributeDescriptions[3] = {
		{
			.location	= 0,
			.binding	= 0,
			.format		= VK_FORMAT_R32G32_SFLOAT,
			.offset		= offsetof(Vertex2DTextureColor, position)
		}, {
			.location	= 1,
			.binding	= 0,
			.format		= VK_FORMAT_R32G32_SFLOAT,
			.offset		= offsetof(Vertex2DTextureColor, texCoord)
		}, {
			.location	= 2,
			.binding	= 0,
			.format		= VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset		= offsetof(Vertex2DTextureColor, color)
		}
	};

	const VkVertexInputBindingDescription bindingDescription = {
		.binding	= 0,
		.stride		= sizeof(Vertex2DTextureColor),
		.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX
	};

public:
	size_t	 byteSize()				  { return sizeof(Vertex2DTextureColor); }

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
