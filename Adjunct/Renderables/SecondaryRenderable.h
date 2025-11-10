//
// SecondaryRenderable.h
//	VulkanModule AddOns
//
// Renderable class for static geometry using Vulkan Secondary Command Buffers.
//
// Secondary command buffers are recorded once at initialization time (AT_INIT_TIME_ONLY)
//	and executed repeatedly within primary command buffers via vkCmdExecuteCommands().
// This is the most efficient approach for truly static geometry like skyboxes, static
//	environments, or unchanging decorative elements.
//
// Benefits vs. re-recording each frame:
//	- Zero CPU overhead per frame (no recording).
//	- Commands recorded once and reused indefinitely.
//	- Perfect for large static meshes or complex geometry.
//
// When to use SecondaryRenderable vs. Renderable:
//	- Use SecondaryRenderable: Skybox, static environments, unchanging decorations.
//	- Use Renderable: Animated objects, objects with changing transforms via Dynamic UBO,
//					  objects that may toggle visibility or change appearance.
//
// Created 9 Nov 2024 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef SecondaryRenderable_h
#define SecondaryRenderable_h

#include "iRenderable.h"


struct SecondaryRenderable : public iRenderable
{
public:
	// Standard constructor - uses default render pass and extent.
	SecondaryRenderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform);

	// Custom constructor - allows custom render pass and extent (e.g., for shadow mapping).
	SecondaryRenderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform,
						iRenderPass* pCustomRenderPass, VkExtent2D customExtent);

	~SecondaryRenderable();

	// Returns command recording mode - always AT_INIT_TIME_ONLY for secondary command buffers.
	iRenderable* newConcretion(CommandRecording* pRecordingMode) const override
	{
		*pRecordingMode = AT_INIT_TIME_ONLY;
		return new SecondaryRenderable(*this);
	}

	// Record Vulkan draw commands into secondary command buffer.
	//	Note: This is called during initialization to pre-record commands.
	void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex) override;

	// Allocate and record secondary command buffers (one per frame).
	void AllocateSecondaryCommandBuffers(VulkanSetup& vulkan);

	// Get secondary command buffer for a specific frame.
	VkCommandBuffer GetSecondaryCommandBuffer(int frameIndex) const override
	{
		return secondaryCommandBuffers[frameIndex];
	}

	// Check if this renderable uses secondary command buffers.
	bool IsSecondaryCommandBuffer() const override { return useSecondaryCommandBuffers; }

private:
	// Secondary command buffer state
	std::vector<VkCommandBuffer> secondaryCommandBuffers;	// One per frame
	bool useSecondaryCommandBuffers = true;
	VulkanSetup* pVulkan = nullptr;		// Store for allocation/cleanup.
};

#endif	// SecondaryRenderable_h
