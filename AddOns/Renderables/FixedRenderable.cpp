//
// FixedRenderable.cpp
//	VulkanModule AddOns
//
// Created 3/26/20 by Tadd
//	© 2020 Megaphone Studios
//
#include "FixedRenderable.h"
#include "PrimitiveBuffer.h"

// instantiate the otherwise-abstract static
vector<iRenderable*>	Renderables::pRenderables;


FixedRenderable::FixedRenderable(Renderable& renderable, VulkanSetup& vulkan, iPlatform& platform)
	:	iRenderable(renderable, vulkan, platform)
{ }
/*FixedRenderable::~FixedRenderable()	// ~iRenderable() should be automatic
{
	//iRenderable::~iRenderable();	//TJ: necessary? or automatic?
}*/


void FixedRenderable::IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());

	//if (pDescriptors) // if (addedOn.pUniformBuffer)
	if (descriptors.exist())
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipeline.getPipelineLayout(), 0, 1,
								&descriptors.getSets()[0], 0, nullptr);
								 //TJ_TODO_IMPORTANT! ^^^ THIS NEEDS TO BE THE BUFFER INDEX?

	if (addOns.pVertexBuffer) {
		VkBuffer vertexBuffers[] = { addOns.pVertexBuffer->getVkBuffer() };
		VkDeviceSize offsets[]	 = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	}
	if (addOns.pIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, addOns.pIndexBuffer->getVkBuffer(),
							 0, IndexBufferIndexTypeEnum);

		vkCmdDrawIndexed(commandBuffer, vertexObject.indexCount, vertexObject.instanceCount,
										vertexObject.firstIndex, vertexObject.vertexOffset,
										vertexObject.firstInstance);
	} else
		vkCmdDraw(commandBuffer, vertexObject.vertexCount, vertexObject.instanceCount,
								 vertexObject.firstVertex, vertexObject.firstInstance);
}
