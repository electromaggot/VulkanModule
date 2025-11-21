//
// ShadowMap.cpp
//	VulkanModule Adjunct
//
// Implementation of shadow mapping resources.
//
// Created 3 Oct 2025 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "ShadowMap.h"
#include "GraphicsDevice.h"
#include "CommandObjects.h"
#include "VulkanPlatform.h"


ShadowMap::ShadowMap(GraphicsDevice& graphicsDevice, CommandPool& commandPool,
					 uint32_t shadowMapWidth, uint32_t shadowMapHeight)
	:	device(graphicsDevice)
		, commandPool(commandPool)
		, width(shadowMapWidth)
		, height(shadowMapHeight)
		, depthFormat(VK_FORMAT_D32_SFLOAT)	// 32-bit float depth for high precision.
		, renderPass(device, depthFormat)
		, shadowImage(VK_NULL_HANDLE)
		, shadowImageMemory(VK_NULL_HANDLE)
		, shadowImageView(VK_NULL_HANDLE)
		, shadowSampler(VK_NULL_HANDLE)
		, shadowFramebuffer(VK_NULL_HANDLE)
{
	create();
}

ShadowMap::~ShadowMap()
{
	destroy();
}

void ShadowMap::create()
{
	createShadowImage();
	createShadowImageView();
	createShadowSampler();
	createShadowFramebuffer();
}

void ShadowMap::Recreate()
{
	destroy();
	create();
	renderPass.Recreate();
}

void ShadowMap::createShadowImage()
{
	VkImageCreateInfo imageInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = depthFormat,
		.extent = {
			.width = width,
			.height = height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VkDevice vkDevice = device.getLogical();
	if (vkCreateImage(vkDevice, &imageInfo, nullptr, &shadowImage) != VK_SUCCESS)
		Fatal("Failed to create shadow map image!");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(vkDevice, shadowImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = nullptr,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
										   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &shadowImageMemory) != VK_SUCCESS)
		Fatal("Failed to allocate shadow map image memory!");

	vkBindImageMemory(vkDevice, shadowImage, shadowImageMemory, 0);

	// Transition shadow map to depth attachment layout for first use.
	// Create a one-time command buffer for the layout transition:
	VkCommandBufferAllocateInfo cmdAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = commandPool.getVkCommandPool(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(vkDevice, &cmdAllocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = nullptr
	};
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Transition from UNDEFINED to DEPTH_STENCIL_READ_ONLY_OPTIMAL:
	// This matches the layout that shadow maps end in after rendering,
	// ensuring consistency across all frames (including first use).
	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask		 = 0,
		.dstAccessMask		 = VK_ACCESS_SHADER_READ_BIT,
		.oldLayout			 = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout			 = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image				 = shadowImage,
		.subresourceRange = {
			.aspectMask		= VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel	= 0,
			.levelCount		= 1,
			.baseArrayLayer	= 0,
			.layerCount		= 1
		}
	};

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	vkEndCommandBuffer(commandBuffer);

	// Submit and wait
	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = nullptr,
		.pWaitDstStageMask = nullptr,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = nullptr
	};

	vkQueueSubmit(device.Queues.getCurrent(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.Queues.getCurrent());
	vkFreeCommandBuffers(vkDevice, commandPool.getVkCommandPool(), 1, &commandBuffer);
}

void ShadowMap::createShadowImageView()
{
	VkImageViewCreateInfo viewInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags	  = 0,
		.image	  = shadowImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format	  = depthFormat,
		.components = {
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY
		},
		.subresourceRange = {
			.aspectMask		= VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel	= 0,
			.levelCount	 	= 1,
			.baseArrayLayer = 0,
			.layerCount		= 1
		}
	};
	VkDevice vkDevice = device.getLogical();
	if (vkCreateImageView(vkDevice, &viewInfo, nullptr, &shadowImageView) != VK_SUCCESS)
		Fatal("Failed to create shadow map image view!");
}

void ShadowMap::createShadowSampler()
{
	VkSamplerCreateInfo samplerInfo = {
		.sType			= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext			= nullptr,
		.flags			= 0,
		.magFilter		= VK_FILTER_LINEAR,	// Linear filtering for PCF.
		.minFilter		= VK_FILTER_LINEAR,
		.mipmapMode		= VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU	= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV	= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW	= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias		= 0.0f,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy	= 1.0f,
		.compareEnable	= VK_FALSE,			// Disable hardware PCF for manual control.
		.compareOp		= VK_COMPARE_OP_LESS,
		.minLod			= 0.0f,
		.maxLod			= 1.0f,
		.borderColor	= VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
		.unnormalizedCoordinates = VK_FALSE
	};

	VkDevice vkDevice = device.getLogical();
	if (vkCreateSampler(vkDevice, &samplerInfo, nullptr, &shadowSampler) != VK_SUCCESS)
		Fatal("Failed to create shadow map sampler!");
}

void ShadowMap::createShadowFramebuffer()
{
	VkFramebufferCreateInfo framebufferInfo = {
		.sType			 = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext			 = nullptr,
		.flags			 = 0,
		.renderPass		 = renderPass.getVkRenderPass(),
		.attachmentCount = 1,
		.pAttachments	 = &shadowImageView,
		.width			 = width,
		.height			 = height,
		.layers			 = 1
	};

	VkDevice vkDevice = device.getLogical();
	if (vkCreateFramebuffer(vkDevice, &framebufferInfo, nullptr, &shadowFramebuffer) != VK_SUCCESS)
		Fatal("Failed to create shadow framebuffer!");
}

void ShadowMap::destroy()
{
	VkDevice vkDevice = device.getLogical();

	if (shadowFramebuffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(vkDevice, shadowFramebuffer, nullptr);
		shadowFramebuffer = VK_NULL_HANDLE;
	}
	if (shadowSampler != VK_NULL_HANDLE) {
		vkDestroySampler(vkDevice, shadowSampler, nullptr);
		shadowSampler = VK_NULL_HANDLE;
	}
	if (shadowImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(vkDevice, shadowImageView, nullptr);
		shadowImageView = VK_NULL_HANDLE;
	}
	if (shadowImage != VK_NULL_HANDLE) {
		vkDestroyImage(vkDevice, shadowImage, nullptr);
		shadowImage = VK_NULL_HANDLE;
	}
	if (shadowImageMemory != VK_NULL_HANDLE) {
		vkFreeMemory(vkDevice, shadowImageMemory, nullptr);
		shadowImageMemory = VK_NULL_HANDLE;
	}
}

uint32_t ShadowMap::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device.getGPU(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	Fatal("Failed to find suitable memory type for shadow map!");
	return 0;
}
