//
// FixedRenderable.cpp
//	VulkanModule AddOns
//
// See header file comment for overview.
//
// 3/26/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "FixedRenderable.h"
#include "PrimitiveBuffer.h"


FixedRenderable::FixedRenderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform, VkRenderPass customRenderPass, VkExtent2D customExtent)
	:	iRenderable(drawable, vulkan, platform, customRenderPass, customExtent)
{ }


void FixedRenderable::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex)
{
	static int logCount = 0;
	static const int MAX_LOGS = 10;  // Log first 10 renderables
	if (logCount < MAX_LOGS) {
		Log(LOW, "FixedRenderable::IssueBindAndDrawCommands [%d] for %s", logCount, name.c_str());
		Log(LOW, "  vertexCount=%u, indexCount=%u, instanceCount=%u",
			vertexObject.vertexCount, vertexObject.indexCount, vertexObject.instanceCount);
		if (hasDynamicOffset) {
			Log(LOW, "  hasDynamicOffset=true, dynamicOffset=%u", dynamicOffset);
		}
		if (descriptors.exist()) {
			Log(LOW, "  descriptors exist: %p, bufferIndex=%d", &descriptors.getSets()[bufferIndex], bufferIndex);
		} else {
			Log(LOW, "  WARNING: descriptors DO NOT exist!");
		}
		logCount++;
	}

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());

	if (descriptors.exist()) {
		if (hasDynamicOffset) {
			// Bind descriptor with dynamic offset for per-object data:
			uint32_t dynamicOffsets[] = { dynamicOffset };
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
									pipeline.getPipelineLayout(), 0, 1,
									&descriptors.getSets()[bufferIndex], 1, dynamicOffsets);
		} else {
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
									pipeline.getPipelineLayout(), 0, 1,
									&descriptors.getSets()[bufferIndex], 0, nullptr);
		}
	}

	if (addOns.pVertexBuffer) {
		VkBuffer vertexBuffers[] = { addOns.pVertexBuffer->getVk() };
		VkDeviceSize offsets[]	 = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	}
	if (addOns.pIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, addOns.pIndexBuffer->getVk(),
							 0, VkIndexTypes[vertexObject.indexType]);

		vkCmdDrawIndexed(commandBuffer, vertexObject.indexCount, vertexObject.instanceCount,
										vertexObject.firstIndex, vertexObject.vertexOffset,
										vertexObject.firstInstance);
	} else {
		// Debug log draw calls for first few renderables.
		static int drawLogCount = 0;
		if (drawLogCount < 6) {
			Log(LOW, "  vkCmdDraw: vertexCount=%u, instanceCount=%u, firstVertex=%u, firstInstance=%u",
				vertexObject.vertexCount, vertexObject.instanceCount,
				vertexObject.firstVertex, vertexObject.firstInstance);
			drawLogCount++;
		}
		vkCmdDraw(commandBuffer, vertexObject.vertexCount, vertexObject.instanceCount,
								 vertexObject.firstVertex, vertexObject.firstInstance);
	}
}
