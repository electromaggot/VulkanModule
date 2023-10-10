//
// DrawableSpec.h
//	VulkanModule AddOns
//
// Specify a Drawable
//	A specifier/specification which specifies a Drawable, but isn't the Drawable item itself.
//	(nomenclature note at bottom)
//
// 3/26/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DrawableSpec_h
#define DrawableSpec_h

#include "ShaderModules.h"
#include "MeshObject.h"
#include "UniformBufferLiterals.h"
#include "TextureImage.h"
#include "Customizer.h"


struct DrawableProperties {
public:
	MeshObject&			mesh;
	Shaders				shaders;
	vector<UBO>			pUBOs;
	vector<TextureSpec>	textures;
	Customizer			customize = NONE;
};

class DrawableSpecifier : public DrawableProperties {
public:
	DrawableSpecifier(MeshObject& refVtxObj)
		:	DrawableProperties { refVtxObj }
	{ }		// other members, all vectors, should automatically construct empty

	DrawableSpecifier(DrawableProperties& refProps)
		:	DrawableProperties { refProps }
	{ }
};


#endif	// DrawableSpec_h


// NOMENCLATURE NOTE
// This class's name was part of a terminology change from "Renderable" to "Drawable" partly because it is
//	shorter, but mainly to further abstract the concept of a "Drawable Item" from the idea or mechanics of
//	actually rendering it.  This may eventually extend to other code still named ...Renderable... perhaps.
