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

DynamicUniformBuffer::DynamicUniformBuffer(uint32_t maxObjects, uint32_t framesInFlight,
										   GraphicsDevice& device)
	: BufferBase(device)
	, maxObjects(maxObjects)
	, framesInFlight(framesInFlight)
{
	// Calculate aligned size for dynamic UBO requirements
	VkPhysicalDeviceProperties properties;
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
		// Use BufferBase's helper method to create the buffer
		createGeneralBuffer(totalBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i], uniformBuffersMemory[i]);

		// Map memory for this buffer
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

void DynamicUniformBuffer::updateObjectTransform(uint32_t frameIndex, uint32_t objectIndex, const mat4& modelMatrix)
{
	if (frameIndex >= framesInFlight || objectIndex >= maxObjects) {
		return; // Invalid indices
	}

	// Calculate offset for this object
	size_t offset = objectIndex * alignedObjectSize;

	// Get pointer to this object's data
	uint8_t* bufferData = static_cast<uint8_t*>(mappedMemory[frameIndex]);
	PerObjectData* objectData = reinterpret_cast<PerObjectData*>(bufferData + offset);

	// Copy model matrix
	memcpy(objectData->model, glm::value_ptr(modelMatrix), sizeof(objectData->model));

	// For normal matrix, use model matrix for now (proper implementation would be inverse transpose)
	memcpy(objectData->normalMatrix, glm::value_ptr(modelMatrix), sizeof(objectData->normalMatrix));
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
		bufferInfo.range = alignedObjectSize; // Range per object
	}
	return bufferInfo;
}

void DynamicUniformBuffer::Recreate(uint32_t maxObjects, uint32_t framesInFlight)
{
	destroy();

	this->maxObjects = maxObjects;
	this->framesInFlight = framesInFlight;

	// Recalculate buffer size
	totalBufferSize = static_cast<VkDeviceSize>(alignedObjectSize * maxObjects);

	create();
}

uint32_t DynamicUniformBuffer::calculateAlignedSize(uint32_t size, uint32_t alignment)
{
	return (size + alignment - 1) & ~(alignment - 1);
}
