#include "imgui.h"
#ifdef IMGUI_DISABLE
	#include "stubs/sDearImGui.h"	// (overrides all below)
#endif
//
// DearImGui.h
//	General App Chassis, Platform Layer, GUI System, Vulkan-centric
//
// Object representing Dear ImGui setup, persistence, and rendering, inspired
//	by code originally found in: imgui/examples/example_sdl_vulkan/main.cpp
//
// Created 3/22/20 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DearImGui_h
#define DearImGui_h

#include "iPlatform.h"
#include "iRenderable.h"


class DearImGui : public iRenderableBase {
public:
	DearImGui(VulkanSetup& vulkan, iPlatform& platform);
	DearImGui(const DearImGui& other);  	// copy constructor
	~DearImGui();

private:
	static bool s_imguiShutdown;	// Track if ImGui has been shut down. (shared across all instances)

		// METHODS

	iRenderableBase* newConcretion(CommandRecording* pRecordingMode) const
	{
		*pRecordingMode = UPON_EACH_FRAME;

		return new DearImGui(*this);
	}

	void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex = 0);

	bool Update(GameClock& time);

private:
	void preRender(void (*pfnLayOutGui)(DearImGui&), iPlatform& platform);

	VkCommandBuffer  allocateCommandBuffer(VkCommandPool commandPool, VkDevice device);
	VkDescriptorPool createDescriptorPool(VkDevice device);

		// MEMBERS

	VkResult	err;
	VkDevice&	device;				// (saved for destruction)
	VkDescriptorPool descriptorPool;	// ImGui descriptor pool (must be destroyed manually)

	string		iniFileName;		// https://github.com/ocornut/imgui/issues/454

public:		// made available to MainGui(this):
	iPlatform&	platform;
};

#endif	// DearImGui_h


/* DEV NOTE
Font upload is now automatic in ImGui docking branch (2025), as the following was removed:
	void uploadFonts(VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue);
*/
