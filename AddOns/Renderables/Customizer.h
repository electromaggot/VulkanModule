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
	NONE		= 0,
	WIREFRAME	= 0b00000001	// e.g. render with VK_POLYGON_MODE _LINE instead of _FILL
};


#endif	// Customizer_h
