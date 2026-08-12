
#include "../imgui.h"	// See if IMGUI_DISABLEd -
#ifndef IMGUI_DISABLE	//	- not disabled, compile real ImGui file:
	#include "../../External/imgui-src/backends/imgui_impl_vulkan.cpp"
#else	// - disabled: provide stub functions (& let compiler optimize these away)...

//
// imgui_impl_vulkan.cpp (stub version)
//	No-op stub for Dear ImGui Vulkan backend when GUI disabled.
//
// Tadd Jensen 9 Nov 2023
//	© 0000 (uncopyrighted; use at will)
//
#include "simgui_impl_vulkan.h"
#include "simgui.h"


bool ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo* info)  { return true; }

void ImGui_ImplVulkan_Shutdown()			{ }

void ImGui_ImplVulkan_NewFrame()			{ }

bool ImGui_ImplVulkan_CreateFontsTexture()	{ return true; }

void ImGui_ImplVulkan_DestroyFontsTexture()	{ }

void ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, VkCommandBuffer command_buffer)  { }


#endif	// IMGUI_DISABLE
