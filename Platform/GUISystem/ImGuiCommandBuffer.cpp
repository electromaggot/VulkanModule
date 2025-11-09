//
// ImGuiCommandBuffer.cpp
//	VulkanModule, Platform Layer, GUI System
//
// See header file for overview.
//
// Tadd Jensen 9 Nov 2023
//	© 0000 (uncopyrighted; use at will)
//
#include "ImGuiCommandBuffer.h"
#include "VulkanSetup.h"
#include "DearImGui.h"
#include "Logging.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"


ImGuiCommandBuffer::ImGuiCommandBuffer(VulkanSetup& vulkan)
	: numFrames(0)
{
	createCommandBuffers(vulkan);
}

ImGuiCommandBuffer::~ImGuiCommandBuffer()
{
	// Command buffers freed automatically when command pool is destroyed.
	// No manual cleanup needed here.
}


void ImGuiCommandBuffer::createCommandBuffers(VulkanSetup& vulkan)
{
	numFrames = vulkan.swapchain.getNumImages();
	commandBuffers.resize(numFrames);

	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = vulkan.command.vkPool(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = numFrames
	};

	VkResult result = vkAllocateCommandBuffers(vulkan.device.getLogical(), &allocInfo, commandBuffers.data());
	if (result != VK_SUCCESS)
		Fatal("Failed to allocate ImGui command buffers!");

	Log(LOW, "ImGuiCommandBuffer: Allocated %u command buffers", numFrames);
}


void ImGuiCommandBuffer::freeCommandBuffers(VkDevice device, VkCommandPool commandPool)
{
	if (! commandBuffers.empty()) {
		vkFreeCommandBuffers(device, commandPool, (uint32_t)commandBuffers.size(), commandBuffers.data());
		commandBuffers.clear();
	}
}


void ImGuiCommandBuffer::recordImGuiCommands(DearImGui& gui, uint32_t frameIndex)
{
	if (frameIndex >= commandBuffers.size()) {
		Log(ERROR, "ImGuiCommandBuffer: Invalid frame index %u (max %zu)", frameIndex, commandBuffers.size());
		return;
	}

	VkCommandBuffer& cmdBuffer = commandBuffers[frameIndex];

	VkCommandBufferBeginInfo beginInfo = {						// Begin command buffer.
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,	// Re-recorded every frame.
		.pInheritanceInfo = nullptr
	};

	VkResult result = vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	if (result != VK_SUCCESS) {
		Log(ERROR, "ImGuiCommandBuffer: Failed to begin command buffer");
		return;
	}

	gui.IssueBindAndDrawCommands(cmdBuffer, frameIndex);		// Record ImGui draw commands.
	// Note: DearImGui::IssueBindAndDrawCommands handles its own pipeline/descriptor binding.

	result = vkEndCommandBuffer(cmdBuffer);						// End command buffer.
	if (result != VK_SUCCESS)
		Log(ERROR, "ImGuiCommandBuffer: Failed to end command buffer");
}


VkCommandBuffer ImGuiCommandBuffer::getCommandBuffer(uint32_t frameIndex) const
{
	if (frameIndex >= commandBuffers.size()) {
		Log(ERROR, "ImGuiCommandBuffer: Invalid frame index %u", frameIndex);
		return VK_NULL_HANDLE;
	}
	return commandBuffers[frameIndex];
}


void ImGuiCommandBuffer::recreate(VulkanSetup& vulkan)
{
	freeCommandBuffers(vulkan.device.getLogical(), vulkan.command.vkPool());
	createCommandBuffers(vulkan);
	Log(LOW, "ImGuiCommandBuffer: Recreated command buffers");
}
