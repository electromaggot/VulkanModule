//
// iGuiSystem.h
//	Platform-related "Interface"/Base Class
//	General App Chassis, GUI System, Vulkan-centric
//
// Created 3/22/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef iGuiSystem_h
#define iGuiSystem_h

#include "iRenderable.h"


class iGuiSystem {
public:
	virtual void Update() { }
	virtual void Draw()   { }

//	virtual Renderable getRenderable() { return Renderable(); }
};

#endif // iGuiSystem_h
