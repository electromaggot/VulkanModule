//
// ShadowMappingTypes.h
//
// Common type definitions for shadow mapping system.
//	Separated from implementation to minimize dependencies.
// Customizes shadowing configuration.
//	Shadow mapping provides realistic shadows by rendering the scene from the light's perspective.
//	Quality/performance trade-offs can be tuned by constants via the enumerations below.
//
// Created 3 Oct 2025 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ShadowMappingTypes_h
#define ShadowMappingTypes_h


// Shadow mapping algorithm selection.
enum ShadowTechnique {
	SHADOW_TECHNIQUE_NONE,      // Shadows disabled - zero VRAM allocation.
	SHADOW_TECHNIQUE_BASIC      // Basic shadow mapping (current implementation).
	// Future: SHADOW_TECHNIQUE_CASCADE, SHADOW_TECHNIQUE_VSM, etc.
};

// Shadow projection mode - how simulated light rays are cast.
enum ShadowProjectionMode {
	SHADOW_ORTHOGRAPHIC,    // Directional/sun light (parallel rays) - faster, uniform shadow quality.
	SHADOW_PERSPECTIVE      // Point light source (radial rays from position) - matches Phong lighting,
};							//													accurate shadow positioning.

// Shadow camera orientation mode - direction the shadow camera looks.
enum ShadowCameraMode {
	SHADOW_CAMERA_CUSTOM_DIRECTION,   // Uses custom direction vector (e.g. directional/sun light or
									  //	spotlights), set via separate parameter.
	SHADOW_CAMERA_LOOK_AT_TARGET      // Looks from light position toward a specified target point
									  //	(e.g. scene focus for point light shadows).
};


// *_CUSTOM_DIRECTION defaults to -Y direction, replacing former SHADOW_CAMERA_STRAIGHT_DOWN which
//		was meant to prevent clipping with wide FOV, perhaps mimicking lamp with top reflector.
// *_LOOK_AT_TARGET defaults to origin, similarly replacing the once SHADOW_CAMERA_LOOK_AT_ORIGIN.

#endif // ShadowMappingTypes_h
