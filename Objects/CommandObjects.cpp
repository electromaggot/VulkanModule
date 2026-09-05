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
#include "ResourceTracker.h"


#pragma mark - CommandPool

CommandPool::CommandPool(GraphicsDevice& graphicsDevice)
	:	device(graphicsDevice)
{
	create();
}
CommandPool::~CommandPool()
{
	destroy();
	Log(DEAD, "Destroyed: CommandPool");
}

void CommandPool::create()
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
void CommandPool::destroy()		// Idempotent for device-loss teardown.
{
	vkDestroyCommandPool(device.getLogical(), vkCommandPool, nullALLOC);
	vkCommandPool = VK_NULL_HANDLE;
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

	Log(DEAD, "Destroyed: CommandBufferSet");
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

void CommandBufferSet::recordCommands(vector<iRenderableBase*> pBufferRenderables, int iFrame, VkFramebuffer& framebuffer,
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

	// Build pipeline batches for optimized recording; reduces pipeline binds from O(N) to O(M).
	// Returns self-managed renderables (like ImGui) to render last.
	vector<iRenderableBase*> selfManagedRenderables = batchManager.buildBatches(pBufferRenderables);

	size_t numBufferSets = vkCommandBuffers.size();

	for (int iBuffer = 0; iBuffer < numBufferSets; ++iBuffer)
	{
		VkCommandBuffer& commandBuffer = vkCommandBuffers[iBuffer];

		call = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		if (call != VK_SUCCESS)
			Fatal("Fail to even Begin recording Command Buffer," + ErrStr(call));

//		if (iBuffer > 0)	// await prior buffer executions's completion
//			event.CmdWaitRecordTo(commandBuffer);

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Record all batches with optimized pipeline binding, self-managed renderables last.
		//	Note iFrame, NOT iBuffer: renderables index their per-frame descriptor sets (and secondary
		//	command buffers) with this.  See the DEV NOTE at the end of 'Adjunct/UniformBuffer.cpp'.
		batchManager.recordBatches(commandBuffer, iFrame, selfManagedRenderables);

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
	// CRITICAL: Clear renderables BEFORE destroying command pool and buffers!
	// Renderables contain Vulkan objects (pipelines, descriptors, etc.) that
	//	must be destroyed before the command pool and device are destroyed.
	renderables.Clear();

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
	vector<iRenderableBase*> mergedRenderables;
	BuildMergedVectorFromTypedSources(mergedRenderables);

	for (int iFrame = 0; iFrame < numFrames; ++iFrame) {	// Allocate and record command buffers for all frames.
		if (buffersByFrame[iFrame].numBufferSets() > 0)		// Free old buffers first (reload case: prevents accumulation).
			buffersByFrame[iFrame].freeVkCommandBuffers();
		buffersByFrame[iFrame].allocateVkCommandBuffer();
		buffersByFrame[iFrame].recordCommands(mergedRenderables, iFrame, vulkan.framebuffers[iFrame],
											  vulkan.swapchain.getExtent(), vulkan.renderPass.getVkRenderPass());
	}
}

void CommandControl::BuildMergedVectorFromTypedSources(vector<iRenderableBase*>& mergedRenderables)
{
	mergedRenderables.reserve(renderables.getNormalCount() + renderables.getSelfManagedCount());

	for (iRenderable* p : renderables.getNormalRenderables()) {				// Merge normal renderables.
		mergedRenderables.push_back(p);
	}
	for (iRenderableBase* p : renderables.getSelfManagedRenderables()) {	// Merge self-managed renderables.
		mergedRenderables.push_back(p);
	}
}

// (Re)Record command buffers for next frame.
//
void CommandControl::RecordRenderablesForNextFrame(VulkanSetup& vulkan, int iNextFrame)
{
	vector<iRenderableBase*> mergedRenderables;
	BuildMergedVectorFromTypedSources(mergedRenderables);

	// Record command buffer for next frame.
	buffersByFrame[iNextFrame].recordCommands(mergedRenderables, iNextFrame, vulkan.framebuffers[iNextFrame],
											  vulkan.swapchain.getExtent(), vulkan.renderPass.getVkRenderPass());
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
