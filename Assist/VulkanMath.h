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


#if NO_GLM		// (if your needs are ultra simple...)
	typedef float vec2[2];
	typedef float vec3[3];	// Note that these floats match SFLOAT in VK_FORMAT.
	typedef float mat4[4][4];
#else
	// (Make sure glm's base directory is in your project's header paths!)

	#define GLM_FORCE_LEFT_HANDED		// for Vulkan, opposed to OpenGL!
	#define GLM_FORCE_RADIANS
	#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
	#include <glm/glm.hpp>
	#include <glm/gtc/matrix_transform.hpp>

	using glm::vec2; using glm::vec3; using glm::vec4; using glm::mat4;
	using glm::radians;
#endif


#endif // VulkanMath_h
