//
// Customizer.h
//	VulkanModule AddOns
//
// Allow a Renderable to specify custom flags about itself to
//	receive specialized handling by the renderer.
//	Override by replacing this file with a copy (at Project level)
//	if your engine requires additional game-level customizations.
//
// Defines a bitfield so that customization may be stacked.
//
// 1/27/21 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Customizer_h
#define Customizer_h


enum Customizer
{
	NONE				 = 0,
	WIREFRAME			 = 0b00000001,	// Render with e.g. VK_POLYGON_MODE _LINE instead of _FILL.
	SHOW_BACKFACES		 = 0b00000010,	// Override default: VK_CULL_MODE_BACK_BIT with _NONE.
	FRONT_CLOCKWISE		 = 0b00000100,	//		   i.e. NOT: VK_FRONT_FACE_COUNTER_CLOCKWISE.
	MODELED_FOR_DIRECT3D = 0b00001000,	// Versus default, model created for OpenGL + Right-Handed (same as Vulkan).
	ALPHA_BLENDING		 = 0b00010000,	// Enable alpha blending for transparency, e.g. billboards, particles, etc.
										//   NOTE: Also disables depth writes (transparent objects shouldn't occlude).
	LINE_TOPOLOGY		 = 0b00100000,	// Use VK_PRIMITIVE_TOPOLOGY_LINE_LIST instead of TRIANGLE_LIST.
	POINT_TOPOLOGY		 = 0b0100000000000,	// Use VK_PRIMITIVE_TOPOLOGY_POINT_LIST.
	DEPTH_LEQUAL		 = 0b01000000,	// Use VK_COMPARE_OP_LEQUAL instead of LESS, e.g. for skyboxes at far plane.
	DISABLE_DEPTH_TEST	 = 0b10000000,	// Disable depth testing: always visible, renders on top.
	DYNAMIC_GEOMETRY	 = 0b100000000,	// Use host-visible vertex buffer for frequent updates (no command buffers).
	DISABLE_DEPTH_WRITE	 = 0b1000000000,	// Disable depth writes, e.g. for a HUD-like overlay effect.
	ALPHA_BLEND_DEPTH_WRITE = 0b10000000000	// Alpha blending WITH depth writes: blend colors but still occlude objects behind.
};

inline Customizer operator | (Customizer left, Customizer right)
{
  return static_cast<Customizer>(static_cast<int>(left) | static_cast<int>(right));
}

#endif	// Customizer_h
