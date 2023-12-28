//
// CommandObjects.cpp
//	Vulkan Setup
//
// See header file comment for overview.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "CommandObjects.h"
#include "VulkanSingleton.h"


#pragma mark - CommandPool

CommandPool::CommandPool(GraphicsDevice& graphicsDevice)
	:	device(graphicsDevice)
{
	VkCommandPoolCreateInfo poolInfo = {
		.sType	= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex  = device.Queues.getFamilyIndex()
	};

	call = vkCreateCommandPool(device.getLogical(), &poolInfo, nullALLOC, &vkCommandPool);

	if (call != VK_SUCCESS)
		Fatal("Create Command Pool FAILURE" + ErrStr(call));
}
CommandPool::~CommandPool()
{
	vkDestroyCommandPool(device.getLogical(), vkCommandPool, nullALLOC);
}


#pragma mark - CommandBuffer

CommandBufferSet::CommandBufferSet()
	:	event(* new Event(CommandControl::device()))
{
	beginInfo = {
		.sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext	= nullptr,
		.flags	= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // flagged by Validation Performace Warning/BestPractices-vkBeginCommandBuffer-simultaneous-use
		.pInheritanceInfo = nullptr
	};
}
CommandBufferSet::~CommandBufferSet()
{
	freeVkCommandBuffers();
	delete &event;
}

void CommandBufferSet::allocateVkCommandBuffer()
{
	vkCommandBuffers.emplace_back();

	VkCommandBufferAllocateInfo allocInfo = {
		.sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext	= nullptr,
		.commandPool		= CommandControl::vkPool(),
		.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	call = vkAllocateCommandBuffers(CommandControl::device().getLogical(), &allocInfo, &vkCommandBuffers.back());

	if (call != VK_SUCCESS)
		Fatal("Allocate Command Buffers FAILURE" + ErrStr(call));
}
void CommandBufferSet::freeVkCommandBuffers()
{
	vkFreeCommandBuffers(CommandControl::device().getLogical(), CommandControl::vkPool(),
						 (uint32_t) vkCommandBuffers.size(), vkCommandBuffers.data());
	vkCommandBuffers.clear();
}

void CommandBufferSet::recordCommands(vector<iRenderable*> pBufferRenderables, VkFramebuffer& framebuffer,
									  VkExtent2D& swapchainExtent, VkRenderPass& renderPass)
{
	VkClearValue clearValues[] = {
		{ .color = { VulkanSingleton::instance().ClearColor } },
		{ .depthStencil = { 1.0f, 0 } }		// far view plane 1.0 ≡ starting depth furthest possible
	};

	VkRenderPassBeginInfo renderPassInfo = {
		.sType	= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext	= nullptr,
		.renderPass		 = renderPass,
		.framebuffer	 = framebuffer,
		.renderArea		 = { { 0, 0 }, swapchainExtent },
		.clearValueCount = N_ELEMENTS_IN_ARRAY(clearValues),
		.pClearValues	 = clearValues
	};

	size_t numBufferSets = vkCommandBuffers.size();

	for (int iBuffer = 0; iBuffer < numBufferSets; ++iBuffer)	//TJ_TODO: Re: repeat for every frame buffer, can't one same CommandBuffer be shared?
	{
		VkCommandBuffer& commandBuffer = vkCommandBuffers[iBuffer];

		call = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		if (call != VK_SUCCESS)
			Fatal("Fail to even Begin recording Command Buffer," + ErrStr(call));

//		if (iBuffer > 0)	// await prior buffer executions's completion
//			event.CmdWaitRecordTo(commandBuffer);

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		for (auto pRenderable : pBufferRenderables)
			pRenderable->IssueBindAndDrawCommands(commandBuffer, iBuffer);	// implemented by iRenderable subclass

		vkCmdEndRenderPass(commandBuffer);

//		if (iBuffer < numBufferSets - 1)	// trigger next buffer's execution to begin
//			event.CmdSetRecordTo(commandBuffer);

		call = vkEndCommandBuffer(commandBuffer);
		if (call != VK_SUCCESS)
			Fatal("Fail to record (End) Command Buffer," + ErrStr(call));
	}
}


#pragma mark - CommandControl

CommandControl::CommandControl(Framebuffers& framebuffers, GraphicsDevice& graphics)
	:	commandPool(CommandPool(graphics))
{
	pSingleton	= this;
	Create(framebuffers);
}

CommandControl*	CommandControl::pSingleton = nullptr;

CommandControl::~CommandControl()
{
	Destroy();
}


void CommandControl::Create(Framebuffers& framebuffers)
{
	numFrames	= (uint32_t) framebuffers.getVkFramebuffers().size();
	buffersByFrame = new CommandBufferSet[numFrames];
}

void CommandControl::Destroy()
{
	delete[] buffersByFrame;
	// (also no need to vkFreeCommandBuffers, as vkDestroyCommandPool will)
}


// Allocate all VkCommandBuffers and Record those for AT_INIT_TIME_ONLY Renderable set.
//
void CommandControl::PostInitPrepBuffers(VulkanSetup& vulkan)
{
	size_t numBufferSets = renderables.recordables.size();

	for (int iBufferSet = 0; iBufferSet < numBufferSets; ++iBufferSet) {
		CommandRecordable& recordable = renderables.recordables[iBufferSet];
		for (int iFrame = 0; iFrame < numFrames; ++iFrame)
		{
			buffersByFrame[iFrame].allocateVkCommandBuffer();

			if (recordable.recordMode == AT_INIT_TIME_ONLY)
				buffersByFrame[iFrame].recordCommands(recordable.pRenderables, vulkan.framebuffers[iFrame],
													  vulkan.swapchain.getExtent(), vulkan.renderPass.getVkRenderPass());
		}
	}
	assert(numBufferSets == buffersByFrame[0].numBufferSets());
}

// (Re)Record those Renderables that specified UPON_EACH_FRAME.
//
void CommandControl::RecordRenderablesForNextFrame(VulkanSetup& vulkan, int iNextFrame)
{
	size_t numBufferSets = renderables.recordables.size();

	for (int iBufferSet = 0; iBufferSet < numBufferSets; ++iBufferSet) {
		CommandRecordable& recordable = renderables.recordables[iBufferSet];
		if (recordable.recordMode == UPON_EACH_FRAME)
			buffersByFrame[iNextFrame].recordCommands(recordable.pRenderables, vulkan.framebuffers[iNextFrame],
													  vulkan.swapchain.getExtent(), vulkan.renderPass.getVkRenderPass());
	}
}


// Clear-out VkCommandBuffers; commandPool can stay as-is.
//
void CommandControl::RecreateBuffers(Framebuffers& framebuffers)
{
	Destroy();
	Create(framebuffers);
}

// If pVertexObject null, the Vertex/Index Buffers will not be reloaded, but the same ones
//	reused when CommandBuffers recreate.  Otherwise, pass a pointer to one to reload with,
//	or a VertexBaseObject with .vertices = nullptr to eliminate the Vertex Buffer altogether
//	(for instance if vertices are handled in-shader).

// Recreate each renderable; tear down and rebuild VkCommandBuffer Sets.
//	Note that commandPool can stay as-is.
//
void CommandControl::RecreateRenderables(VulkanSetup& vulkan)
{
	RecreateBuffers(vulkan.framebuffers);
	renderables.Recreate(vulkan);
	PostInitPrepBuffers(vulkan);
}
