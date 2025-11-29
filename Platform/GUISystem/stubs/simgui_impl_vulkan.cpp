//
// imgui_impl_vulkan.cpp
//	Dummy placeholder for Dear ImGui Vulkan backend.
//
// Tadd Jensen 9 Nov 2023
//	© 0000 (uncopyrighted; use at will)
//
#include "imgui_impl_vulkan.h"

bool ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo* info, VkRenderPass render_pass)
{
	return true;
}

void ImGui_ImplVulkan_Shutdown()
{ }

void ImGui_ImplVulkan_NewFrame()
{ }

bool ImGui_ImplVulkan_CreateFontsTexture()
{
	return true;
}

void ImGui_ImplVulkan_DestroyFontsTexture()
{ }
