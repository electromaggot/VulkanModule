//
// DynamicRenderable.cpp
//	VulkanModule AddOns
//
// See header file comment for overview.
//
// 3/26/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "DynamicRenderable.h"
#include "PrimitiveBuffer.h"


DynamicRenderable::DynamicRenderable(Renderable& renderable, VulkanSetup& vulkan, iPlatform& platform)
	:	iRenderable(renderable, vulkan, platform)
{ }


void DynamicRenderable::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex)
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
