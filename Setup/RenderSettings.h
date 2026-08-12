//
// RenderSettings.h
//	Vulkan Setup
//
// Render-related variables, encapsulated.
//
// 7/15/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef RenderSettings_h
#define RenderSettings_h

struct
{
	const VkBool32	useAnisotropy	= true;		// Enable for sharper textures at oblique angles (floors, walls).
												//	(↑ disable for performance)
	const float		anisotropyLevel	= 16;		//	(or decrease this value) (values > 16 superfluous)

	const VkBool32	useMipLod		= true;		// Enable Mipmapping/Level-of-detail (especially for grid floor).

} RenderSettings;

#endif	// RenderSettings_h
