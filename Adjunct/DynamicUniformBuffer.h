//
// DynamicUniformBuffer.h
//	VulkanModule AddOns
//
// Enhanced uniform buffer that supports multiple objects with dynamic offsets.
// This provides better performance than individual uniform buffers per object
// by packing multiple object transforms into a single buffer.
//
// Created for SceneEdit integration
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DynamicUniformBuffer_h
#define DynamicUniformBuffer_h

#include "BufferBase.h"
#include "VulkanMath.h"
#include <vector>

class DynamicUniformBuffer : BufferBase
{
public:
	// Structure for per-object uniform data
	struct PerObjectData {
		alignas(16) float model[16];		// Model matrix
		alignas(16) float normalMatrix[16];	// Normal matrix (inverse transpose)
	};

	DynamicUniformBuffer(uint32_t maxObjects, uint32_t framesInFlight,
						 GraphicsDevice& device);
	~DynamicUniformBuffer();

	// Update a specific object's transform
	void updateObjectTransform(uint32_t frameIndex, uint32_t objectIndex, const mat4& modelMatrix);

	// Get dynamic offset for a specific object
	uint32_t getDynamicOffset(uint32_t objectIndex) const;

	// Get descriptor buffer info for binding
	VkDescriptorBufferInfo getDescriptorBufferInfo(uint32_t frameIndex) const;

	// Get the size per object (aligned)
	uint32_t getAlignedObjectSize() const { return alignedObjectSize; }

	// Recreate buffers (e.g., on window resize)
	void Recreate(uint32_t maxObjects, uint32_t framesInFlight);

private:
	void create();
	void destroy();
	uint32_t calculateAlignedSize(uint32_t size, uint32_t alignment);

	// Buffer management
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> mappedMemory;

	// Configuration
	uint32_t maxObjects;
	uint32_t framesInFlight;
	uint32_t alignedObjectSize;	// Size per object, aligned for dynamic UBO
	VkDeviceSize totalBufferSize;
};

#endif // DynamicUniformBuffer_h