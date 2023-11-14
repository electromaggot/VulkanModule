//
// Vertex3DTypes.h
//	Vulkan Vertex-based Add-ons
//
// Encapsulate Vertex with 3D position (always present)
//	plus arbitrary property configurations:
//	 	normal  texture   vertex
//	 	vector   coords   color
//		------   ------   ------
//	 #1		N		N		N		// geometry only (wireframe?)
//	 #2		Y		N		N		// shaded solid single color
//	 #3		N		Y		N		// textured unshaded
//	 #4		N		N		Y		// solid multi-color unshaded
//	 #5		Y		Y		N		// shaded textured
//	 #6		N		Y		Y		// textured multi-color-tinted
//	 #7		Y		N		Y		// shaded multi-color
//	 #8		Y		Y		Y		// shaded textured multi-tinted
//
// Pretty much exclusively uses 32-bit floats (aka signed _SFLOAT).
//
// Created 9/17/23 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Vertex3DTypes_h
#define Vertex3DTypes_h

#include "vulkan/vulkan.h"		// for VkFormat (see vulkan_core.h)
#include <string.h>				// for memcmp
#include "VulkanMath.h"			// for vecN
#include "VertexAttribute.h"


struct Vertex3D 					// #1
{
	vec3 position;
			   // ↖ these must match, obviously ↘
	static constexpr VertexAttribute layout[] = { POSITION };
	//uint16_t attriBits = POSITION_BIT;

	bool operator == (const Vertex3D& other) const {
		return position == other.position;
	}
};

struct Vertex3DNormal				// #2
{
	vec3 position;
	vec3 normal;

	static constexpr VertexAttribute layout[] = {
		POSITION,
		NORMAL
	};
	//uint16_t attriBits = POSITION_BIT | NORMAL_BIT;

	bool operator == (const Vertex3DNormal& other) const {
		return position == other.position && normal == other.normal;
	}
};

struct Vertex3DTexture				// #3
{
	vec3 position;
	vec2 texCoord;

	static constexpr VertexAttribute layout[] = {
		POSITION,
		TEXCOORD
	};
	//uint16_t attriBits = POSITION_BIT | TEXCOORD_BIT;

	bool operator == (const Vertex3DTexture& other) const {
		return position == other.position && texCoord == other.texCoord;
	}
};

struct Vertex3DColor				// #4
{
	vec3 position;
	vec4 color;

	static constexpr VertexAttribute layout[] = {
		POSITION,
		COLOR
	};
	//uint16_t attriBits = POSITION_BIT | COLOR_BIT;

	bool operator == (const Vertex3DColor& other) const {
		return position == other.position && color == other.color;
	}
};

struct Vertex3DNormalTexture		// #5
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;

	static constexpr VertexAttribute layout[] = {
		POSITION,
		NORMAL,
		TEXCOORD
	};
	//uint16_t attriBits = POSITION_BIT | NORMAL_BIT | TEXCOORD_BIT;

	bool operator == (const Vertex3DNormalTexture& other) const {
		return memcmp(this, &other, sizeof(*this)) == 0;
	}
};

struct Vertex3DTextureColor			// #6
{
	vec3 position;
	vec2 texCoord;
	vec4 color;

	static constexpr VertexAttribute layout[] = {
		POSITION,
		TEXCOORD,
		COLOR
	};
	//uint16_t attriBits = POSITION_BIT | TEXCOORD_BIT | COLOR_BIT;

	bool operator == (const Vertex3DTextureColor& other) const {
		return position == other.position && color == other.color && texCoord == other.texCoord;
	}
};

struct Vertex3DNormalColor			// #7
{
	vec3 position;
	vec3 normal;
	vec4 color;

	static constexpr VertexAttribute layout[] = {
		POSITION,
		NORMAL,
		COLOR
	};
	//uint16_t attriBits = POSITION_BIT | NORMAL_BIT | COLOR_BIT;

	bool operator == (const Vertex3DNormalColor& other) const {
		return memcmp(this, &other, sizeof(*this)) == 0;
	}
};

struct Vertex3DNormalTextureColor	// #8
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
	vec4 color;

	static constexpr VertexAttribute layout[] = {
		POSITION,
		NORMAL,
		TEXCOORD,
		COLOR
	};
	//uint16_t attriBits = POSITION_BIT | NORMAL_BIT | TEXCOORD_BIT | COLOR_BIT;

	bool operator == (const Vertex3DNormalTextureColor& other) const {
		return memcmp(this, &other, sizeof(*this)) == 0;
	}
};


#include "VertexDescription.h"	// the above will all need this,
								//	so include for convenience

#endif // Vertex3DTypes_h
