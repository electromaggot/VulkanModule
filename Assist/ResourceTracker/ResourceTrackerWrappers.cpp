//
// ResourceTrackerWrappers.cpp
//	VulkanModule, Assist/ResourceTracker
//
// Implementation of all Vulkan API tracking wrappers.
// Each wrapper calls the real Vulkan function and tracks resource lifecycle.
//
// Pattern:
//  - trkCreateX: Calls vkCreateX, tracks on VK_SUCCESS
//  - trkDestroyX: Calls vkDestroyX, always tracks destruction
//
// Wrappers are only compiled in debug builds (#ifndef NDEBUG).
// In release builds, macros are not defined so these are never called.
//
// Created 29 Nov 2024 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "ResourceTracker.h"
#include <vulkan/vulkan.h>

#ifndef NDEBUG

// Undefine all macros to access real Vulkan functions
// (macros were defined in ResourceTracker.h, need to access original functions here)
#undef vkCreateInstance
#undef vkDestroyInstance
#undef vkCreateDevice
#undef vkDestroyDevice
#undef vkDestroySurfaceKHR
#undef vkCreateSwapchainKHR
#undef vkDestroySwapchainKHR
#undef vkCreateImageView
#undef vkDestroyImageView
#undef vkCreateImage
#undef vkDestroyImage
#undef vkCreateBuffer
#undef vkDestroyBuffer
#undef vkAllocateMemory
#undef vkFreeMemory
#undef vkCreateRenderPass
#undef vkDestroyRenderPass
#undef vkCreateFramebuffer
#undef vkDestroyFramebuffer
#undef vkCreateGraphicsPipelines
#undef vkDestroyPipeline
#undef vkCreatePipelineLayout
#undef vkDestroyPipelineLayout
#undef vkCreateShaderModule
#undef vkDestroyShaderModule
#undef vkCreateDescriptorSetLayout
#undef vkDestroyDescriptorSetLayout
#undef vkCreateDescriptorPool
#undef vkDestroyDescriptorPool
#undef vkAllocateDescriptorSets
#undef vkCreateSemaphore
#undef vkDestroySemaphore
#undef vkCreateFence
#undef vkDestroyFence
#undef vkCreateCommandPool
#undef vkDestroyCommandPool
#undef vkAllocateCommandBuffers
#undef vkFreeCommandBuffers
#undef vkCreateSampler
#undef vkDestroySampler
#undef vkCreateEvent
#undef vkDestroyEvent


//
// Instance/Device
//

VkResult trkCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                           const VkAllocationCallbacks* pAllocator,
                           VkInstance* pInstance)
{
	VkResult result = vkCreateInstance(pCreateInfo, pAllocator, pInstance);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_INSTANCE);
	return result;
}

void trkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
	vkDestroyInstance(instance, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_INSTANCE);
}

VkResult trkCreateDevice(VkPhysicalDevice physicalDevice,
                        const VkDeviceCreateInfo* pCreateInfo,
                        const VkAllocationCallbacks* pAllocator,
                        VkDevice* pDevice)
{
	VkResult result = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_DEVICE);
	return result;
}

void trkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
	vkDestroyDevice(device, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_DEVICE);
}


//
// Surface
//

void trkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                         const VkAllocationCallbacks* pAllocator)
{
	vkDestroySurfaceKHR(instance, surface, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_SURFACE);
}


//
// Swapchain
//

VkResult trkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
{
	VkResult result = vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_SWAPCHAIN);
	return result;
}

void trkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                           const VkAllocationCallbacks* pAllocator)
{
	vkDestroySwapchainKHR(device, swapchain, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_SWAPCHAIN);
}


//
// Image Views
//

VkResult trkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                            const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
	VkResult result = vkCreateImageView(device, pCreateInfo, pAllocator, pView);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_IMAGE_VIEW);
	return result;
}

void trkDestroyImageView(VkDevice device, VkImageView imageView,
                        const VkAllocationCallbacks* pAllocator)
{
	vkDestroyImageView(device, imageView, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_IMAGE_VIEW);
}


//
// Images
//

VkResult trkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                       const VkAllocationCallbacks* pAllocator, VkImage* pImage)
{
	VkResult result = vkCreateImage(device, pCreateInfo, pAllocator, pImage);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_IMAGE);
	return result;
}

void trkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
{
	vkDestroyImage(device, image, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_IMAGE);
}


//
// Buffers
//

VkResult trkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                        const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
	VkResult result = vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_BUFFER);
	return result;
}

void trkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
	vkDestroyBuffer(device, buffer, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_BUFFER);
}


//
// Memory
//

VkResult trkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                          const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
	VkResult result = vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_DEVICE_MEMORY);
	return result;
}

void trkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
	vkFreeMemory(device, memory, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_DEVICE_MEMORY);
}


//
// Render Passes
//

VkResult trkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                             const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
	VkResult result = vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_RENDER_PASS);
	return result;
}

void trkDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                         const VkAllocationCallbacks* pAllocator)
{
	vkDestroyRenderPass(device, renderPass, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_RENDER_PASS);
}


//
// Framebuffers
//

VkResult trkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                              const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
	VkResult result = vkCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_FRAMEBUFFER);
	return result;
}

void trkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                          const VkAllocationCallbacks* pAllocator)
{
	vkDestroyFramebuffer(device, framebuffer, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_FRAMEBUFFER);
}


//
// Graphics Pipelines
//

VkResult trkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                   uint32_t createInfoCount,
                                   const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
	VkResult result = vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount,
	                                             pCreateInfos, pAllocator, pPipelines);
	// Track each successfully created pipeline
	if (result == VK_SUCCESS) {
		for (uint32_t i = 0; i < createInfoCount; i++)
			VK_TRACK_CREATE(VK_RESOURCE_PIPELINE);
	}
	return result;
}

void trkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
{
	vkDestroyPipeline(device, pipeline, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_PIPELINE);
}


//
// Pipeline Layouts
//

VkResult trkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator,
                                 VkPipelineLayout* pPipelineLayout)
{
	VkResult result = vkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_PIPELINE_LAYOUT);
	return result;
}

void trkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                              const VkAllocationCallbacks* pAllocator)
{
	vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_PIPELINE_LAYOUT);
}


//
// Shader Modules
//

VkResult trkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
	VkResult result = vkCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_SHADER_MODULE);
	return result;
}

void trkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                           const VkAllocationCallbacks* pAllocator)
{
	vkDestroyShaderModule(device, shaderModule, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_SHADER_MODULE);
}


//
// Descriptor Set Layouts
//

VkResult trkCreateDescriptorSetLayout(VkDevice device,
                                      const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDescriptorSetLayout* pSetLayout)
{
	VkResult result = vkCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_DESCRIPTOR_SET_LAYOUT);
	return result;
}

void trkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                   const VkAllocationCallbacks* pAllocator)
{
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_DESCRIPTOR_SET_LAYOUT);
}


//
// Descriptor Pools
//

VkResult trkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator,
                                 VkDescriptorPool* pDescriptorPool)
{
	VkResult result = vkCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_DESCRIPTOR_POOL);
	return result;
}

void trkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                              const VkAllocationCallbacks* pAllocator)
{
	// Get the number of descriptor sets allocated from this pool (and remove from map)
	uint32_t descriptorSetCount = ResourceTracker::instance().getDescriptorSetCountForPool(descriptorPool);

	// Destroy the pool (this implicitly frees all descriptor sets allocated from it)
	vkDestroyDescriptorPool(device, descriptorPool, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_DESCRIPTOR_POOL);

	// Track the implicit destruction of descriptor sets
	for (uint32_t i = 0; i < descriptorSetCount; i++)
		VK_TRACK_DESTROY(VK_RESOURCE_DESCRIPTOR_SET);
}


//
// Descriptor Sets (allocation)
//

VkResult trkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                   VkDescriptorSet* pDescriptorSets)
{
	VkResult result = vkAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
	// Track each successfully allocated descriptor set
	if (result == VK_SUCCESS) {
		for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++)
			VK_TRACK_CREATE(VK_RESOURCE_DESCRIPTOR_SET);
		// Also track the pool → descriptor set count mapping for implicit destruction
		ResourceTracker::instance().trackDescriptorSetAllocation(
			pAllocateInfo->descriptorPool, pAllocateInfo->descriptorSetCount);
	}
	return result;
}


//
// Semaphores
//

VkResult trkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                           const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore)
{
	VkResult result = vkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_SEMAPHORE);
	return result;
}

void trkDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                        const VkAllocationCallbacks* pAllocator)
{
	vkDestroySemaphore(device, semaphore, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_SEMAPHORE);
}


//
// Fences
//

VkResult trkCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                       const VkAllocationCallbacks* pAllocator, VkFence* pFence)
{
	VkResult result = vkCreateFence(device, pCreateInfo, pAllocator, pFence);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_FENCE);
	return result;
}

void trkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator)
{
	vkDestroyFence(device, fence, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_FENCE);
}


//
// Command Pools
//

VkResult trkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                              const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool)
{
	VkResult result = vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_COMMAND_POOL);
	return result;
}

void trkDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                          const VkAllocationCallbacks* pAllocator)
{
	vkDestroyCommandPool(device, commandPool, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_COMMAND_POOL);
}


//
// Command Buffers (allocation)
//

VkResult trkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                   VkCommandBuffer* pCommandBuffers)
{
	VkResult result = vkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
	// Track each successfully allocated command buffer
	if (result == VK_SUCCESS) {
		for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++)
			VK_TRACK_CREATE(VK_RESOURCE_COMMAND_BUFFER);
	}
	return result;
}

void trkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                          const VkCommandBuffer* pCommandBuffers)
{
	vkFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
	// Track each freed command buffer
	for (uint32_t i = 0; i < commandBufferCount; i++)
		VK_TRACK_DESTROY(VK_RESOURCE_COMMAND_BUFFER);
}


//
// Samplers
//

VkResult trkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                         const VkAllocationCallbacks* pAllocator, VkSampler* pSampler)
{
	VkResult result = vkCreateSampler(device, pCreateInfo, pAllocator, pSampler);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_SAMPLER);
	return result;
}

void trkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator)
{
	vkDestroySampler(device, sampler, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_SAMPLER);
}


//
// Events
//

VkResult trkCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                       const VkAllocationCallbacks* pAllocator, VkEvent* pEvent)
{
	VkResult result = vkCreateEvent(device, pCreateInfo, pAllocator, pEvent);
	if (result == VK_SUCCESS)
		VK_TRACK_CREATE(VK_RESOURCE_EVENT);
	return result;
}

void trkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator)
{
	vkDestroyEvent(device, event, pAllocator);
	VK_TRACK_DESTROY(VK_RESOURCE_EVENT);
}

#endif // NDEBUG
