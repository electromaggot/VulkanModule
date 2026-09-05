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
#include "DynamicUniformBuffer.h"

#include "PrimitiveBuffer.h"
#include "Customizer.h"


AddOns::AddOns(DrawableSpecifier& drawable, VulkanSetup& setup, iPlatform& abstractPlatform)
	:	vulkan(setup),
		platform(abstractPlatform)
{
	createVertexAndOrIndexBuffers(drawable.mesh, drawable.customize);
	createDescribedItems(drawable.pUBOs, drawable.textures, drawable.runtimeTextures,
						 drawable.perFrameRuntimeTextures, abstractPlatform);
}

AddOns::~AddOns()
{
	destroyVertexAndOrIndexBuffers();
	destroyDescribedItems();
}


#pragma mark - VERTEX / INDEX BUFFERS

void AddOns::createVertexAndOrIndexBuffers(MeshObject& meshObject, Customizer customize)
{
	if (meshObject.vertices) {
		VkCommandPool commandPool = vulkan.command.vkPool();

		// Check if this is dynamic geometry, e.g. continuously updated waveforms, particles, animated models...
		bool isDynamic = (customize & DYNAMIC_GEOMETRY) != 0;

		if (isDynamic) {	// Create host-visible vertex buffer: CPU-mappable, no command buffers for updates.
			pVertexBuffer = new PrimitiveBuffer(commandPool, vulkan.device);
			VkDeviceSize bufferSize = meshObject.vertexBufferSize();
			pVertexBuffer->CreateVertexBuffer(meshObject.vertices, bufferSize, true);  // hostVisible = true
		} else {	// Create standard device-local vertex buffer: best GPU performance, uses staging for updates.
			pVertexBuffer = new PrimitiveBuffer(meshObject, commandPool, vulkan.device);
		}

		if (meshObject.indices) {
			if (isDynamic) {	// Create host-visible index buffer for dynamic geometry
				pIndexBuffer = new PrimitiveBuffer(commandPool, vulkan.device);
				VkDeviceSize indexBufferSize = meshObject.indexCount *
					(meshObject.indexType == MeshDefaultIndexType ? sizeof(IndexBufferDefaultIndexType) : sizeof(uint32_t));
				pIndexBuffer->CreateIndexBuffer(meshObject.indices, indexBufferSize, meshObject.indexType, true); // ← hostVisible = true
			} else {	// Create standard device-local index buffer
				if (meshObject.indexType == MeshDefaultIndexType) {
					pIndexBuffer = new PrimitiveBuffer((IndexBufferDefaultIndexType*) meshObject.indices,
													   meshObject.indexCount,
													   commandPool, vulkan.device);
				} else {
					pIndexBuffer = new PrimitiveBuffer(meshObject.indexType, meshObject.indices, meshObject.indexCount,
													   commandPool, vulkan.device);
				}
			}
		}
	}
}

void AddOns::destroyVertexAndOrIndexBuffers()
{
	delete pVertexBuffer;
	pVertexBuffer = nullptr;

	delete pIndexBuffer;
	pIndexBuffer = nullptr;
}


void AddOns::Recreate(MeshObject& meshObject)
{
	if (meshObject.vertices) {				// (if new vertices exist to overwrite the old ones)
		destroyVertexAndOrIndexBuffers();			// <--(this also deletes index buffers regardless of
													//		if new indices exist to overwrite old ones)
		createVertexAndOrIndexBuffers(meshObject);
	}
}


#pragma mark - UBO / TEXTURE DESCRIPTORS

// Assemble a collection of Descriptors to be "added on."  Ordering is critical:
//	make sure each INDEX matches its "layout(binding = <INDEX>)" in your Shader...
//																					// e.g. :
void AddOns::createDescribedItems(vector<UBO>& UBOs, vector<TextureSpec>& textureSpecs,
								  vector<VkDescriptorImageInfo>& runtimeTextures,
								  vector<vector<VkDescriptorImageInfo>>& perFrameRuntimeTextures,
								  iPlatform& platform)
{
	// Uniform Buffer Objects first (explicitly: the MVP UBO)
	for (UBO& eachUBO : UBOs) {
		ubos.push_back(eachUBO);

		if (eachUBO.isDynamic) {
			// Dynamic UBO: No per-object UniformBuffer created, using shared DynamicUniformBuffer
			pUniformBuffers.push_back(nullptr);

			// Per-frame, exactly as for a regular UBO below.  The dynamic OFFSET picks the object; the descriptor still has
			//	to pick the FRAME, since DynamicUniformBuffer allocates a separate buffer per frame and updateObjectTransform()
			//	writes the one being drawn.  Naming frame N here reads frame N's transforms, applies UBOs specific to frame N.
			vector<VkDescriptorBufferInfo> perFrameBufferInfo;
			for (uint32_t iFrame = 0; iFrame < eachUBO.pDynamicUBO->getFramesInFlight(); ++iFrame)
				perFrameBufferInfo.push_back(eachUBO.pDynamicUBO->getDescriptorBufferInfo(iFrame));

			described.emplace_back(perFrameBufferInfo, eachUBO.getShaderStageFlags(), DYNAMIC_BUFFER);
		} else {
			// Regular UBO: create UniformBuffer as before
			UniformBuffer* pUniformBuffer = new UniformBuffer(eachUBO.byteSize, vulkan.swapchain, vulkan.device);
			pUniformBuffers.push_back(pUniformBuffer);

			// One VkDescriptorBufferInfo per swapchain image, so each frame's descriptor set names the buffer that frame's Update()
			//	actually writes.  (Hand over a single info aimed every set at buffer 0, leave the other buffers written but unread.)
			vector<VkDescriptorBufferInfo> perFrameBufferInfo;
			for (uint32_t iFrame = 0; iFrame < pUniformBuffer->NumBuffers(); ++iFrame)
				perFrameBufferInfo.push_back(pUniformBuffer->getDescriptorBufferInfo(iFrame));
																	// layout(binding = 0)	<-- appears in Vertex Shader
			described.emplace_back(perFrameBufferInfo, eachUBO.getShaderStageFlags());
		}
	}																// If there's > 1 UBO above, adjust the layout
																	//	number below, (binding = N + 1) accordingly!
	// Textures next (may be more than one)... order is important here too == binding index
	for (TextureSpec& textureSpec : textureSpecs) {
		if (textureSpec.fileName || textureSpec.pImageInfo) {
			texspecs.push_back(textureSpec);
			TextureImage* pTexture = new TextureImage(texspecs.back(), vulkan.command.vkPool(), vulkan.device, platform);
			if (pTexture) {
				pTextureImages.emplace_back(pTexture);				// layout(binding = 1) ... 2) ... 3)...	 <-- in Fragment Shader
				described.emplace_back( pTexture->getDescriptorImageInfo(),	VK_SHADER_STAGE_FRAGMENT_BIT);
			}						//										^^^^^^^^^^ TODO: ^^^^^^^^^^ We don't have a
		}							//		mechanism (YET!) allowing an image to be specified for the VERTEX STAGE, which
	}								//		could be helpful for something like offseting vertices based on a depth map.

	// Runtime textures (e.g., shadow maps) - already created, just add descriptors:
	for (VkDescriptorImageInfo& imageInfo : runtimeTextures) {
		described.emplace_back(imageInfo, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	// Per-frame runtime textures (e.g., shadow maps with frames-in-flight).
	// Each texture binding has one image per swapchain frame to prevent cross-frame races.
	for (vector<VkDescriptorImageInfo>& perFrameImageInfo : perFrameRuntimeTextures) {
		described.emplace_back(perFrameImageInfo, VK_SHADER_STAGE_FRAGMENT_BIT);
	}
}

void AddOns::destroyDescribedItems()
{
	for (auto& pUniformBuffer : pUniformBuffers) {
		if (pUniformBuffer)  // Skip nullptr entries (Dynamic UBOs)
			delete pUniformBuffer;
	}
	pUniformBuffers.clear();
	for (auto& pTextureImage : pTextureImages)
		delete pTextureImage;
	pTextureImages.clear();
}


void AddOns::RecreateDescribables()
{
	for (auto& pUniformBuffer : pUniformBuffers) {
		if (pUniformBuffer)  // Skip nullptr entries (Dynamic UBOs)
			pUniformBuffer->Recreate(-1, vulkan.swapchain);
	}

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
	for (int index = 0; index < pUniformBuffers.size(); ++index)
	{
		if (ubos[index].isDynamic) {	// Dynamic UBO: query shared buffer info, per frame as below.
			DynamicUniformBuffer* pDynamicUBO = ubos[index].pDynamicUBO;
			vector<VkDescriptorBufferInfo> perFrameDynamicInfo;
			for (uint32_t iFrame = 0; iFrame < pDynamicUBO->getFramesInFlight(); ++iFrame)
				perFrameDynamicInfo.push_back(pDynamicUBO->getDescriptorBufferInfo(iFrame));

			redescribedAddOns.emplace_back(perFrameDynamicInfo, ubos[index].getShaderStageFlags(), DYNAMIC_BUFFER);
		} else {						// Per-frame, exactly as in createDescribedItems(), otherwise resize quietly re-points
			UniformBuffer* pUniformBuffer = pUniformBuffers[index];			//	every frame's descriptor set back at buffer 0.
			vector<VkDescriptorBufferInfo> perFrameBufferInfo;
			for (uint32_t iFrame = 0; iFrame < pUniformBuffer->NumBuffers(); ++iFrame)
				perFrameBufferInfo.push_back(pUniformBuffer->getDescriptorBufferInfo(iFrame));

			redescribedAddOns.emplace_back(perFrameBufferInfo, ubos[index].getShaderStageFlags());
		}
	}
	for (auto& pTextureImage : pTextureImages)
		redescribedAddOns.emplace_back(pTextureImage->getDescriptorImageInfo(), VK_SHADER_STAGE_FRAGMENT_BIT);
	return redescribedAddOns;
}
