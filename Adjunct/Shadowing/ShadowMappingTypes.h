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
	SHADOW_CAMERA_STRAIGHT_DOWN,      // Points down -Y axis (default, prevents clipping with wide FOV),
									  //	perhaps mimics lamp with top reflector.
	SHADOW_CAMERA_CUSTOM_DIRECTION,   // Uses custom direction vector (e.g. for spotlights), set via
									  //	separate parameter.
	SHADOW_CAMERA_LOOK_AT_ORIGIN      // Looks from light position toward scene origin.
};


#endif // ShadowMappingTypes_h
