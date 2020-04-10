//
// DearImGui.h
//	General App Chassis, Platform Layer, GUI System, Vulkan-centric
//
// Created 3/22/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DearImGui_h
#define DearImGui_h

#include "iGuiSystem.h"

#include "iPlatform.h"
#include "VulkanPlatform.h"
#include "VulkanSetup.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl.h"


class DearImGui : public iGuiSystem {
public:
	DearImGui(VulkanSetup& vulkan, iPlatform& platform);
	~DearImGui();

	void uploadFonts(VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue);

private:
	VkResult err;

	VkDevice& device;

	VkFramebuffer* framebuffers; //TEMPORARY!

	ImGui_ImplVulkanH_Window guiWindow;	//TEMPORARY?

	VkCommandPool	commandPool;	// Handle for shared/overall CommandPool
	VkQueue			graphicsQueue;	//	and one for existing Queue.
	VkCommandBuffer	commandBuffer;	// New custom CommandBuffer for GUI.

public:
	void Update();
	void PreRender(void (*pfnLayOutGui)(), iPlatform& platform);

private:
	void FrameRender();
	void FramePresent(); //WANT THIS TO NOT BE USED, in favor of my unified one
};

#endif // DearImGui_h
