//
// Renderable.h
//	VulkanModule AddOns
//
// Unified renderable class that combines fixed and dynamic rendering capabilities.
// Defaults to re-recording command buffers each frame (UPON_EACH_FRAME) for maximum
//	flexibility, which is suitable for most use cases including:
//		- Scene objects that move (via Dynamic UBO).
//		- Objects that change appearance (textures, shaders).
//		- Animated or morphing geometry.
//
// Performance note: Re-recording command buffers has minimal overhead on modern CPUs.
// For truly static geometry that never changes (e.g., skybox), use
//	Secondary Command Buffers (see SecondaryCommandBuffer class) for optimal performance.
//
// Tadd Jensen 9 Nov 2023
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Renderable_h
#define Renderable_h

#include "iRenderable.h"


struct Renderable : public iRenderable
{
public:
	// Standard constructor - uses default render pass and extent.
	Renderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform);

	// Custom constructor - allows custom render pass and extent (e.g., for shadow mapping).
	Renderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform,
			   iRenderPass* pCustomRenderPass, VkExtent2D customExtent);

	// Returns command recording mode - defaults to UPON_EACH_FRAME for flexibility.
	iRenderable* newConcretion(CommandRecording* pRecordingMode) const override
	{
		*pRecordingMode = UPON_EACH_FRAME;
		return new Renderable(*this);
	}

	// Record Vulkan draw commands into the command buffer.
	//	skipPipelineBind: Set to true when using batched rendering (pipeline already bound by batch manager).
	void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex) override;
	void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex, bool skipPipelineBind);

	// Dynamic UBO support for efficient per-object transforms.
	uint32_t dynamicOffset = 0;
	bool  hasDynamicOffset = false;
};

#endif	// Renderable_h
