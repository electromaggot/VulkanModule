//
// ImageResource.h
//	Vulkan Objects
//
// Isolate specific Vulkan image handling shared between the Depth Buffer
//	(which uses an image) and added-on Textures (which of course use images).
//	Note that this reference to "images" also means ImageView and DeviceMemory.
//
// Created 10/13/22 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ImageResource_h
#define ImageResource_h

#include "VulkanPlatform.h"		// for Vk_
#include "BufferBase.h"
#include "GraphicsDevice.h"
#include "Mipmaps.h"


class ImageResource : protected BufferBase
{
public:
	ImageResource(GraphicsDevice& graphicsDevice, Mipmaps* pMipmaps = nullptr);
	~ImageResource();

		// MEMBERS
protected:
	VkImage			image;
	VkDeviceMemory	deviceMemory;
	VkImageView		imageView;

	Mipmaps*		pMipmaps;
	ImageInfo		imageInfo;

		// METHODS
protected:
	void destroy();
	void createImageView(VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
					 VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
					 VkImage& image, VkDeviceMemory& imageMemory);
		// getters
public:
	VkImage&	getImage()	{ return image;		}
	ImageInfo&	getInfo()	{ return imageInfo;	}
};


#endif	// ImageResource_h
