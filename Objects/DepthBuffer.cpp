//
// DepthBuffer.cpp
//	Vulkan Add-ons
//
// See header description.
//
// Created 10/12/22 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "DepthBuffer.h"


DepthBuffer::DepthBuffer(GraphicsDevice& graphics, iPlatform& platform)
	:	TextureImage(graphics, platform),
		graphicsDevice(graphics)
{
	device = graphicsDevice.getLogical();

	imageInfo.wide = platform.pixelsWide;
	imageInfo.high = platform.pixelsHigh;
	
	create();
}


DepthBuffer::~DepthBuffer()
{
	destroy();
}

void DepthBuffer::destroy()
{
	vkDestroyImageView(device, imageView, nullptr);
	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, deviceMemory, nullptr);
}


void DepthBuffer::create()
{
	selectBestDepthFormat();
	createImage(imageInfo.wide, imageInfo.high, imageInfo.format, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				image, deviceMemory);
	createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);
}

void DepthBuffer::selectBestDepthFormat() {
	VkFormat candidateFormats[] = {
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,	// S8_UINT ≡ stencil component
		VK_FORMAT_D24_UNORM_S8_UINT		//
	};
	auto format = findSupportedFormat(candidateFormats, N_ELEMENTS_IN_ARRAY(candidateFormats),
									  VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	imageInfo.format = format;
	graphicsDevice.getProfile().selectedDepthFormat = format;
}
VkFormat DepthBuffer::findSupportedFormat(VkFormat candidates[], int nCandidates,
										  VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (int iFormat = 0; iFormat < nCandidates; ++iFormat) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(graphicsDevice.getGPU(), candidates[iFormat], &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features)) {
			return candidates[iFormat];
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features)) {
			return candidates[iFormat];
		}
	}
	Log(ERROR, "DepthBuffering not possible, failed to find any supported format!");
	return VK_FORMAT_MAX_ENUM;
}
bool DepthBuffer::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


void DepthBuffer::Recreate(Swapchain& swapchain)
{
	destroy();
	VkExtent2D swapchainExtent = swapchain.getExtent();
	imageInfo.wide = swapchainExtent.width;
	imageInfo.high = swapchainExtent.height;
	create();
}
