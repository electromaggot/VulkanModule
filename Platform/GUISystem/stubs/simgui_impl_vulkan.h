//
// simgui_impl_vulkan.h (stub version)
//	No-op stub for Dear ImGui Vulkan backend when GUI disabled.
//
// Tadd Jensen 9 Nov 2023
//	© 0000 (uncopyrighted; use at will)
//
#ifndef imgui_impl_vulkan_h
#define imgui_impl_vulkan_h

#include <vulkan/vulkan.h>


// Forward declarations
struct ImDrawData;

struct ImGui_ImplVulkan_PipelineInfo {		// Pipeline info structure for newer ImGui API
	VkRenderPass    RenderPass;
	uint32_t        Subpass;
	VkSampleCountFlagBits MSAASamples;
};

struct ImGui_ImplVulkan_InitInfo {			// Vulkan backend initialization structure
	uint32_t            ApiVersion;
	VkInstance          Instance;
	VkPhysicalDevice    PhysicalDevice;
	VkDevice            Device;
	uint32_t            QueueFamily;
	VkQueue             Queue;
	VkDescriptorPool    DescriptorPool;
	uint32_t            DescriptorPoolSize;
	uint32_t            MinImageCount;
	uint32_t            ImageCount;
	VkPipelineCache     PipelineCache;
	ImGui_ImplVulkan_PipelineInfo PipelineInfoMain;
	bool                UseDynamicRendering;
	const VkAllocationCallbacks* Allocator;
	void                (*CheckVkResultFn)(VkResult err);
	size_t              MinAllocationSize;
};

struct ImGui_ImplVulkanH_Window {
	int                 Width;
	int                 Height;
	VkSwapchainKHR      Swapchain;
	VkSurfaceKHR        Surface;
	VkSurfaceFormatKHR  SurfaceFormat;
	VkPresentModeKHR    PresentMode;
	VkRenderPass        RenderPass;
	bool                UseDynamicRendering;
	bool                ClearEnable;
	VkClearValue        ClearValue;
	uint32_t            FrameIndex;
	uint32_t            ImageCount;
	uint32_t            SemaphoreCount;
	uint32_t            SemaphoreIndex;
	//ImVector<ImGui_ImplVulkanH_Frame>           Frames;
	//ImVector<ImGui_ImplVulkanH_FrameSemaphores> FrameSemaphores;
};

// Stub function declarations
bool ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo* info);
void ImGui_ImplVulkan_Shutdown();
void ImGui_ImplVulkan_NewFrame();
bool ImGui_ImplVulkan_CreateFontsTexture();
void ImGui_ImplVulkan_DestroyFontsTexture();
void ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, VkCommandBuffer command_buffer);


#endif	// imgui_impl_vulkan_h  (same name as non-stub)
