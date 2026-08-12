//
// ShadowPass.cpp
//	VulkanModule - Shadow Mapping Pass Management
//
// Implement command buffer recording (one) for shadow map image (multiple).
//	See header file for details.
//
// Created 1 Oct 2025 by Tadd Jensen
//  © 0000 (uncopyrighted; use at will)
//
#include "ShadowPass.h"
#include "VulkanSetup.h"
#include "VulkanConfigure.h"
#include "iRenderable.h"
#include "Logging.h"


ShadowPass::ShadowPass(VulkanSetup& vulkan, ShadowMap& shadowMap)
	: vulkan(vulkan)
	, shadowMap(shadowMap)
	, device(vulkan.device)
	, commandPool(vulkan.command.getCommandPool())
{
	uint32_t numFrames = vulkan.swapchain.getNumImages();
	allocateCommandBuffers(numFrames);
}

ShadowPass::~ShadowPass()
{
	freeCommandBuffers();
}

void ShadowPass::allocateCommandBuffers(uint32_t numFrames)
{
	shadowCommandBuffers.resize(numFrames);

	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = commandPool.getVkCommandPool(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = numFrames
	};

	VkResult result = vkAllocateCommandBuffers(device.getLogical(), &allocInfo, shadowCommandBuffers.data());
	if (result != VK_SUCCESS) {
		Fatal("Failed to allocate shadow command buffers!" + ErrStr(result));
	}
	Log(LOW, "ShadowPass: Allocated %u shadow command buffers", numFrames);
}

void ShadowPass::freeCommandBuffers()
{
	if (!shadowCommandBuffers.empty()) {
		vkFreeCommandBuffers(device.getLogical(), commandPool.getVkCommandPool(),
							 static_cast<uint32_t>(shadowCommandBuffers.size()),
							 shadowCommandBuffers.data());
		shadowCommandBuffers.clear();
	}
}

void ShadowPass::recreate(uint32_t numFrames)
{
	freeCommandBuffers();
	allocateCommandBuffers(numFrames);
}

void ShadowPass::recordShadowPass(std::vector<iRenderable*>& shadowRenderables, uint32_t frameIndex, ShadowMap& shadowMapForFrame)
{
	VkCommandBuffer commandBuffer = shadowCommandBuffers[frameIndex];

	// Begin command buffer
	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pInheritanceInfo = nullptr
	};

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Begin shadow render pass
	// Note: Render pass handles layout transitions automatically:
	//   - initialLayout (READ_ONLY) -> attachment layout (ATTACHMENT) at start.
	//   - attachment layout -> finalLayout (READ_ONLY) at end.
	VkClearValue clearValue = { .depthStencil = { 1.0f, 0 } };

	VkRenderPassBeginInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = nullptr,
		.renderPass = shadowMapForFrame.getVkRenderPass(),
		.framebuffer = shadowMapForFrame.getFramebuffer(),
		.renderArea = {
			.offset = { 0, 0 },
			.extent = { shadowMapForFrame.getWidth(), shadowMapForFrame.getHeight() }
		},
		.clearValueCount = 1,
		.pClearValues = &clearValue
	};

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Record shadow renderables (render geometry from light's perspective for depth)
	for (iRenderable* pRenderable : shadowRenderables) {
		pRenderable->IssueBindAndDrawCommands(commandBuffer, frameIndex);
	}

	vkCmdEndRenderPass(commandBuffer);
	// Render pass automatically transitioned image back to READ_ONLY layout (finalLayout).

	vkEndCommandBuffer(commandBuffer);
}
