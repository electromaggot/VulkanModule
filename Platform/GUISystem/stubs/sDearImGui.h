//
// DearImGui.h (stub version)
//	No-op stub for Dear ImGui when GUI system is disabled.
//
// Provides same interface as real DearImGui.h but does nothing.
// Used when ENABLE_IMGUI=OFF in CMake.
//
// Created 11/23/25 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DearImGui_h
#define DearImGui_h

#include "iPlatform.h"
#include "iRenderable.h"


class DearImGui : public iRenderableBase {
public:
	DearImGui(VulkanSetup& vulkan, iPlatform& platform)
		: device(vulkan.device.getLogical())
		, platform(platform)
	{
		Log(FAIL, "DearImGui disabled.");
	}

	DearImGui(const DearImGui& other)
		: device(other.device)
		, platform(other.platform)
	{ }

	~DearImGui() { }

private:
	static bool s_imguiShutdown;

		// METHODS

	iRenderableBase* newConcretion(CommandRecording* pRecordingMode) const
	{
		*pRecordingMode = UPON_EACH_FRAME;
		return new DearImGui(*this);
	}

	void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex = 0) { }

	bool Update(GameClock& time) { return true; }

		// MEMBERS

	VkDevice&	device;
	string		iniFileName;

public:
	iPlatform&	platform;
};


#endif	// DearImGui_h  (same name as non-stub)
