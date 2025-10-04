//
// ShadowPass.h
// VulkanModule - Shadow Mapping Pass Management
//
// Handles command buffer allocation, recording, and submission for shadow depth pass.
// Coordinates shadow pass rendering before main pass for proper shadow mapping.
//

#ifndef ShadowPass_h
#define ShadowPass_h

#include "ShadowMap.h"
#include "../../Objects/CommandObjects.h"
#include "../../Objects/GraphicsDevice.h"
#include <vector>

class VulkanSetup;
struct iRenderable;
class ShaderModules;
class GraphicsPipeline;
class DynamicUniformBuffer;
class UBO;

class ShadowPass
{
public:
	ShadowPass(VulkanSetup& vulkan, ShadowMap& shadowMap);
	~ShadowPass();

	// Record shadow pass commands for a specific frame
	// shadowRenderables: renderables to draw for shadow depth (uses existing vertex buffers)
	void recordShadowPass(std::vector<iRenderable*>& shadowRenderables, uint32_t frameIndex);

	// Set shadow depth pipeline and UBOs (must be called after initialization)
	void setShadowPipeline(ShaderModules* shadowShaders, const std::vector<UBO>& ubos);

	// Get shadow command buffer for submission
	VkCommandBuffer getCommandBuffer(uint32_t frameIndex) const {
		return shadowCommandBuffers[frameIndex];
	}

	// Recreate command buffers (e.g., on swapchain recreation)
	void recreate(uint32_t numFrames);

private:
	VulkanSetup& vulkan;
	ShadowMap& shadowMap;
	GraphicsDevice& device;
	CommandPool& commandPool;

	std::vector<VkCommandBuffer> shadowCommandBuffers;  // One per frame

	void allocateCommandBuffers(uint32_t numFrames);
	void freeCommandBuffers();
};

#endif // ShadowPass_h
