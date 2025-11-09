//
// ImGuiCommandBuffer.h
//	VulkanModule, Platform Layer, GUI System
//
// Dedicated command buffer manager for ImGui rendering.
// Keeps ImGui completely isolated from scene rendering to prevent state interference.
//
// Design:
//	- Own command buffer (not shared with scene renderables)
//	- Own render pass (can overlap with scene, but separate recording)
//	- Recorded every frame independently
//	- Submitted after scene rendering
//
// Tadd Jensen 9 Nov 2023
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ImGuiCommandBuffer_h
#define ImGuiCommandBuffer_h

#include "VulkanPlatform.h"

// Forward declarations
class VulkanSetup;
class DearImGui;


class ImGuiCommandBuffer
{
public:
	ImGuiCommandBuffer(VulkanSetup& vulkan);
	~ImGuiCommandBuffer();

	// Record ImGui commands into dedicated buffer for specified frame.
	void recordImGuiCommands(DearImGui& gui, uint32_t frameIndex);

	// Get command buffer for specified frame, for submission.
	VkCommandBuffer getCommandBuffer(uint32_t frameIndex) const;

	// Recreate resources (e.g. called on window resize).
	void recreate(VulkanSetup& vulkan);

private:
	void createCommandBuffers(VulkanSetup& vulkan);
	void freeCommandBuffers(VkDevice device, VkCommandPool commandPool);

	std::vector<VkCommandBuffer> commandBuffers;  // One per swapchain image.
	uint32_t numFrames;
};

#endif	// ImGuiCommandBuffer_h
