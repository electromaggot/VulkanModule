//
// AddOns.cpp
//	VulkanModule AddOns
//
// See header file comment for overview.
//
// 3/24/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "AddOns.h"

#include "CommandObjects.h"

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
	destroyVertexAndOrIndexBuffers();
}


#pragma mark - VERTEX / INDEX BUFFERS

void AddOns::createVertexAndOrIndexBuffers(VertexBasedObject& vertexObject)
{
	if (vertexObject.vertices) {
		VkCommandPool commandPool = vulkan.command.vkPool();
		pVertexBuffer = new PrimitiveBuffer(vertexObject, commandPool, vulkan.device);
		if (vertexObject.indices)
			pIndexBuffer = new PrimitiveBuffer((IndexBufferIndexType*) vertexObject.indices,
																	   vertexObject.indexCount,
											   commandPool, vulkan.device);
	}
}

void AddOns::destroyVertexAndOrIndexBuffers()
{
	delete pVertexBuffer;
	pVertexBuffer = nullptr;

	delete pIndexBuffer;
	pIndexBuffer = nullptr;
}


void AddOns::Recreate(VertexBasedObject& vertexObject)
{
	if (vertexObject.vertices) {			// (if new vertices exist to overwrite the old ones)
		destroyVertexAndOrIndexBuffers();			// <--(this also deletes index buffers regardless of
													//		if new indices exist to overwrite old ones)
		createVertexAndOrIndexBuffers(vertexObject);
	}
}


#pragma mark - UBO / TEXTURE DESCRIPTORS

// Assemble a collection of Descriptors to be "added on."  Ordering is critical:
//	make sure each INDEX matches its "layout(binding = <INDEX>)" in your Shader...
//																					// e.g. :
void AddOns::createDescribedItems(UBO* pUBO, TextureSpec textureSpecs[],
								  iPlatform& platform)
{
	// Uniform Buffer Objects first (explicitly: the MVP UBO)
	if (pUBO) {
		ubo = *pUBO;
		pUniformBuffer = new UniformBuffer(ubo.byteSize, vulkan.swapchain, vulkan.device);
		described.emplace_back( pUniformBuffer->getDescriptorBufferInfo(),			// layout(binding = 0)
								ubo.getShaderStageFlags());
	}

	// Textures next (may be more than one)... order is important here too == binding index
	for (TextureSpec* pTextureSpec = textureSpecs; pTextureSpec && pTextureSpec->fileName; ++pTextureSpec) {
		texspecs.push_back(*pTextureSpec);
		TextureImage* pTexture = new TextureImage(texspecs.back(), vulkan.command.vkPool(), vulkan.device, platform);
		if (pTexture) {
			pTextureImages.emplace_back(pTexture);
			described.emplace_back( pTexture->getDescriptorImageInfo(),				// layout(binding = 1) ... 2) ... 3)...
									VK_SHADER_STAGE_FRAGMENT_BIT);
									// ^^^^^^ TODO: ^^^^^^^^ We don't have a mechanism (YET!) allowing an image to
		}							//		be specified for the VERTEX STAGE, which could be helpful for something
	}								//		like offseting vertices based on a depth map.
}


void AddOns::RecreateDescribables()
{
	if (pUniformBuffer)
		pUniformBuffer->Recreate(-1, vulkan.swapchain);

	// Recreation (reload or regeneration) of TextureImages wasn't given much consideration, so if it is
	//	really necessary, simply destroy and reinstantiate them... especially since the specs are saved.

	for (auto& pTextureImage : pTextureImages)
		delete pTextureImage;
	pTextureImages.clear();
	for (auto& texspec : texspecs) {
		TextureImage* pTexture = new TextureImage(texspec, vulkan.command.vkPool(), vulkan.device, platform);
		pTextureImages.emplace_back(pTexture);
	}
}

// A more sophistocated means of reloading/regenerating TextureImages may be necessary later, should those
//	images change or animate, or otherwise require loading entirely new images (and discarding old ones).
//
vector<DescribEd> AddOns::reDescribe()
{
	vector<DescribEd> redescribedAddOns;
	if (pUniformBuffer)
		redescribedAddOns.emplace_back(pUniformBuffer->getDescriptorBufferInfo(),
									   ubo.getShaderStageFlags());
	for (auto& pTextureImage : pTextureImages)
		redescribedAddOns.emplace_back(pTextureImage->getDescriptorImageInfo(),
									   VK_SHADER_STAGE_FRAGMENT_BIT);
	return redescribedAddOns;
}
