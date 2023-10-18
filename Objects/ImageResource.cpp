//
// ImageResource.cpp
//	Vulkan Objects
//
// See header description.
//
// Created 10/13/22 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "ImageResource.h"


ImageResource::ImageResource(GraphicsDevice& graphicsDevice, Mipmaps* optionalMipmaps)
	:	BufferBase(graphicsDevice),
		pMipmaps(optionalMipmaps)
{ }

ImageResource::~ImageResource()
{
	vkDestroyImageView(device, imageView, nullptr);
	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, deviceMemory, nullptr);
}


// VkCreate a texture-specific ImageView, which is how the image data is accessed.
// (some code here is identical to Swapchain::createImageViews, but not much, plus it's
//	specialized and isolated in that class... so actually more minimal to repeat/separate)
//
void ImageResource::createImageView(VkImageAspectFlags aspectFlags/* = VK_IMAGE_ASPECT_COLOR_BIT*/)
{
	VkImageViewCreateInfo viewInfo = {
		.sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.image		= image,
		.viewType	= VK_IMAGE_VIEW_TYPE_2D,
		.format		= imageInfo.format,
		.components = { .r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY,
						.b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY },
		.subresourceRange = {
			.aspectMask		= aspectFlags,
			.baseMipLevel	= 0,
			.levelCount		= pMipmaps ? pMipmaps->NumLevels() : 1, // (note: NumLevels will also be 1 if mipmaps not used)
			.baseArrayLayer = 0,
			.layerCount		= 1
		}
	};

	call = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
	if (call != VK_SUCCESS)
		Fatal("Create Image View for texture FAILURE" + ErrStr(call));
}

// Note that certain formats, e.g. produce: VK_ERROR_FORMAT_NOT_SUPPORTED: VkFormat VK_FORMAT_R8G8B8_UNORM is not supported on this platform.
//
void ImageResource::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
								VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
								VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo = {
		.sType	= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.imageType	 = VK_IMAGE_TYPE_2D,
		.format		 = format,
		.extent		 = { width, height, 1 /* = depth, must be 1 */ },
		.mipLevels	 = pMipmaps ? pMipmaps->CalculateNumberOfLevels(width, height) : 1,
		.arrayLayers = 1,
		.samples	 = VK_SAMPLE_COUNT_1_BIT,
		.tiling		 = tiling,
		.usage		 = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount	= 0,
		.pQueueFamilyIndices	= nullptr,
		.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED,
	};

	call = vkCreateImage(device, &imageInfo, nullptr, &image);
	if (call != VK_SUCCESS)
		Fatal("Create Image FAILURE" + ErrStr(call));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {
		.sType	= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext	= nullptr,
		.allocationSize	 = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
	};

	call = vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory);
	if (call != VK_SUCCESS)
		Fatal("Allocate Memory for image FAILURE" + ErrStr(call));

	vkBindImageMemory(device, image, imageMemory, 0);
}
