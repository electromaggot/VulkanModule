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
	WIREFRAME			 = 0b00000001,	// e.g. render with: VK_POLYGON_MODE _LINE instead of _FILL
	SHOW_BACKFACES		 = 0b00000010,	// override default: VK_CULL_MODE_BACK_BIT with _NONE
	FRONT_CLOCKWISE		 = 0b00000100,	//		   i.e. NOT: VK_FRONT_FACE_COUNTER_CLOCKWISE
	MODELED_FOR_DIRECT3D = 0b00001000,	// versus default, model created for OpenGL + Right-Handed (same as Vulkan)
	ALPHA_BLENDING		 = 0b00010000,	// enable alpha blending for transparency (billboards, particles, etc.)
	LINE_TOPOLOGY		 = 0b00100000,	// use VK_PRIMITIVE_TOPOLOGY_LINE_LIST instead of TRIANGLE_LIST
	DEPTH_LEQUAL		 = 0b01000000,	// use VK_COMPARE_OP_LEQUAL instead of LESS (for skyboxes at far plane)
	DISABLE_DEPTH_TEST	 = 0b10000000	// disable depth testing (always visible, renders on top)
};

inline Customizer operator | (Customizer left, Customizer right)
{
  return static_cast<Customizer>(static_cast<int>(left) | static_cast<int>(right));
}

#endif	// Customizer_h
