//
// SecondaryRenderable.cpp
//	VulkanModule AddOns
//
// See header file comment for overview.
//
// Created 9 Nov 2024 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "SecondaryRenderable.h"
#include "VulkanSetup.h"
#include "CommandObjects.h"
#include "PrimitiveBuffer.h"


SecondaryRenderable::SecondaryRenderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform)
	: iRenderable(drawable, vulkan, platform), pVulkan(&vulkan)
{ }

SecondaryRenderable::SecondaryRenderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform,
										 iRenderPass* pCustomRenderPass, VkExtent2D customExtent)
	: iRenderable(drawable, vulkan, platform, pCustomRenderPass, customExtent), pVulkan(&vulkan)
{ }

SecondaryRenderable::~SecondaryRenderable()
{
	if (!secondaryCommandBuffers.empty() && pVulkan) {	// Free secondary command buffers:
		vkFreeCommandBuffers(pVulkan->device.getLogical(), pVulkan->command.getCommandPool().getVkCommandPool(),
							 (uint32_t)secondaryCommandBuffers.size(), secondaryCommandBuffers.data());
		secondaryCommandBuffers.clear();
	}
}


// Allocate secondary command buffers, one per swapchain image.
//	Must be called after construction but before first use.
//
void SecondaryRenderable::AllocateSecondaryCommandBuffers(VulkanSetup& vulkan)
{
	uint32_t numFrames = vulkan.command.NumFrames();
	secondaryCommandBuffers.resize(numFrames);

	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = vulkan.command.getCommandPool().getVkCommandPool(),
		.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,  // KEY: Secondary command buffer
		.commandBufferCount = numFrames
	};

	VkResult result = vkAllocateCommandBuffers(vulkan.device.getLogical(), &allocInfo, secondaryCommandBuffers.data());
	if (result != VK_SUCCESS)
		Fatal("Failed to allocate secondary command buffers" + ErrStr(result));

	// Record commands into each secondary command buffer.
	for (uint32_t i = 0; i < numFrames; ++i)
	{
		// Set up inheritance info - inherits render pass state from primary command buffer.
		VkCommandBufferInheritanceInfo inheritanceInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = vulkan.renderPass.getVkRenderPass(),
			.subpass = 0,
			.framebuffer = VK_NULL_HANDLE,  // Not specifying framebuffer (compatible with all)
			.occlusionQueryEnable = VK_FALSE,
			.queryFlags = 0,
			.pipelineStatistics = 0
		};

		VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,  // Will be executed within render pass.
			.pInheritanceInfo = &inheritanceInfo
		};

		result = vkBeginCommandBuffer(secondaryCommandBuffers[i], &beginInfo);
		if (result != VK_SUCCESS)
			Fatal("Failed to begin secondary command buffer" + ErrStr(result));

		IssueBindAndDrawCommands(secondaryCommandBuffers[i], i);		// Record draw commands.

		result = vkEndCommandBuffer(secondaryCommandBuffers[i]);
		if (result != VK_SUCCESS)
			Fatal("Failed to end secondary command buffer" + ErrStr(result));
	}
	Log(LOW, "SecondaryRenderable: Allocated and recorded %u secondary command buffers for %s", numFrames, name.c_str());
}

// Record Vulkan draw commands into the secondary command buffer.
//	Called during AllocateSecondaryCommandBuffers() to pre-record commands.
//
void SecondaryRenderable::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex)
{
	// Bind graphics pipeline:
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());

	if (descriptors.exist()) {		// Bind descriptor sets:
		// DIAGNOSTIC: Check for out-of-bounds or null descriptor set access.
		auto& sets = descriptors.getSets();
		if (bufferIndex < 0 || bufferIndex >= (int)sets.size()) {
			Log(ERROR, "2NDARY DESCRIPTOR OOB: '%s' bufferIndex=%d sets.size()=%zu", name.c_str(), bufferIndex, sets.size());
			return;
		}
		if (sets[bufferIndex] == VK_NULL_HANDLE) {
			Log(ERROR, "2NDARY DESCRIPTOR NULL: '%s' bufferIndex=%d", name.c_str(), bufferIndex);
			return;
		}
		if (hasDynamicOffset) {
			// Bind descriptor with dynamic offset for per-object transforms via Dynamic UBO.
			uint32_t dynamicOffsets[] = { dynamicOffset };
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
									pipeline.getPipelineLayout(), 0, 1,
									&sets[bufferIndex], 1, dynamicOffsets);
		} else {	// Standard descriptor binding without dynamic offset.
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
									pipeline.getPipelineLayout(), 0, 1,
									&sets[bufferIndex], 0, nullptr);
		}
	}

	if (addOns.pVertexBuffer) {		// Bind vertex buffer:
		VkBuffer vertexBuffers[] = { addOns.pVertexBuffer->getVk() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	}

	if (addOns.pIndexBuffer) {		// Draw indexed or non-indexed:
				// Indexed draw: use index buffer.
		vkCmdBindIndexBuffer(commandBuffer, addOns.pIndexBuffer->getVk(),
							 0, VkIndexTypes[vertexObject.indexType]);
		vkCmdDrawIndexed(commandBuffer, vertexObject.indexCount, vertexObject.instanceCount,
										vertexObject.firstIndex, vertexObject.vertexOffset,
										vertexObject.firstInstance);
	} else {	// Non-indexed draw: use vertex buffer directly.
		vkCmdDraw(commandBuffer, vertexObject.vertexCount, vertexObject.instanceCount,
								 vertexObject.firstVertex, vertexObject.firstInstance);
	}
}
