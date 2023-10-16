//
// Framebuffers.cpp
//	Vulkan Setup
//
// See matched header file for definitive main comment.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "Framebuffers.h"
#include "VulkanSingleton.h"


Framebuffers::Framebuffers(Swapchain& swapchain, DepthBuffer& depthBuffer,
						   RenderPass& renderPass, GraphicsDevice& graphics)
	:	device(graphics.getLogical())
{
	create(swapchain.getImageViews(), swapchain.getExtent(),
		   renderPass.getVkRenderPass(), depthBuffer.getpImageView());
}

Framebuffers::~Framebuffers()  { destroy(); }

void Framebuffers::destroy()
{
	for (auto& framebuffer : framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullALLOC);
}


void Framebuffers::create(vector<VkImageView>& swapchainImageViews, VkExtent2D& extent,
						  VkRenderPass renderPass, VkImageView* pDepthImageView = nullptr)
{
	size_t nImageViews = swapchainImageViews.size();

	framebuffers.resize(nImageViews);

	bool useDepthBuffer = (pDepthImageView != nullptr);

	uint32_t nAttachmentsPerBuffer = 1;
	if (useDepthBuffer) nAttachmentsPerBuffer += 1;

	for (size_t iBufferView = 0; iBufferView < nImageViews; ++iBufferView)
	{
		VkImageView attachments[nAttachmentsPerBuffer];
		attachments[0] = swapchainImageViews[iBufferView];
		if (useDepthBuffer)
			attachments[1] = *pDepthImageView;

		VkFramebufferCreateInfo framebufferInfo = {
			.sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext	= nullptr,
			.flags	= 0,
			.renderPass		 = renderPass,
			.attachmentCount = nAttachmentsPerBuffer,
			.pAttachments	 = attachments,
			.width			 = extent.width,
			.height			 = extent.height,
			.layers			 = AppConstants.SupportStereo3D ? (uint32_t) 2 : 1
		};

		call = vkCreateFramebuffer(device, &framebufferInfo, nullALLOC, &framebuffers[iBufferView]);

		if (call != VK_SUCCESS)
			Fatal("Create Framebuffer FAILURE" + ErrStr(call));
	}
}

void Framebuffers::Recreate(Swapchain& swapchain, DepthBuffer& depthBuffer, RenderPass& renderPass)
{
	destroy();
	create(swapchain.getImageViews(), swapchain.getExtent(),
		   renderPass.getVkRenderPass(), depthBuffer.getpImageView());
}
