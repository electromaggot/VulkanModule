
#include "../imgui.h"	// See if IMGUI_DISABLEd -
#ifndef IMGUI_DISABLE	//	- not disabled, compile real ImGui.
	#include "../../External/imgui-src/imgui.cpp"
	#include "../../External/imgui-src/imgui_draw.cpp"		// Do not specify any of these original
	#include "../../External/imgui-src/imgui_tables.cpp"	//	ImGui source files in your project
	#include "../../External/imgui-src/imgui_widgets.cpp"	//	to be compiled!  Otherwise you
															//	defeat the purpose here, which
	#include "../../External/implot-src/implot.cpp"			//	is to make ImGui disableable.
	#include "../../External/implot-src/implot_items.cpp"
#else	// - disabled: provide stub functions (& let compiler optimize these away)...

//
// imgui.cpp (stub version)
//	Stub implementation for ImGui when GUI disabled.
//
// Created 11/23/25 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "imgui.h"

// Static member is now a local static in GetIO() to avoid linker duplicate symbols

#endif	// IMGUI_DISABLE
