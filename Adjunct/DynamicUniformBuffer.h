//
// DynamicUniformBuffer.h
//	VulkanModule AddOns
//
// Enhanced uniform buffer that supports multiple objects with dynamic offsets.
// This provides better performance than individual uniform buffers per object
//	by packing multiple object transforms into a single buffer.
//
// Created 1-Oct-2024 by Tadd Jensen
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
	// Structure for per-object uniform data (std140 layout)
	struct PerObjectData {
		alignas(16) float model[16];		// Model matrix (64 bytes)
		alignas(4)  float opacity;			// Object opacity for fade effects (default 1.0)
		alignas(4)  float effectFlags;		// Bitfield for shader effects (default 0.0, app-defined meaning)
		alignas(4)  float effectParam;		// General-purpose effect parameter (default 0.0, app-defined meaning)
		alignas(4)  float effectParam2;		// Second effect parameter (default 0.0, app-defined meaning)
	};

	DynamicUniformBuffer(uint32_t maxObjects, uint32_t framesInFlight,
						 GraphicsDevice& device);
	~DynamicUniformBuffer();

	// Update a specific object's transform, opacity, and optional effect flags/param
	void updateObjectTransform(uint32_t frameIndex, uint32_t objectIndex, const mat4& modelMatrix,
							   float opacity = 1.0f, float effectFlags = 0.0f,
							   float effectParam = 0.0f, float effectParam2 = 0.0f);

	// Get dynamic offset for a specific object
	uint32_t getDynamicOffset(uint32_t objectIndex) const;

	// Get descriptor buffer info for binding
	VkDescriptorBufferInfo getDescriptorBufferInfo(uint32_t frameIndex) const;

	// Get the size per object (aligned)
	uint32_t getAlignedObjectSize() const { return alignedObjectSize; }

	// Recreate buffers (e.g., on window resize)
	void Recreate(uint32_t maxObjects, uint32_t framesInFlight);

	void destroy();		// Public for device-loss teardown (old device); idempotent (clears vectors).

private:
	void create();
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
