//
// VertexAttribute.h
//	Vulkan Vertex-based Add-ons
//
// Help describe a Vertex's content, using either an array of enums or bits in a word.
//	The former is human-efficient (more readable), the latter machine-efficient (faster smaller).
//
// Created 10/5/23 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef VertexAttribute_h
#define VertexAttribute_h

#include "vulkan/vulkan.h"		// for VkFormat (see vulkan_core.h)
#include "VulkanMath.h"			// for vecN
#include "Universal.h"			// for N_ELEMENTS...


typedef uint16_t  AttributeBits;

enum VertexAttribute {
	POSITION = 0,		// ←- will always be present
	NORMAL	 = 1,
	TEXCOORD = 2,
	COLOR	 = 3
};	// ↑ ≡ ↓
const VkFormat AttributeFormats[] {
	VK_FORMAT_R32G32B32_SFLOAT,		// POSITION
	VK_FORMAT_R32G32B32_SFLOAT,		// NORMAL
	VK_FORMAT_R32G32_SFLOAT,		// TEXCOORD
	VK_FORMAT_R32G32B32A32_SFLOAT	// COLOR
};	// ↑ ≡ ↓
const int AttributeByteSizes[] {
	sizeof(vec3),					// POSITION
	sizeof(vec3),					// NORMAL
	sizeof(vec2),					// TEXCOORD
	sizeof(vec4)					// COLOR
};	// ↑ ≡ ↓
enum AttributeBit {
	POSITION_BIT =	0b00000001,
	NORMAL_BIT =	0b00000010,
	TEXCOORD_BIT =	0b00000100,
	COLOR_BIT =		0b00001000
};	// ↑ ≡ ↓
const AttributeBits Attribits[] {
	POSITION_BIT,
	NORMAL_BIT,
	TEXCOORD_BIT,
	COLOR_BIT
};

const int NumBitsUsed = N_ELEMENTS_IN_ARRAY(Attribits);
const AttributeBits PastLastBit = 1 << NumBitsUsed;


inline int numBytesGivenAttributes(AttributeBits attribits) {
	int sizeVertex = 0;
	for (int iBit = 0; iBit < N_ELEMENTS_IN_ARRAY(Attribits); ++iBit)
		sizeVertex += (Attribits[iBit] & attribits) ? AttributeByteSizes[iBit] : 0;
	return sizeVertex;
}
	/* alternately... uh, non-iterative, but more-hard-coded and less-maintainable
	sizeVertex = (attribits & Attribits[POSITION]) ? AttributeByteSizes[POSITION] : 0
			   + (attribits & Attribits[NORMAL])   ? AttributeByteSizes[NORMAL]	  : 0
			   + (attribits & Attribits[TEXCOORD]) ? AttributeByteSizes[TEXCOORD] : 0
			   + (attribits & Attribits[COLOR])	   ? AttributeByteSizes[COLOR]	  : 0;
}*/


#endif	// VertexAttribute_h
