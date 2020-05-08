//
// imgui_impl_sdl.h
//	Dummy placeholder for Dear ImGui or other GUI system.
//
// If you want to actually add Dear ImGui to your project, remove from it the
//	reference to this file and replace with one to ImGui's actual imgui_impl_sdl.h
//
// Created 3/17/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef imgui_impl_sdl_h
#define imgui_impl_sdl_h

#include <SDL.h>


void ImGui_ImplSDL2_InitForVulkan(SDL_Window* pWindow);

void ImGui_ImplSDL2_NewFrame(SDL_Window* pWindow);


#endif	// imgui_impl_sdl_h
