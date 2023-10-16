//
// Framebuffers.h
//	Vulkan Setup
//
// Encapsulate multiple VkFramebuffers.
//	Generally a child object of Swapchain.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Framebuffers_h
#define Framebuffers_h

#include "GraphicsDevice.h"
#include "Swapchain.h"
#include "DepthBuffer.h"
#include "RenderPass.h"


class Framebuffers
{
public:
	VkFramebuffer& operator [] (int index) {
		return framebuffers[index];
	}

	Framebuffers(Swapchain& swapchain, DepthBuffer& depthBuffer,
				 RenderPass& renderPass, GraphicsDevice& graphics);
	~Framebuffers();

		// MEMBERS
private:
	vector<VkFramebuffer>	framebuffers;

	VkDevice&	device;		// Save so as to destroy the same way as constructed

		// METHODS
private:
	void create(vector<VkImageView>& swapchainImageViews, VkExtent2D& extent,
				VkRenderPass renderPass, VkImageView* pDepthImageView);
	void destroy();
public:
	void Recreate(Swapchain& swapchain, DepthBuffer& depthBuffer, RenderPass& renderPass);

		// getters
	vector<VkFramebuffer>& getVkFramebuffers()	{ return framebuffers; }
};

#endif // Framebuffers_h
