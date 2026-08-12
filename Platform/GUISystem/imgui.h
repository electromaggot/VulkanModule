//
// imgui.h REPLACEMENT
//	General App Chassis, Platform Layer, GUI System, Vulkan-centric
//
// This file is of critical importance, as `#include "imgui.h"` is
//	 standard atop all of the app's ImGui-based GUI window files.
//	However in an effort to make ImGui "disableable" the following approach is taken:
//	 - this file replaces the original "imgui.h" and based on whether ImGui is disabled
//		or not, either in-turn #includes the original ImGui one, or a stubbed-out version,
//		as well as the other headers.  #ifdef include guards keep later references correct.
//	 - for the implementation ".cpp" files, only the stub versions are part of the build,
//		but each of those first #includes this file to grab the value of IMGUI_DISABLE,
//		then based on that, either defers to (further #include) the original ImGui file,
//		or provides its own stubs to no-op-replace all originally referenced functions.
//
// Created 3/22/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// IF YOU DO NOT WANT OR NEED ImGui PRESENT IN YOUR PROJECT,
//	UNCOMMENT THE FOLLOWING #define LINE TO GLOBALLY EXCLUDE IT.

//#define IMGUI_DISABLE

// NOTE that all 4 of the External/imgui-src/* files listed below also abide by
//	ImGui's own IMGUI_DISABLE, which when provided, effectively deactivates them.
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#ifndef IMGUI_DISABLE
	#define IMGUI_DEFINE_MATH_OPERATORS
	#include "../../External/imgui-src/imgui.h"							// Do not include these anywhere individually,
	#include "../../External/imgui-src/imgui_internal.h"				//	or you'll break the purpose here, which is
	#include "../../External/imgui-src/backends/imgui_impl_vulkan.h"	//	to make ImGui optional/disableable. Instead:
	#include "../../External/imgui-src/backends/imgui_impl_sdl2.h"		// Always just include this file, imgui.h

	#include "../../External/implot-src/implot.h"
#else
	#include "stubs/simgui.h"
	//#include "stubs/imgui_internal.h"			// (not needed, actually, although that could change someday)
	#include "stubs/simgui_impl_vulkan.h"
	#include "stubs/simgui_impl_sdl2.h"

	#include "stubs/simplot.h"
#endif
