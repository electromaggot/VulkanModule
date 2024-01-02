//
// DrawableSpecifier.h
//	VulkanModule AddOns
//
// Specify a Drawable
//	A specifier/specification which specifies a Drawable, but isn't the Drawable item itself.
//	(nomenclature note at bottom)
//
// 3/26/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DrawableSpecifier_h
#define DrawableSpecifier_h

#include "ShaderModules.h"
#include "MeshObject.h"
#include "UniformBufferLiterals.h"
#include "TextureImage.h"
#include "Customizer.h"
#include "GameClock.h"


struct DrawableProperties {
public:
	MeshObject&			mesh;
	string				name;
	Shaders				shaders;
	vector<UBO>			pUBOs;
	vector<TextureSpec>	textures;
	Customizer			customize = NONE;
	bool				(*updateMethod)(GameClock&) = nullptr;
};


#define	DrawableObjectName	inline static const char*


class DrawableSpecifier : public DrawableProperties {
public:
	DrawableSpecifier(MeshObject& refVtxObj, const char* drawbjectName)
		:	DrawableProperties { refVtxObj, drawbjectName }
	{		// other members, all vectors, should automatically construct empty
		Log(RAW, "SPAWN %s...", drawbjectName);
	}

	DrawableSpecifier(DrawableProperties& refProps)
		:	DrawableProperties { refProps }
	{ }

	~DrawableSpecifier() {
		Log(RAW, "NUKED %s.", name.c_str());
	}
};


#endif	// DrawableSpecifier_h


// NOMENCLATURE NOTE
// This class's name was part of a terminology change from "Renderable" to "Drawable" partly because it is
//	shorter, but mainly to further abstract the concept of a "Drawable Item" from the idea or mechanics of
//	actually rendering it.  This may eventually extend to other code still named ...Renderable... perhaps.
