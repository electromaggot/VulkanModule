//
// FrameCapture.h
//	VulkanModule Adjunct
//
// A frame kept as it was presented: the swapchain image just rendered, copied into an image of the same format and
//	size that may be sampled, e.g. a still backdrop for a loading screen, a screenshot, or a thumbnail.  The same
//	format keeps the round trip exact (an sRGB swapchain is decoded when sampled, encoded again when drawn).
//
// The copy is made between a frame's rendering and its presentation: it waits on the semaphore its rendering signals
//	and then for the queue, so the frame's presentation waits on nothing.  It needs a swapchain whose images may be
//	copied from (Swapchain::canCapture()).  A device child: destroy it before the device goes (e.g. at sleep).
//
// 10/31/26 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef FrameCapture_h
#define FrameCapture_h

#include "ImageResource.h"
#include "Swapchain.h"


class FrameCapture : public ImageResource
{
public:
	FrameCapture(GraphicsDevice& graphicsDevice, VkCommandPool commandPool);
	~FrameCapture() = default;		// (~ImageResource() destroys the image, its memory, and its view.)

	// Copy swapchain image iImage once `rendered` signals (consuming it), before the image is presented.
	//	False if it can't be (a swapchain not copyable, its format not sampleable) and nothing is kept.
	bool Capture(Swapchain& swapchain, uint32_t iImage, VkSemaphore rendered);

	bool		 isCaptured() const	{ return captured; }
	VkImageView	 getImageView()		{ return imageView; }
	VkExtent2D	 getExtent() const	{ return { (uint32_t) imageInfo.wide, (uint32_t) imageInfo.high }; }

	static constexpr VkImageLayout LAYOUT = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;	// The image's arrangement
																						//	once captured.
private:
	GraphicsDevice&	graphicsDevice;
	VkCommandPool	commandPool;
	bool			captured = false;

	void create(VkFormat format, VkExtent2D extent);
	void recordCopy(VkCommandBuffer commands, VkImage source, VkExtent2D extent);
};

#endif // FrameCapture_h
