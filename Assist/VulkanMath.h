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


// Flips the world to +Z forward, left-handed (Direct3D/Unity-style) instead of Vulkan's
//	standard right-handed +Z-out-of-screen.  Its sole effect is to reverse GraphicsPipeline's
//	default VkFrontFace (see GraphicsPipeline.cpp), since left-handed geometry winds opposite.
//
// This defaults OFF so that this module behaves like standard Vulkan out-of-the-box: a
//	consumer that asks for nothing special gets counter-clockwise front faces, and geometry
//	authored to the usual Vulkan/glm conventions renders right-side-out.  Applications built
//	around a left-handed world (e.g. one that flies a camera forward into ascending +Z) opt in
//	from their own build, leaving this library free of any single application's convention:
//
//		target_compile_definitions(MyApp PRIVATE INVERT_Z_SETTING=true)
//
#ifndef INVERT_Z_SETTING
	#define INVERT_Z_SETTING false
#endif

constexpr bool INVERT_Z = INVERT_Z_SETTING;

enum CoordinateSystem {
	VULKAN_OPENGL_RIGHT_HANDED,		// +Z out of screen
	DIRECT3D_UNITY_LEFT_HANDED		// +Z into screen  (which may be switched to, per 3D-world context)
};


#if NO_GLM		// (if your needs are ultra simple...)
	typedef float vec2[2];
	typedef float vec3[3];	// Note that these floats match SFLOAT in VK_FORMAT.
	typedef float mat4[4][4];
#else
	// (Make sure glm's base directory is in your project's header paths!)

	//#define GLM_FORCE_LEFT_HANDED			// See note at file's end for why this is excluded...
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
//	Our engine allows models of either LH ("designed for Unity") or RH ("designed
//	for OpenGL") based on the MODELED_FOR_DIRECT3D Customizer.h flag, which can
//	selectively call either methods at run-time.  However GLM_FORCE_LEFT_HANDED
//	is a compile-time #define, so henceforth applies to all rendering.
// Therefore GLM_FORCE_LEFT_HANDED is excluded and its effect otherwise
//	applied by explicit alternative operations just mentioned.
