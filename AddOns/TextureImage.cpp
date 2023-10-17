//
// TextureImage.cpp
//	Vulkan Add-ons
//
// See header description.
//
// Created 6/29/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "TextureImage.h"
#include "FileSystem.h"

#include "iPlatform.h"
#include "CommandObjects.h"
#include "RenderSettings.h"


TextureImage::TextureImage(TextureSpec& texSpec, VkCommandPool& pool, GraphicsDevice& device,
						   iPlatform& platform, VkSampler injectedSampler)
	:	BufferBase(device),
		CommandBufferBase(pool, device),
		specified(texSpec),
		mipmaps(pool, device)
{
	if (texSpec.fileName)
		create(texSpec, device, platform);
	else if (texSpec.pImageInfo)
		createBlank(*texSpec.pImageInfo, device, platform);
	else {
		Log(ERROR, "Cannot create TextureImage blank (no fileName) without ImageInfo specified size/format.");
		return;
	}
	createImageView();
	if (injectedSampler != VK_NULL_HANDLE) {
		sampler = injectedSampler;
		wasSamplerInjected = true;
	} else {
		createSampler(texSpec);
	}
}

TextureImage::TextureImage(GraphicsDevice& device, iPlatform& platform)
	:	BufferBase(device),
		CommandBufferBase(CommandControl::vkPool(), device),
		mipmaps(CommandControl::vkPool(), device)	// (even if unused must still initialize)
{
	wasSamplerInjected = true;	//TJ_TODO: temporary, staves-off crash in vkDestroySampler below.
}

TextureImage::~TextureImage()
{
	if (! wasSamplerInjected)
		vkDestroySampler(device, sampler, nullptr);
	vkDestroyImageView(device, imageView, nullptr);
	if (pStagingBuffer) {
		delete pStagingBuffer;
		pStagingBuffer = nullptr;
	}
	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, deviceMemory, nullptr);
}

void TextureImage::ReGenerateMipmaps()
{
	if (pStagingBuffer)	// i.e. wantMutable && texSpec.filterMode == MIPMAP)
	{
		mipmaps.Generate(image, imageInfo.format, imageInfo.wide, imageInfo.high);
	}
}


void TextureImage::create(TextureSpec& texSpec, GraphicsDevice& graphicsDevice, iPlatform& platform)
{
	VkImageTiling tiling = /*texSpec.wantMutable ? VK_IMAGE_TILING_LINEAR :*/ VK_IMAGE_TILING_OPTIMAL;	//TODO: uncomment when handled better:
														   //TODO: ^this^ presumed necessary, but MoltenVK error: VK_ERROR_FEATURE_NOT_PRESENT
	string fileFullPath = FileSystem::TextureFileFullPath(texSpec.fileName);

	imageInfo = platform.ImageSource().Load(fileFullPath.c_str());

	if (!graphicsDevice.IsImageFormatSupported(imageInfo.format, tiling)) {
		VkFormat BEST_FORMAT = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
		Log(WARN, "Vulkan says selected device does not support: %s", VkFormatString(imageInfo.format));
		Log(WARN, "    Converting to: %s", VkFormatString(BEST_FORMAT));
		imageInfo = platform.ImageSource().ConvertTo(BEST_FORMAT);
	}
	uint32_t width	= imageInfo.wide;
	uint32_t height	= imageInfo.high;
	VkFormat format	= imageInfo.format;

	pStagingBuffer	= new class StagingBuffer(*this);

	pStagingBuffer->CopyInImageData(texSpec);

	createImage(width, height, format, tiling,
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, deviceMemory);

	transitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED,					// from
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);					// <- to

	pStagingBuffer->CopyOutToTextureImage();

	if (texSpec.filterMode == MIPMAP)
		mipmaps.Generate(image, format, width, height);
	else
		transitionImageLayout(image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	// from
							  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);			// <- to

	if (! texSpec.wantMutable) {
		delete pStagingBuffer;
		pStagingBuffer = nullptr;
	}
}

void TextureImage::createBlank(ImageInfo& params, GraphicsDevice& graphicsDevice, iPlatform& platform, bool mipmap)
{
	imageInfo = params;

	pStagingBuffer	= new class StagingBuffer(*this);

	pStagingBuffer->CreateAndMapBuffer(params.numBytes);

	pStagingBuffer->Clear();

	createImage(params.wide, params.high, params.format, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, deviceMemory);

	transitionImageLayout(image, params.format, VK_IMAGE_LAYOUT_UNDEFINED,					// from
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);							// <- to

	pStagingBuffer->CopyOutToTextureImage();

	if (mipmap)
		mipmaps.Generate(image, params.format, params.wide, params.high);
	else
		transitionImageLayout(image, params.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	// from
							  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);					// <- to
}

// VkCreate a texture-specific ImageView, which is how the image data is accessed.
// (some code here is identical to Swapchain::createImageViews, but not much, plus it's
//	specialized and isolated in that class... so actually more minimal to repeat/separate)
//
void TextureImage::createImageView(VkImageAspectFlags aspectFlags/* = VK_IMAGE_ASPECT_COLOR_BIT*/)
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
			.levelCount		= mipmaps.NumLevels(),		// (note: will be 1 if mipmaps not used)
			.baseArrayLayer = 0,
			.layerCount		= 1
		}
	};

	call = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
	if (call != VK_SUCCESS)
		Fatal("Create Image View for texture FAILURE" + ErrStr(call));
}

// Note that filterMode MIPMAP will imply minFilter_LINEAR and mipmapMode_LINEAR,
//	which already aligns with the general default of filterMode LINEAR.
//		(it makes sense that minFilter applies to mipmapping, because mipmapping IS minifying)
// filterMode NEAREST is a special case for either performance testing or an instance where
//	render quality is unimportant.  When set, it is applied to mipmaps, but those would have
//	to be enabled elsewhere, apart from the general simplification offered by TextureSpec.
//
void TextureImage::createSampler(TextureSpec& texSpec)
{
	VkSamplerAddressMode wrapping = texSpec.wrapMode == CLAMP ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
								  : texSpec.wrapMode == MIRROR ? VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT
								  : VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VkFilter filtering = texSpec.filterMode != NEAREST ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;	// <-- zooms to blocky pixels!
																								//	   (or shrinks to sloppy pixels!)
	VkSamplerCreateInfo samplerInfo = {
		.sType	= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.magFilter	 = filtering,
		.minFilter	 = filtering,
		.mipmapMode  = texSpec.filterMode != NEAREST ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST,
		.addressModeU	= wrapping,
		.addressModeV	= wrapping,
		.addressModeW	= wrapping,
		.mipLodBias	 = 0,
		.anisotropyEnable	= RenderSettings.useAnisotropy ? (VkBool32) VK_TRUE : VK_FALSE,
		.maxAnisotropy		= RenderSettings.useAnisotropy ? RenderSettings.anisotropyLevel : 1,
		.compareEnable	= VK_FALSE,
		.compareOp		= VK_COMPARE_OP_ALWAYS,
		.minLod		 = 0,
		.maxLod		 = 0,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE	// (i.e. 0.0 to 1.0 range vs. in pixels)
	};

	call = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
	if (call != VK_SUCCESS)
		Fatal("Create Sampler for texture FAILURE" + ErrStr(call));
}

// Note that certain formats, e.g. produce: VK_ERROR_FORMAT_NOT_SUPPORTED: VkFormat VK_FORMAT_R8G8B8_UNORM is not supported on this platform.
//
void TextureImage::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
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
		.mipLevels	 = mipmaps.CalculateNumberOfLevels(width, height),
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

// Barrier between transferring image, and upon finish, reading it.
// See https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#synchronization-access-types-supported
//
void TextureImage::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
										 								 VkImageLayout newLayout)
{
	VkCommandBuffer commands = beginSingleSubmitCommands();

	VkImageMemoryBarrier barrier = {
		.sType	= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext	= nullptr,
		//.srcAccessMask	// see logic
		//.dstAccessMask	//	 below	  and (*)
		.oldLayout	= oldLayout,
		.newLayout	= newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image	= image,
		.subresourceRange = {
			.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel	= 0,
			.levelCount		= mipmaps.NumLevels(),
			.baseArrayLayer	= 0,
			.layerCount		= 1
		}
	};

	VkPipelineStageFlags sourceStage	  = 0;
	VkPipelineStageFlags destinationStage = 0;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED		// undefined --> transfer destination
	 && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL		// transfer destination --> shader reading
		  && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage		 = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}										//^^^^^^^^^^^^^
	else
		Fatal("Unsupported Layout Transition, oldLayout " + to_string(oldLayout) + " newLayout " + to_string(newLayout));

	vkCmdPipelineBarrier(	commands,
							sourceStage, destinationStage,
							0,	 // vs. VK_DEPENDENCY_BY_REGION_BIT
							0, nullptr,		// memory barriers
							0, nullptr,		// buffer memory barriers
							1, &barrier		// image memory barriers
						);

	endAndSubmitCommands(commands);
}

// Use CommandBuffer/GPU to copy StagingBuffer to VkImage.  Be mindful on Mac of your VK_FORMAT should the following message result:
//[***MoltenVK ERROR***] VK_ERROR_FORMAT_NOT_SUPPORTED: vkCmdCopyBufferToImage(): The image is using Metal format MTLPixelFormatInvalid as a
//	substitute for Vulkan format VK_FORMAT_UNDEFINED. Since the pixel size is different, content for the image cannot be copied to or from a buffer.
//
void TextureImage::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commands = beginSingleSubmitCommands();

	VkBufferImageCopy copyRegion = {
		.bufferOffset		= 0,
		.bufferRowLength	= 0,
		.bufferImageHeight	= 0,
		.imageSubresource = {
			.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel		= 0,	// only applies to the 1:1 mipmap (user: re-generate for the other levels)
			.baseArrayLayer	= 0,
			.layerCount		= 1
		},
		.imageOffset = { 0, 0, 0 },
		.imageExtent = { width, height, 1 }
	};
	const uint32_t	numRegions = 1;

	vkCmdCopyBufferToImage(commands, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						   numRegions, &copyRegion);

	endAndSubmitCommands(commands);
}

//\\//\\  STAGING BUFFER nested class   //\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\

TextureImage::StagingBuffer::StagingBuffer(TextureImage& image)
	:	texture(image)
{ }

#define DEREFERENCE(textureImage)						\
	ImageInfo&	  imageData = textureImage.imageInfo;	\
	VkDeviceSize& nBytes	= imageData.numBytes;		\
	int&		  width		= imageData.wide;			\
	int&		  height	= imageData.high;

void TextureImage::StagingBuffer::CreateAndMapBuffer(VkDeviceSize& nBytes)
{
	texture.createGeneralBuffer(nBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								vkBuffer, deviceMemory);
	vkMapMemory(texture.device, deviceMemory, 0, nBytes, 0, (void**) &pBytesStaged);
}

// Copy into the StagingBuffer the just-loaded image.
void TextureImage::StagingBuffer::CopyInImageData(TextureSpec& spec)
{
	DEREFERENCE(texture)
	char* pPixels = (char*) imageData.pPixels;

	CreateAndMapBuffer(nBytes);

	if (! spec.flipVertical)
		memcpy(pBytesStaged, pPixels, static_cast<size_t>(nBytes));
	else {
		size_t bytesPerRow = nBytes / width;	// (imageSize was calculated using pitch)
		char* pSource = pPixels;
		char* pDestination = pBytesStaged + nBytes;
		for (uint32_t iRow = height; iRow > 0; --iRow) {
			pDestination -= bytesPerRow;
			memcpy(pDestination, pSource, bytesPerRow);
			pSource += bytesPerRow;
		}
	}
	if (spec.wantMutable)
		keepMemoryMapped = true;
	else
		vkUnmapMemory(texture.device, deviceMemory);
}

TextureImage::StagingBuffer::~StagingBuffer()
{
	if (keepMemoryMapped)
		vkUnmapMemory(texture.device, deviceMemory);
	vkDestroyBuffer(texture.device, vkBuffer, nullptr);
	vkFreeMemory(texture.device, deviceMemory, nullptr);
}


void TextureImage::StagingBuffer::CopyOutToTextureImage()
{
	ImageInfo& img = texture.imageInfo;
	texture.copyBufferToImage(vkBuffer, texture.image, img.wide, img.high);
}

// For test/debug: diagonal line of black pixels across image.
//	Note that for mipmaps, this may only write to the max-resolution map (and "blend
//	away" into the next-lower map shown) unless you re-generate the mipmaps post-mutate.
void TextureImage::StagingBuffer::experimentalWrite() {
	DEREFERENCE(texture)
	size_t bytesPerRow = nBytes / height;
	size_t bytesPerPixel = nBytes / (width * height);
	char* pDestination = pBytesStaged;
	char* pEnd = pDestination + nBytes;
	while (pDestination < pEnd) {
		memset(pDestination, 0, bytesPerPixel);
		pDestination += bytesPerRow + bytesPerPixel;
	}
	CopyOutToTextureImage();	// (i.e. change won't show up unless copied OUT of staging buffer!)
}


/* DEV NOTE (*)

At start, no wait necessary, thus empty (0) AccessMask and _TOP_OF_PIPE_BIT pipeline stage.

Subsequent VK_PIPELINE_STAGE_TRANSFER_BIT is a "pseudo" pipeline stage.  See:
https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPipelineStageFlagBits.html


TJ_TODO: setupCommandBuffer, flushSetupCommands

*/
