//
// imgui_impl_vulkan.h
//	Dummy placeholder for Dear ImGui Vulkan backend.
//
// Tadd Jensen 9 Nov 2023
//	© 0000 (uncopyrighted; use at will)
//
#ifndef imgui_impl_vulkan_h
#define imgui_impl_vulkan_h

#include <vulkan/vulkan.h>

// Vulkan backend initialization structure
struct ImGui_ImplVulkan_InitInfo {
	VkInstance          Instance;
	VkPhysicalDevice    PhysicalDevice;
	VkDevice            Device;
	uint32_t            QueueFamily;
	VkQueue             Queue;
	VkPipelineCache     PipelineCache;
	VkDescriptorPool    DescriptorPool;
	uint32_t            MinImageCount;
	uint32_t            ImageCount;
	VkSampleCountFlagBits MSAASamples;
	const VkAllocationCallbacks* Allocator;
	void                (*CheckVkResultFn)(VkResult err);
};

// Stub function declarations
bool ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo* info, VkRenderPass render_pass);
void ImGui_ImplVulkan_Shutdown();
void ImGui_ImplVulkan_NewFrame();
bool ImGui_ImplVulkan_CreateFontsTexture();
void ImGui_ImplVulkan_DestroyFontsTexture();

#endif	// imgui_impl_vulkan_h
