//
// CommandObjects.h
//	Vulkan Setup
//
// Encapsulate Command Pool/Buffer/queues.
//		(TJ_LATER_NOTE: actually not the "queue" as that's actually device-side *receiver* of a command buffer)
//																		and more tied to the FrameBuffer (?)
// This class also coordinates two types of CommandBuffers: "fixed" ones
//	recorded at initialization-time, not re-recorded, but Submitted repeatedly;
//	and "active" CommandBuffers that re-record their commands upon every new frame.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef CommandObjects_h
#define CommandObjects_h

#include "GraphicsDevice.h"
#include "WindowSurface.h"
#include "Swapchain.h"
#include "RenderPass.h"
#include "Framebuffers.h"
#include "GraphicsPipeline.h"
#include "Descriptors.h"


class CommandPool
{
	friend class CommandObjects;
public:
	CommandPool(GraphicsDevice& graphicsDevice);
	~CommandPool();

	static CommandPool& Singleton(GraphicsDevice& graphics);
	VkCommandPool&		getVkInstance()	{ return vkInstance; }
private:
	static CommandPool*	pSelf;
	VkCommandPool		vkInstance;
	GraphicsDevice&		device;
};


enum RecordCommands {	/// i.e. Request this CommandBuffer to be recorded:
	AT_INIT_TIME_ONLY,	///  - once at initialization-time and not re-recorded before frames.
	UPON_EACH_FRAME		///  - repeatedly on every frame, and Reset before the next one.
};

class CommandBuffers
{

};


class CommandObjects
{
public:
	CommandObjects(Framebuffers& framebuffers, RenderPass& renderPass,
				   Swapchain& swapchain, GraphicsDevice& graphics);
	~CommandObjects();

		// MEMBERS
private:
	CommandPool		 commandPool;

	VkCommandBuffer* commandBuffers;
	uint32_t		 nCommandBuffers;

	GraphicsDevice&	 device;

	VkCommandBufferBeginInfo	beginInfo;

		// METHODS
private:
	void createCommandPool();
	void allocateCommandBuffers();
	void createCommandBuffers(vector<VkFramebuffer>& framebuffers, VkExtent2D& swapChainExtent,
							  VkRenderPass& renderPass);
	void recordCommands(vector<VkFramebuffer>& framebuffers, VkExtent2D& swapChainExtent,
						VkRenderPass& renderPass);
public:
	void Recreate(Framebuffers& framebuffers, RenderPass& renderPass,
				  Swapchain& swapchain, bool reloadMesh = false);
		// getters
	const VkCommandBuffer*	CommandBuffers()	{	return commandBuffers;	}
	uint32_t				NumCommandBuffers() {	return nCommandBuffers;	}
};

#endif // CommandObjects_h
