//
// FixedRenderable.cpp
//	VulkanModule AddOns
//
// See header file comment for overview.
//
// Created 3/26/20 by Tadd
//	© 2020 Megaphone Studios
//
#include "FixedRenderable.h"
#include "PrimitiveBuffer.h"


FixedRenderable::FixedRenderable(Renderable& renderable, VulkanSetup& vulkan, iPlatform& platform)
	:	iRenderable(renderable, vulkan, platform)
{ }


void FixedRenderable::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());

	if (descriptors.exist())
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipeline.getPipelineLayout(), 0, 1,
								&descriptors.getSets()[bufferIndex], 0, nullptr);

	if (addOns.pVertexBuffer) {
		VkBuffer vertexBuffers[] = { addOns.pVertexBuffer->getVk() };
		VkDeviceSize offsets[]	 = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	}
	if (addOns.pIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, addOns.pIndexBuffer->getVk(),
							 0, IndexBufferIndexTypeEnum);

		vkCmdDrawIndexed(commandBuffer, vertexObject.indexCount, vertexObject.instanceCount,
										vertexObject.firstIndex, vertexObject.vertexOffset,
										vertexObject.firstInstance);
	} else
		vkCmdDraw(commandBuffer, vertexObject.vertexCount, vertexObject.instanceCount,
								 vertexObject.firstVertex, vertexObject.firstInstance);
}
