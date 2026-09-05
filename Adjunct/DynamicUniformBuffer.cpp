//
// DynamicUniformBuffer.cpp
//	VulkanModule AddOns
//
// Implementation of enhanced uniform buffer with dynamic offsets.
//	See header file for more information.
//
// Created 1-Oct-2024 by Tadd Jensen
//  © 0000 (uncopyrighted; use at will)
//
#include "DynamicUniformBuffer.h"
#include "GraphicsDevice.h"
#include "VulkanPlatform.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <stdexcept>
#include <algorithm>

DynamicUniformBuffer::DynamicUniformBuffer(uint32_t maxObjects, Swapchain& swapchain, GraphicsDevice& device)
	:	BufferBase(device)
	  , maxObjects(maxObjects)
	  , framesInFlight(static_cast<uint32_t>(swapchain.getImageViews().size()))
{
	VkPhysicalDeviceProperties properties;				// Calculate aligned size for dynamic UBO requirements.
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	uint32_t minAlignment = static_cast<uint32_t>(properties.limits.minUniformBufferOffsetAlignment);
	alignedObjectSize = calculateAlignedSize(sizeof(PerObjectData), minAlignment);

	totalBufferSize = static_cast<VkDeviceSize>(alignedObjectSize * maxObjects);

	create();
}

DynamicUniformBuffer::~DynamicUniformBuffer()
{
	destroy();
}


void DynamicUniformBuffer::create()
{
	uniformBuffers.resize(framesInFlight);
	uniformBuffersMemory.resize(framesInFlight);
	mappedMemory.resize(framesInFlight);

	for (uint32_t i = 0; i < framesInFlight; ++i) {
		createGeneralBuffer(totalBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,	// Use BufferBase's helper method
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,	//	 to create the buffer.
			uniformBuffers[i], uniformBuffersMemory[i]);
																					// Map memory for this buffer:
		call = vkMapMemory(device, uniformBuffersMemory[i], 0, totalBufferSize, 0, &mappedMemory[i]);
		if (call != VK_SUCCESS) {
			Fatal("Failed to map dynamic uniform buffer memory!" + ErrStr(call));
		}
	}
}

void DynamicUniformBuffer::destroy()
{
	for (uint32_t i = 0; i < uniformBuffers.size(); ++i) {
		if (mappedMemory[i]) {
			vkUnmapMemory(device, uniformBuffersMemory[i]);
		}
		if (uniformBuffers[i] != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, uniformBuffers[i], nullALLOC);
		}
		if (uniformBuffersMemory[i] != VK_NULL_HANDLE) {
			vkFreeMemory(device, uniformBuffersMemory[i], nullALLOC);
		}
	}
	uniformBuffers.clear();
	uniformBuffersMemory.clear();
	mappedMemory.clear();
}

void DynamicUniformBuffer::updateObjectTransform(uint32_t frameIndex, uint32_t objectIndex, const mat4& modelMatrix,
												  float opacity, float effectFlags, float effectParam, float effectParam2)
{
	if (frameIndex >= framesInFlight || objectIndex >= maxObjects)
		return;		// Invalid indices

	size_t offset = objectIndex * alignedObjectSize;						// Calculate offset for this object.

	uint8_t* bufferData = static_cast<uint8_t*>(mappedMemory[frameIndex]);	// Get pointer to this object's data.
	PerObjectData* objectData = reinterpret_cast<PerObjectData*>(bufferData + offset);
																			// Copy model matrix, opacity, and effect flags:
	memcpy(objectData->model, glm::value_ptr(modelMatrix), sizeof(objectData->model));
	objectData->opacity = opacity;
	objectData->effectFlags = effectFlags;
	objectData->effectParam = effectParam;
	objectData->effectParam2 = effectParam2;
}

uint32_t DynamicUniformBuffer::getDynamicOffset(uint32_t objectIndex) const
{
	return objectIndex * alignedObjectSize;
}

VkDescriptorBufferInfo DynamicUniformBuffer::getDescriptorBufferInfo(uint32_t frameIndex) const
{
	VkDescriptorBufferInfo bufferInfo{};
	if (frameIndex < uniformBuffers.size()) {
		bufferInfo.buffer = uniformBuffers[frameIndex];
		bufferInfo.offset = 0;
		bufferInfo.range = alignedObjectSize;	// Range per object
	}
	return bufferInfo;
}

void DynamicUniformBuffer::Recreate(uint32_t maxObjects, Swapchain& swapchain)
{
	destroy();

	this->maxObjects = maxObjects;
	this->framesInFlight = static_cast<uint32_t>(swapchain.getImageViews().size());

	totalBufferSize = static_cast<VkDeviceSize>(alignedObjectSize * maxObjects);	// Recalculate buffer size.

	create();
}

uint32_t DynamicUniformBuffer::calculateAlignedSize(uint32_t size, uint32_t alignment)
{
	return (size + alignment - 1) & ~(alignment - 1);
}
