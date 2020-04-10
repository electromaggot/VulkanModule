//
// AddOns.cpp
//	VulkanModule AddOns
//
// See header for overview.
//
// Created 3/26/20 by Tadd
//	© 2020 Megaphone Studios
//
#include "AddOns.h"

#include "iRenderable.h"
#include "PrimitiveBuffer.h"


AddOns::AddOns(Renderable& renderable, VulkanSetup& setup, iPlatform& abstractPlatform)
	:	vulkan(setup),
		platform(abstractPlatform)
{
	createVertexAndOrIndexBuffers(renderable.vertexSpec);
	createDescribedItems(renderable.pUBOs.data(), renderable.textureSpecs.data(), abstractPlatform);
}

AddOns::~AddOns()
{
	delete pVertexBuffer;
	delete pIndexBuffer;
}


void AddOns::createVertexAndOrIndexBuffers(VertexBasedObject& vertexObject)
{
	if (vertexObject.vertices) {
		pVertexBuffer = new PrimitiveBuffer(vertexObject, vulkan.commandPool.getVkInstance(), vulkan.device);
		if (vertexObject.indices)
			pIndexBuffer = new PrimitiveBuffer((IndexBufferIndexType*) vertexObject.indices,
																	   vertexObject.indexCount,
											   vulkan.commandPool.getVkInstance(), vulkan.device);
	}
}


void AddOns::Recreate(VertexBasedObject& vertexObject/*, CommandPool& commandPool, GraphicsDevice& device / *Descriptors* pDescriptors*/)
{
	if (vertexObject.vertices) {	// (if new vertices exist to overwrite the old ones)
		delete pVertexBuffer;
		pVertexBuffer = nullptr;

		delete pIndexBuffer;		// (delete these regardless of if new indices exist to overwrite old ones)
		pIndexBuffer = nullptr;
	}
	createVertexAndOrIndexBuffers(vertexObject);
}


// Assemble a collection of Descriptors to be "added on."  Ordering is critical:
//	make sure each INDEX matches its "layout(binding = <INDEX>)" in your Shader...
//																					// e.g. :
void AddOns::createDescribedItems(UBO* pUBO, TextureSpec textureSpecs[],
								  iPlatform& platform)
{
	// Uniform Buffer Objects first (explicitly: the MVP UBO)
	if (pUBO) {
		pUniformBuffer = new UniformBuffer(pUBO->byteSize, vulkan.swapchain, vulkan.device);
		shaderStageForUBO = pUBO->getShaderStageFlags();
		described.emplace_back( pUniformBuffer->getDescriptorBufferInfo(),			// layout(binding = 0)
								shaderStageForUBO);
	}

	// Textures next (may be more than one)... order is important here too == binding index
	for (TextureSpec* pTextureSpec = textureSpecs; pTextureSpec && pTextureSpec->fileName; ++pTextureSpec) {
		TextureImage* pTexture = new TextureImage(*pTextureSpec, vulkan.commandPool.getVkInstance(), vulkan.device, platform);
		if (pTexture) {
			pTextureImages.emplace_back(pTexture);
			described.emplace_back( pTexture->getDescriptorImageInfo(),				// layout(binding = 1) ... 2) ... 3)...
									VK_SHADER_STAGE_FRAGMENT_BIT);
									// ^^^^^^ TODO: ^^^^^^^^ We don't have a mechanism (YET!) allowing an image to
		}							//		be specified for the VERTEX STAGE, which could be helpful for something
	}								//		like offseting vertices based on a depth map.
}


//TJ_TODO: This looks like it should be moved back into VulkanSetup, so that it itself is
//	calling things like " swapchain.Recreate() " ... that doesn't really seem like our job
//	here, just because we happen to (perhaps inappropriately) have a pointer to VulkanSetup.
//															...but what *can* we Recreate?...
void AddOns::RecreateRenderingRudiments()
{
	//const bool reloadMesh = false;

	/*vkDeviceWaitIdle(vulkan.device.getLogical());
	vulkan.swapchain.Recreate();
	vulkan.framebuffers.Recreate(vulkan.swapchain, vulkan.renderPass);*/

	//VertexType* pVertexType = pVertexObject ? const_cast<VertexType*>(&pVertexObject->vertexType) : nullptr;

	if (pUniformBuffer)
		pUniformBuffer->Recreate(-1, vulkan.swapchain);

	/*vector<Described> describedAddOns = reDescribe();
	if (pDescriptors)
		pDescriptors->Recreate(describedAddOns, vulkan.swapchain);

	pPipeline->Recreate(*pShaderModules, vulkan.renderPass, vulkan.swapchain, pVertexType, pDescriptors);
	pCommandObjects->Recreate(*pPipeline, vulkan.framebuffers, vulkan.renderPass, vulkan.swapchain,
							  reloadMesh);*/
}

// Note that TextureImages are specifically not reloaded or regenerated.
//	This may be necessary later, should the images change or animate, although
//	loading entirely new images (and discarding old ones) is separate matter.
//
vector<DescribEd> AddOns::reDescribe()
{
	vector<DescribEd> redescribedAddOns;
	if (pUniformBuffer)
		redescribedAddOns.emplace_back(pUniformBuffer->getDescriptorBufferInfo(),
									   shaderStageForUBO);
	for (auto& pTextureImage : pTextureImages)
		redescribedAddOns.emplace_back(pTextureImage->getDescriptorImageInfo(),
									   VK_SHADER_STAGE_FRAGMENT_BIT);
	return redescribedAddOns;
}
