//
// VulkanMath.h
//	Vulkan Module
//
// To assist in using Vulkan, embraces and relies-on GLM.
//		https://glm.g-truc.net/0.9.9/		https://github.com/g-truc/glm
//	(Although in the most simplistic of use-cases, it may not be absolutely necessary.)
//
// Created 3/23/23 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef VulkanMath_h
#define VulkanMath_h


enum CoordinateSystem {
	OPENGL_RIGHT_HANDED,
	VULKAN_LEFT_HANDED
};


#if NO_GLM		// (if your needs are ultra simple...)
	typedef float vec2[2];
	typedef float vec3[3];	// Note that these floats match SFLOAT in VK_FORMAT.
	typedef float mat4[4][4];
#else
	// (Make sure glm's base directory is in your project's header paths!)

	//#define GLM_FORCE_LEFT_HANDED			// See note at file's end for why this is now excluded...
	#define GLM_FORCE_RADIANS
	#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
	#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// Vulkan uses [0,1] depth range (OpenGL uses [-1,1])
	#include <glm/glm.hpp>
	#include <glm/gtc/matrix_transform.hpp>

	using glm::vec2; using glm::vec3; using glm::vec4;
	using glm::mat3; using glm::mat4;
	using glm::radians;
#endif


#endif // VulkanMath_h


// COORDINATE SYSTEM NOTE
// Despite our codebase being Vulkan-centric, GLM_FORCE_LEFT_HANDED does not seem
//	necessary because of the way that glm::lookAt() and glm:perspective() are called,
//	then the Projection matrix specifically altered or Clip Space correction applied.
//	LunarG's Vulkan sample code does this, acting on models that appear Right-Handed
//	and have Clockwise-wound front-facing triangles.
// At the same time, GLM_FORCE_LEFT_HANDED seems to invoke variants of the aforenamed
//	methods, such as lookAtLH() vs. lookAtRH() or perspectiveLH() vs. perspectiveRH().
//	Our engine allows models of either LH ("designed for Vulkan") or RH ("designed
//	for OpenGL") based on the MODELED_FOR_VULKAN Customizer.h flag, which can
//	selectively call either methods at run-time.  However GLM_FORCE_LEFT_HANDED
//	is a compile-time #define, so henceforth applies to all rendering.
// Therefore GLM_FORCE_LEFT_HANDED is excluded and its effect otherwise
//	applied by explicit alternative operations just mentioned.
