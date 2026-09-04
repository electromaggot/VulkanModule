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
#include "ResourceTracker.h"


ImageResource::ImageResource(GraphicsDevice& graphicsDevice, Mipmaps* optionalMipmaps)
	:	BufferBase(graphicsDevice),
		pMipmaps(optionalMipmaps)
{ }

ImageResource::~ImageResource()
{
	destroy();

	Log(DEAD, "Destroyed: ImageResource (image, imageView, deviceMemory)");
}

void ImageResource::destroy()
{
	if (imageInfo.format != VK_FORMAT_UNDEFINED) {
		if (existsImageView) {
			vkDestroyImageView(device, imageView, nullptr);
			existsImageView = false;
		}
		if (existsImage) {
			vkDestroyImage(device, image, nullptr);
			existsImage = false;
		}
		if (existsDeviceMemory) {
			vkFreeMemory(device, imageDeviceMemory, nullptr);
			existsDeviceMemory = false;
		}
	}
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
			.levelCount		= pMipmaps ? pMipmaps->NumLevels() : 1,	// (1 unless mipmapping was opted into)
			.baseArrayLayer = 0,
			.layerCount		= 1
		}
	};

	call = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
	if (call != VK_SUCCESS)
		Fatal("Create Image View for texture FAILURE" + ErrStr(call));
	existsImageView = true;
}

// Note that certain formats, e.g. produce: VK_ERROR_FORMAT_NOT_SUPPORTED: VkFormat VK_FORMAT_R8G8B8_UNORM is not supported on this platform.
//
void ImageResource::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
								VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
	VkImageCreateInfo imageInfo = {
		.sType	= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.imageType	 = VK_IMAGE_TYPE_2D,
		.format		 = format,
		.extent		 = { width, height, 1 /* = depth, must be 1 */ },
		.mipLevels	 = pMipmaps ? pMipmaps->NumLevels() : 1,	// a QUERY -- the owner opts in (and
						//	fills!) via Mipmaps::UseFullChain() before calling here; see (*) below.
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
	existsImage = true;

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {
		.sType	= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext	= nullptr,
		.allocationSize	 = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
	};

	call = vkAllocateMemory(device, &allocInfo, nullptr, &imageDeviceMemory);
	if (call != VK_SUCCESS)
		Fatal("Allocate Memory for image FAILURE" + ErrStr(call));
	existsDeviceMemory = true;

	vkBindImageMemory(device, image, imageDeviceMemory, 0);
}


/* (*) DEV NOTE - why mipLevels is only ever QUERIED here

	This line used to call Mipmaps::CalculateNumberOfLevels(width, height), which despite its name
	did not merely calculate: it ASSIGNED Mipmaps::numLevels.  Every TextureImage hands
	ImageResource a Mipmaps (only DepthBuffer passes none), so simply asking how many levels there
	were is what turned mipmapping on -- for every texture, whether or not it wanted it.  A 512x512
	image got 10 levels; a texture loaded with plain LINEAR filtering never ran Generate(), so
	levels 1..9 stayed empty, while createImageView() exposed all ten and createSampler() set
	maxLod = 9.

	Nothing complained.  It stayed invisible until a textured surface was MINIFIED, at which point
	the sampler computed a LOD above 0, read an empty level, and returned pure black -- which,
	against a black clear colour, looks like an object that simply is not rendering.  It cost a long
	debugging session in HelloVulkanSDL, whose textured quads are minified.

	The API now separates the two ideas: LevelsToFit() computes (pure, static), UseFullChain()
	opts in (the one mutator), and IsEnabled()/NumLevels() report.  Sizing the image, creating its
	view, generating the chain, and clamping the sampler's maxLod all read that single state, so
	they cannot drift apart again.
*/
