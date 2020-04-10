//
// CommandObjects.cpp
//	Vulkan Setup
//
// See matched header file for definitive main comment.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "CommandObjects.h"
#include "VulkanSingleton.h"
#include "iRenderable.h"


CommandObjects::CommandObjects(Framebuffers& framebuffers, RenderPass& renderPass,
							   Swapchain& swapchain, GraphicsDevice& graphics)
	:	commandPool(CommandPool::Singleton(graphics)),
		commandBuffers(nullptr),
		device(graphics)
{
	beginInfo = {
		.sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext	= nullptr,
		.flags	= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
		.pInheritanceInfo = nullptr
	};

	createCommandBuffers(framebuffers.getVkFramebuffers(), swapchain.getExtent(),
						 renderPass.getVkRenderPass());
}

CommandObjects::~CommandObjects()
{
	delete commandBuffers;	/// (also no need to vkFreeCommandBuffers, as vkDestroyCommandPool will)
}


#pragma mark - CommandPool

//TJ_TODO: Revisit this.  VulkanSetup.commandPool instantiates it, then that instance referenced here via
//			CommandObjects.commandPool.  Seems like "one simple instance" should be simplified & shored-up.

CommandPool::CommandPool(GraphicsDevice& graphicsDevice)
	:	device(graphicsDevice)
{
	CommandPool::pSelf = this;

	VkCommandPoolCreateInfo poolInfo = {
		.sType	= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.queueFamilyIndex  = device.Queues.GraphicsIndex()
	};

	call = vkCreateCommandPool(device.getLogical(), &poolInfo, nullALLOC, &vkInstance);

	if (call != VK_SUCCESS)
		Fatal("Create Command Pool FAILURE" + ErrStr(call));
}
CommandPool::~CommandPool()
{
	vkDestroyCommandPool(device.getLogical(), vkInstance, nullALLOC);
	pSelf = nullptr;
}
CommandPool* CommandPool::pSelf = nullptr;
CommandPool& CommandPool::Singleton(GraphicsDevice& graphics)
{
	if (pSelf) {
		if (graphics.getLogical() == pSelf->device.getLogical())
			return *pSelf;
		delete pSelf;
	}
	pSelf = new CommandPool(graphics);
	return *pSelf;
}


#pragma mark - CommandObjects IMPLEMENTATION

void CommandObjects::allocateCommandBuffers()
{
	commandBuffers = new VkCommandBuffer[nCommandBuffers];

	VkCommandBufferAllocateInfo allocInfo = {
		.sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext	= nullptr,
		.commandPool		= commandPool.vkInstance,
		.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = nCommandBuffers
	};

	call = vkAllocateCommandBuffers(device.getLogical(), &allocInfo, commandBuffers);

	if (call != VK_SUCCESS)
		Fatal("Allocate Command Buffers FAILURE" + ErrStr(call));
}

//TJ_TODO: With the Allocate, this is really a single-shot...
//			For ACTIVE CommandBuffers, the Allocate should happen once and Buffer re-used.
// Allocate the CommandBuffers and record a first set.  This can be a "single shot" for graphics
//	objects that record once and never have to re-record, or recordCommands() can be called again
//	or repeatedly and the already-allocated Buffers re-used.
//
void CommandObjects::createCommandBuffers(vector<VkFramebuffer>& framebuffers, VkExtent2D& swapchainExtent,
										  VkRenderPass& renderPass)
{
	nCommandBuffers = (uint32_t) framebuffers.size();

	allocateCommandBuffers();

	recordCommands(framebuffers, swapchainExtent, renderPass);
}

void CommandObjects::recordCommands(vector<VkFramebuffer>& framebuffers, VkExtent2D& swapchainExtent,
									VkRenderPass& renderPass)
{
	VkRenderPassBeginInfo renderPassInfo = {
		.sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext	= nullptr,
		.renderPass		 = renderPass,
		.renderArea		 = { { 0, 0 }, swapchainExtent },
		.clearValueCount = 1,
		.pClearValues	 = &VulkanSingleton::instance().ClearColor
	};

	for (int iBuffer = 0; iBuffer < nCommandBuffers; ++iBuffer)		//TJ_TODO: Still seems a little weird to repeat for every
	{																//	frame buffer.  Can't one same CommandBuffer be shared?
		VkCommandBuffer& commandBuffer = commandBuffers[iBuffer];

		call = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		if (call != VK_SUCCESS)
			Fatal("Fail to even Begin recording Command Buffer," + ErrStr(call));

		renderPassInfo.framebuffer	= framebuffers[iBuffer];

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

//		for (auto& renderable : renderables)
//			renderable.RenderAndDraw(commandBuffer);

		vkCmdEndRenderPass(commandBuffer);

		call = vkEndCommandBuffer(commandBuffer);
		if (call != VK_SUCCESS)
			Fatal("Fail to record (End) Command Buffer," + ErrStr(call));
	}
}

// If pVertexObject null, the Vertex/Index Buffers will not be reloaded, but the same ones
//	reused when CommandBuffers recreate.  Otherwise, pass a pointer to one to reload with,
//	or a VertexBaseObject with .vertices = nullptr to eliminate the Vertex Buffer altogether
//	(for instance if vertices are handled in-shader).
//
void CommandObjects::Recreate(Framebuffers& framebuffers, RenderPass& renderPass, Swapchain& swapchain, bool reloadMesh)
{
	vkFreeCommandBuffers(device.getLogical(), commandPool.vkInstance, nCommandBuffers, commandBuffers);

	delete commandBuffers;					// (note that commandPool can stay as-is)

	if (reloadMesh) {		// otherwise keep the same AddOns (e.g. VertexBuffer) we already have loaded
//		for (auto& renderable : renderables)
//			renderable.Recreate(commandBuffer);
	}

	createCommandBuffers(framebuffers.getVkFramebuffers(), swapchain.getExtent(),
						 renderPass.getVkRenderPass());
}
