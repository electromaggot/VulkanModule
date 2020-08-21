//
// DearImGui.h
//	General App Chassis, Platform Layer, GUI System, Vulkan-centric
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
	~DearImGui();

		// METHODS

	iRenderableBase* newConcretion(CommandRecording* pRecordingMode) const
	{
		*pRecordingMode = UPON_EACH_FRAME;

		return new DearImGui(*this);
	}

	void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex = 0);

	void Update(float deltaSeconds);

private:
	void preRender(void (*pfnLayOutGui)(iPlatform&), iPlatform& platform);
	void uploadFonts(VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue);

	VkCommandBuffer  allocateCommandBuffer(VkCommandPool commandPool, VkDevice device);
	VkDescriptorPool createDescriptorPool(VkDevice device);

		// MEMBERS

	VkResult	 err;
	iPlatform&	 platform;
	VkDevice&	 device;		// (saved for destruction)
};

#endif	// DearImGui_h
