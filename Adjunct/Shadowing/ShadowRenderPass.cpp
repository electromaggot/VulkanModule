//
// ShadowRenderPass.cpp
//	Vulkan Setup
//
// See matched header file for definitive main comment.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "ShadowRenderPass.h"


ShadowRenderPass::ShadowRenderPass(GraphicsDevice& graphicsDevice, VkFormat depthFormat)
	:	device(graphicsDevice.getLogical())
		, depthFormat(depthFormat)
{
	create();
}

ShadowRenderPass::~ShadowRenderPass()
{
	destroy();
}

void ShadowRenderPass::Recreate()
{
	destroy();
	create();
}

void ShadowRenderPass::destroy()
{
	vkDestroyRenderPass(device, renderPass, nullALLOC);
	renderPass = VK_NULL_HANDLE;
}

void ShadowRenderPass::create()
{
	VkAttachmentDescription depthAttachment = {
		.flags			= 0,
		.format			= depthFormat,
		.samples		= VK_SAMPLE_COUNT_1_BIT,
		.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp		= VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout	= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,	// Image already transitioned.
		.finalLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL			// For sampling in main pass.
	};

	VkAttachmentReference depthAttachmentRef = {
		.attachment	= 0,
		.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass = {
		.flags					 = 0,
		.pipelineBindPoint		 = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount	 = 0,
		.pInputAttachments		 = nullptr,
		.colorAttachmentCount	 = 0,
		.pColorAttachments		 = nullptr,
		.pResolveAttachments	 = nullptr,
		.pDepthStencilAttachment = &depthAttachmentRef,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments	 = nullptr
	};

	VkSubpassDependency dependency = {
		.srcSubpass		 = VK_SUBPASS_EXTERNAL,
		.dstSubpass		 = 0,
		.srcStageMask	 = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		.dstStageMask	 = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		.srcAccessMask	 = 0,
		.dstAccessMask	 = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.dependencyFlags = 0
	};

	VkRenderPassCreateInfo renderPassInfo = {
		.sType			 = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext			 = nullptr,
		.flags			 = 0,
		.attachmentCount = 1,
		.pAttachments	 = &depthAttachment,
		.subpassCount	 = 1,
		.pSubpasses		 = &subpass,
		.dependencyCount = 1,
		.pDependencies	 = &dependency
	};

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		Fatal("Failed to create shadow render pass!");
}
