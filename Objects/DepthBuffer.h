//
// DepthBuffer.h
//	Vulkan Objects
//
// Encapsulate Depth Buffer and its Image Resources (i.e.
//	a depth buffer incorporates an image, its memory and view).
//
// Created 10/12/22 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DepthBuffer_h
#define DepthBuffer_h

#include "ImageResource.h"
#include "Swapchain.h"


class DepthBuffer : protected ImageResource
{
public:
	DepthBuffer(Swapchain& swapchain, GraphicsDevice& graphics, bool enabled = true);

		// MEMBERS
private:
	GraphicsDevice&	graphicsDevice;

		// METHODS
private:
	void create();
	void selectBestDepthFormat();
	VkFormat findSupportedFormat(VkFormat candidates[], int nCandidates,
								 VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format);
public:
	void Recreate(Swapchain& swapchain);

		// getters
	VkImageView* getpImageView() {
		if (imageInfo.format == VK_FORMAT_UNDEFINED)
			return nullptr;
		return &imageView;
	}
};

#endif	// DepthBuffer_h
