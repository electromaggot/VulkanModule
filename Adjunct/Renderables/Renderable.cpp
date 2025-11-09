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


Renderable::Renderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform)
	:	iRenderable(drawable, vulkan, platform)
{ }

Renderable::Renderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform,
					   iRenderPass* pCustomRenderPass, VkExtent2D customExtent)
	:	iRenderable(drawable, vulkan, platform, pCustomRenderPass, customExtent)
{ }


void Renderable::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex)
{
	IssueBindAndDrawCommands(commandBuffer, bufferIndex, false);
}

void Renderable::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex, bool skipPipelineBind)
{
	// Bind graphics pipeline, unless using batched rendering where pipeline already bound.
	if (! skipPipelineBind)
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());

	if (descriptors.exist()) {		// Bind descriptor sets.
		if (hasDynamicOffset) {
					// Bind descriptor with dynamic offset for per-object transforms via Dynamic UBO.
			uint32_t dynamicOffsets[] = { dynamicOffset };
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
									pipeline.getPipelineLayout(), 0, 1,
									&descriptors.getSets()[bufferIndex], 1, dynamicOffsets);
		} else {	// Standard descriptor binding without dynamic offset.
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
									pipeline.getPipelineLayout(), 0, 1,
									&descriptors.getSets()[bufferIndex], 0, nullptr);
		}
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
