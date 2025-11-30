//
// ResourceTracker.h
//	VulkanModule, Assist/ResourceTracker
//
// Macro interception layer for automatic Vulkan resource tracking.
// Redirects all vkCreate*/vkDestroy* calls to tracking wrappers in debug builds.
//
// In debug builds (#ifndef NDEBUG), this header:
//  1. Declares tracking wrapper functions (trk*) for all Vulkan create/destroy calls
//  2. #defines vkCreate* → trkCreate* macros to intercept all Vulkan calls
//  3. Wrappers call real Vulkan functions and track success/failure automatically
//
// In release builds (NDEBUG defined), macros are not defined - zero overhead.
//
// Include this header AFTER <vulkan/vulkan.h> in commonly-used VulkanModule files.
//
// Created 29 Nov 2024 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ResourceTracker_h
#define ResourceTracker_h

#include <vulkan/vulkan.h>
#include "ResourceTrackerImpl.h"

// Only enable tracking in debug builds
#ifndef NDEBUG

// Forward declarations for tracking wrapper functions
// These wrap real Vulkan API calls and track resource lifecycle

// Instance/Device
VkResult trkCreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                           const VkAllocationCallbacks* pAllocator,
                           VkInstance* pInstance);
void trkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator);

VkResult trkCreateDevice(VkPhysicalDevice physicalDevice,
                        const VkDeviceCreateInfo* pCreateInfo,
                        const VkAllocationCallbacks* pAllocator,
                        VkDevice* pDevice);
void trkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);

// Surface
void trkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                         const VkAllocationCallbacks* pAllocator);

// Swapchain
VkResult trkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
void trkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                           const VkAllocationCallbacks* pAllocator);

// Image Views
VkResult trkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo,
                            const VkAllocationCallbacks* pAllocator, VkImageView* pView);
void trkDestroyImageView(VkDevice device, VkImageView imageView,
                        const VkAllocationCallbacks* pAllocator);

// Images
VkResult trkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                       const VkAllocationCallbacks* pAllocator, VkImage* pImage);
void trkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);

// Buffers
VkResult trkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                        const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);
void trkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);

// Memory
VkResult trkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                          const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
void trkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);

// Render Passes
VkResult trkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                             const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
void trkDestroyRenderPass(VkDevice device, VkRenderPass renderPass,
                         const VkAllocationCallbacks* pAllocator);

// Framebuffers
VkResult trkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                              const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
void trkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer,
                          const VkAllocationCallbacks* pAllocator);

// Graphics Pipelines
VkResult trkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache,
                                   uint32_t createInfoCount,
                                   const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                   const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
void trkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);

// Pipeline Layouts
VkResult trkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator,
                                 VkPipelineLayout* pPipelineLayout);
void trkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout,
                              const VkAllocationCallbacks* pAllocator);

// Shader Modules
VkResult trkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                               const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
void trkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule,
                           const VkAllocationCallbacks* pAllocator);

// Descriptor Set Layouts
VkResult trkCreateDescriptorSetLayout(VkDevice device,
                                      const VkDescriptorSetLayoutCreateInfo* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDescriptorSetLayout* pSetLayout);
void trkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
                                   const VkAllocationCallbacks* pAllocator);

// Descriptor Pools
VkResult trkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo,
                                 const VkAllocationCallbacks* pAllocator,
                                 VkDescriptorPool* pDescriptorPool);
void trkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                              const VkAllocationCallbacks* pAllocator);

// Descriptor Sets (allocation)
VkResult trkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                   VkDescriptorSet* pDescriptorSets);

// Semaphores
VkResult trkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo,
                           const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);
void trkDestroySemaphore(VkDevice device, VkSemaphore semaphore,
                        const VkAllocationCallbacks* pAllocator);

// Fences
VkResult trkCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo,
                       const VkAllocationCallbacks* pAllocator, VkFence* pFence);
void trkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);

// Command Pools
VkResult trkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo,
                              const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
void trkDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                          const VkAllocationCallbacks* pAllocator);

// Command Buffers (allocation)
VkResult trkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo,
                                   VkCommandBuffer* pCommandBuffers);
void trkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                          const VkCommandBuffer* pCommandBuffers);

// Samplers
VkResult trkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo,
                         const VkAllocationCallbacks* pAllocator, VkSampler* pSampler);
void trkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);

// Events
VkResult trkCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo,
                       const VkAllocationCallbacks* pAllocator, VkEvent* pEvent);
void trkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);


// MACRO INTERCEPTION: Redirect all Vulkan calls to tracking wrappers
// These macros replace vkCreate*/vkDestroy* function names throughout VulkanModule

#define vkCreateInstance              trkCreateInstance
#define vkDestroyInstance             trkDestroyInstance
#define vkCreateDevice                trkCreateDevice
#define vkDestroyDevice               trkDestroyDevice
#define vkDestroySurfaceKHR           trkDestroySurfaceKHR
#define vkCreateSwapchainKHR          trkCreateSwapchainKHR
#define vkDestroySwapchainKHR         trkDestroySwapchainKHR
#define vkCreateImageView             trkCreateImageView
#define vkDestroyImageView            trkDestroyImageView
#define vkCreateImage                 trkCreateImage
#define vkDestroyImage                trkDestroyImage
#define vkCreateBuffer                trkCreateBuffer
#define vkDestroyBuffer               trkDestroyBuffer
#define vkAllocateMemory              trkAllocateMemory
#define vkFreeMemory                  trkFreeMemory
#define vkCreateRenderPass            trkCreateRenderPass
#define vkDestroyRenderPass           trkDestroyRenderPass
#define vkCreateFramebuffer           trkCreateFramebuffer
#define vkDestroyFramebuffer          trkDestroyFramebuffer
#define vkCreateGraphicsPipelines     trkCreateGraphicsPipelines
#define vkDestroyPipeline             trkDestroyPipeline
#define vkCreatePipelineLayout        trkCreatePipelineLayout
#define vkDestroyPipelineLayout       trkDestroyPipelineLayout
#define vkCreateShaderModule          trkCreateShaderModule
#define vkDestroyShaderModule         trkDestroyShaderModule
#define vkCreateDescriptorSetLayout   trkCreateDescriptorSetLayout
#define vkDestroyDescriptorSetLayout  trkDestroyDescriptorSetLayout
#define vkCreateDescriptorPool        trkCreateDescriptorPool
#define vkDestroyDescriptorPool       trkDestroyDescriptorPool
#define vkAllocateDescriptorSets      trkAllocateDescriptorSets
#define vkCreateSemaphore             trkCreateSemaphore
#define vkDestroySemaphore            trkDestroySemaphore
#define vkCreateFence                 trkCreateFence
#define vkDestroyFence                trkDestroyFence
#define vkCreateCommandPool           trkCreateCommandPool
#define vkDestroyCommandPool          trkDestroyCommandPool
#define vkAllocateCommandBuffers      trkAllocateCommandBuffers
#define vkFreeCommandBuffers          trkFreeCommandBuffers
#define vkCreateSampler               trkCreateSampler
#define vkDestroySampler              trkDestroySampler
#define vkCreateEvent                 trkCreateEvent
#define vkDestroyEvent                trkDestroyEvent

#endif // NDEBUG

#endif // ResourceTracker_h
