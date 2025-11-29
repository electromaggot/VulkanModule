
#include "../imgui.h"	// See if IMGUI_DISABLEd -
#ifndef IMGUI_DISABLE	//	- not disabled, compile real ImGui file.
	#include "../../External/imgui-src/backends/imgui_impl_sdl2.cpp"
#else	// - disabled: provide stub functions (& let compiler optimize these away)...

//
// imgui_impl_sdl2.cpp (stub version)
//	No-op stub for Dear ImGui SDL backend when GUI disabled.
//
// Created 3/17/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "imgui_impl_sdl2.h"


bool ImGui_ImplSDL2_InitForVulkan(SDL_Window* pWindow)	  { return true; }

void ImGui_ImplSDL2_NewFrame()							  { }

bool ImGui_ImplSDL2_ProcessEvent(const SDL_Event* pEvent) { return false; }

void ImGui_ImplSDL2_Shutdown()							  { }


#endif	// IMGUI_DISABLE
