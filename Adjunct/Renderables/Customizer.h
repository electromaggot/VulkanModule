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
	NONE				= 0,
	WIREFRAME			= 0b00000001,	// e.g. render with: VK_POLYGON_MODE _LINE instead of _FILL
	SHOW_BACKFACES		= 0b00000010,	// override default: VK_CULL_MODE_BACK_BIT with _NONE
	FRONT_CLOCKWISE		= 0b00000100,	//		   i.e. NOT: VK_FRONT_FACE_COUNTER_CLOCKWISE
	MODELED_FOR_VULKAN	= 0b00001000,	// versus default, model created for OpenGL + Right-Handed
	ALPHA_BLENDING		= 0b00010000	// enable alpha blending for transparency (billboards, particles, etc.)
};

inline Customizer operator | (Customizer left, Customizer right)
{
  return static_cast<Customizer>(static_cast<int>(left) | static_cast<int>(right));
}

#endif	// Customizer_h
