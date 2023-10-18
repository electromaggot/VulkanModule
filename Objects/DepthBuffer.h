//
// DepthBuffer.h
//	Vulkan Objects
//
// Encapsulate Depth Buffer Resources: image, its memory and view.
//
// Created 10/12/22 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DepthBuffer_h
#define DepthBuffer_h

#include "ImageResource.h"
#include "Swapchain.h"


class DepthBuffer : public ImageResource
{
public:
	DepthBuffer(GraphicsDevice& graphics, iPlatform& platform);
	~DepthBuffer();

		// MEMBERS
private:
	GraphicsDevice&	graphicsDevice;

		// METHODS
private:
	void create();
	void destroy();
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
