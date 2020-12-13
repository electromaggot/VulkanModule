//
// TextureImage.h
//	Vulkan Add-ons
//
// Encapsulate Texture Image Mapping, which for convenience bundles-in a Sampler and ImageView,
//	as well as supporting methods to: Copy image data, Transition its Format/Layout, etc.
//	This relies on the separate Descriptors (pool, sets, layout) class too.
// Also define a data structure to specify the texture's file name and how it
//	is intended to be sampled, wrapped, or if it should be flipped upside-down.
//
// For the Sampler, which dictates the appearance of this image, there are two options:
//	- Let this class create one automatically based on the TextureSpec parameters, or,
//	- Pass-in a pre-existing Sampler (thus ignoring those related TextureSpec values).
//
// Created 6/29/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef TextureImage_h
#define TextureImage_h

#include "CommandBufferBase.h"
#include "Mipmaps.h"


enum FilterMode {
	LINEAR,
	NEAREST,
	MIPMAP
};

enum WrapMode {
	REPEAT,
	MIRROR,
	CLAMP
};

struct TextureSpec
{
	StrPtr		fileName	 = nullptr;
	FilterMode	filterMode	 = LINEAR;
	WrapMode	wrapMode	 = REPEAT;
	bool		flipVertical = false;	// Set only for pre-flipped image coming from OpenGL. Vulkan orients texture Y-origin correctly.
	bool		wantMutable	 = false;	// User will modify texture later (so retain staging buffer and re-copyBufferToImage "dirtied" subregions).
};


class TextureImage : CommandBufferBase
{
public:
	TextureImage(TextureSpec& texSpec, VkCommandPool& pool, GraphicsDevice& graphicsDevice,
				 iPlatform& platform, VkSampler sampler = VK_NULL_HANDLE);
	~TextureImage();

		// MEMBERS
private:
	VkImage			image;
	VkDeviceMemory	deviceMemory;
	VkImageView		imageView;
	VkSampler		sampler;

	Mipmaps			mipmaps;

	ImageInfo		imageInfo;

	bool			wasSamplerInjected = false;

		// METHODS
private:
	void create(TextureSpec& texSpec, GraphicsDevice& graphicsDevice, iPlatform& platform);
	void destroy();
	void createImageView();
	void createSampler(TextureSpec& texSpec);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
					 VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
					 VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
							   VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
public:
	//void Recreate();
	void ReGenerateMipmaps();

		// getters
	VkDescriptorImageInfo getDescriptorImageInfo() {
		return {
			.sampler	 = sampler,
			.imageView	 = imageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};
	}
	VkImage&		getImage()		{ return image;		}
	ImageInfo&		getInfo()		{ return imageInfo;	}


private:
	class StagingBuffer
	{
	public:
		StagingBuffer(TextureImage& texture);
		~StagingBuffer();

	private:
			// MEMBERS
		VkBuffer		vkBuffer;
		VkDeviceMemory	deviceMemory;

		TextureImage&	texture;

		char*	pBytes;

		bool	keepMemoryMapped = false;

	public:
			// METHODS
		void 	CopyInImageData(TextureSpec& spec);
		void	CopyOutToParentImage();
		void	ExperimentalWrite();

	}* pStagingBuffer;
public:
	StagingBuffer&	StagingBuffer() {
		if (! pStagingBuffer)
			Fatal("Accessing StagingBuffer requires TextureSpec.wantMutable TRUE.");
		return *pStagingBuffer;
	}
};

#endif // TextureImage_h
