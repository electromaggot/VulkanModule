//
// Vertex2DTypes.h
//	Vulkan Vertex-based Add-ons
//
// Encapsulate Vertex with 2D position (always present)
//	plus arbitrary property configurations:
//		texture	  vertex
//	 	 coords	  color
//		 ------   ------
//	 #1		N		N		// single-color
//	 #2		Y		N		// textured
//	 #3		N		Y		// multi-color
//	 #4		Y		Y		// textured multi-color-tinted
//
// Pretty much exclusively uses 32-bit floats (aka signed _SFLOAT).
//
// Created 9/17/23 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Vertex2DTypes_h
#define Vertex2DTypes_h

#include "vulkan/vulkan.h"		// for VkFormat (see vulkan_core.h)
#include <string.h>				// for memcmp
#include "VulkanMath.h"			// for vecN
#include "VertexAttribute.h"


struct _Vertex2D 					// #1
{
	vec2 position;
			   // ↖ these must match, obviously ↘
	static constexpr VertexAttribute layout[] = { POSITION_2D };

	bool operator == (const _Vertex2D& other) const {
		return position == other.position;
	}
};

struct Vertex2DTexture				// #2
{
	vec2 position;
	vec2 texCoord;

	static constexpr VertexAttribute layout[] = {
		POSITION_2D,
		TEXCOORD
	};

	bool operator == (const Vertex2DTexture& other) const {
		return position == other.position && texCoord == other.texCoord;
	}
};

struct Vertex2DColor				// #3
{
	vec2 position;
	vec4 color;

	static constexpr VertexAttribute layout[] = {
		POSITION_2D,
		COLOR
	};

	bool operator == (const Vertex2DColor& other) const {
		return position == other.position && color == other.color;
	}
};

struct Vertex2DTextureColor			// #4
{
	vec2 position;
	vec2 texCoord;
	vec4 color;

	static constexpr VertexAttribute layout[] = {
		POSITION_2D,
		TEXCOORD,
		COLOR
	};

	bool operator == (const Vertex2DTextureColor& other) const {
		return position == other.position && color == other.color && texCoord == other.texCoord;
	}
};


#include "VertexDescription.h"	// the above will all need this,
								//	so include for convenience

#endif // Vertex2DTypes_h
