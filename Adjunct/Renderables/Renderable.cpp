//
// Renderable.cpp
//	VulkanModule AddOns
//
// See header file comment for overview.
//
// Tadd Jensen 9 Nov 2023
//	© 0000 (uncopyrighted; use at will)
//
#include "Renderable.h"
#include "PrimitiveBuffer.h"
#include "Logging.h"


Renderable::Renderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform)
	:	iRenderable(drawable, vulkan, platform)
{ }

Renderable::Renderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform,
					   iRenderPass* pCustomRenderPass, VkExtent2D customExtent)
	:	iRenderable(drawable, vulkan, platform, pCustomRenderPass, customExtent)
{ }


void Renderable::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int iFrame)
{
	IssueBindAndDrawCommands(commandBuffer, iFrame, false);
}

void Renderable::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int iFrame, bool skipPipelineBind)
{
	if (! skipPipelineBind)			// Bind graphics pipeline, unless using batched rendering where pipeline already bound.
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());

	if (descriptors.exist()) {		// Bind descriptor sets.
		// DIAGNOSTIC: Check for out-of-bounds or null descriptor set access.
		auto& sets = descriptors.getSets();
		if (iFrame < 0 || iFrame >= (int)sets.size()) {
			Log(ERROR, "DESCRIPTOR OUT-OF_BOUNDS: '%s' iFrame=%d sets.size()=%zu", name.c_str(), iFrame, sets.size());
			return;		// Skip draw to avoid crash.
		}
		if (sets[iFrame] == VK_NULL_HANDLE) {
			Log(ERROR, "DESCRIPTOR NULL: '%s' iFrame=%d", name.c_str(), iFrame);
			return;		// Skip draw to avoid crash.
		}
		if (hasDynamicOffset) {		// Bind descriptor with dynamic offset for per-object transforms via Dynamic UBO.
			uint32_t dynamicOffsets[] = { dynamicOffset };
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
									pipeline.getPipelineLayout(), 0, 1, &sets[iFrame], 1, dynamicOffsets);
		} else						// Standard descriptor binding without dynamic offset.
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
									pipeline.getPipelineLayout(), 0, 1, &sets[iFrame], 0, nullptr);
	}

	if (addOns.pVertexBuffer) {		// Bind vertex buffer.
		VkBuffer vertexBuffers[] = { addOns.pVertexBuffer->getVk() };
		VkDeviceSize offsets[]	 = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	}

	if (addOns.pIndexBuffer) {		// Draw indexed or non-indexed.
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
