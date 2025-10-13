//
// CommandObjects.h
//	Vulkan Setup
//
// Encapsulate Command Pool and Command Buffers.
//	(but not the "command queue" as that's device-side *receiver* of a command buffer and more tied to the Framebuffer)
//
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
#include "EventObjects.h"
#include "Descriptors.h"

#include "iRenderable.h"


#pragma mark - COMMAND POOL

class CommandPool
{
	friend class CommandControl;
public:
	CommandPool(GraphicsDevice& graphicsDevice);
	~CommandPool();

	VkCommandPool getVkCommandPool() const { return vkCommandPool; }

private:
	VkCommandPool	 vkCommandPool;
	GraphicsDevice&	 device;
};


#pragma mark - COMMAND BUFFERS

class CommandBufferSet
{
public:
	CommandBufferSet();
	~CommandBufferSet();

	vector<VkCommandBuffer>	 vkCommandBuffers;		// size == numBufferSets

	bool					 isChanged_ReRecord;	//TJ_TODO: needs to re-record all frames
private:
	VkCommandBufferBeginInfo beginInfo;
	Event&					 event;

		// METHODS
public:
	void allocateVkCommandBuffer();
	void freeVkCommandBuffers();
	void recordCommands(vector<iRenderable*> pRenderables, VkFramebuffer& framebuffer,
						VkExtent2D& swapChainExtent, VkRenderPass& renderPass);
		// getters
	uint32_t	numBufferSets()	 {	return (uint32_t) vkCommandBuffers.size();	}
};

typedef CommandBufferSet*	CommandBufferSets;	// array of the above per each frame


#pragma mark - COMMAND CONTROL

// Initially, receive/store the number of Framebuffers - that's how many CommandBuffers
//	are needed for any given Renderable.  As each Renderable is defined, it needs a set
//	of CommandBuffers, but those sets may be shared with other Renderables, according to
//	each one's behavior, like CommandRecording mode, or if ordering is more important...
// If overall app uses painter's algorithm (no depth buffer), render order is preserved,
//	even if it means redundant (non-contiguous, otherwise-equivalent) CommandBuffers.
//	Otherwise, order-of-rendering doesn't matter, so buffers can be combined and filled
//	with similar Renderables' commands regardless of when those Renderables were added.
// TJ_TODO: Later Confirm: Is absolutely independent of UBO like MVP?
//
//							SIMPLIFIED DIAGRAM
//	VkCommandBuffer == [ ]						(more in Dev Note at .cpp EOF)
//							iCommandBufferSet
//						 0		 1		 2	 ...   n  = numBufferSets
//		iFrame		0	[ ]	 ,	[ ]	 ,	[ ]		  [ ]
//					1	[ ]	 ,	[ ]	 ,	[ ]	 ...  [ ]
//					2	[ ]	 ,	[ ]	 ,	[ ]		  [ ]
//		NumFrames =	3
//

class CommandControl
{
	static CommandControl*	pSingleton;

		// XSTRUCT
public:
	CommandControl(Framebuffers& framebuffers, GraphicsDevice& device);
	~CommandControl();

		// MEMBERS
	bool		renderInOrderAdded;		// i.e. draw Renderables in same order as they were added

	Renderables	renderables;						// size == numBufferSets

	// Side note: Conceptually, "Renderables" may seem like they should be separate from the Vulkan
	//	Module, however the Renderable and its Command Buffer go hand-in-hand (that is, the specific
	//	draw commands it writes into the buffer); the same goes for its Shaders, their Pipeline, etc.

private:
	CommandPool	commandPool;

	uint32_t	numFrames;

	CommandBufferSets	buffersByFrame;				// size == numFrames (Framebuffers.size())

		// METHODS
	void recordCommands(int iFrame, vector<iRenderable*> pRenderables, VulkanSetup& vulkan);

public:
	void Create(Framebuffers& framebuffers);
	void Destroy();

	void PostInitPrepBuffers(VulkanSetup& vulkan);
	void RecordRenderablesUponEachFrame(VulkanSetup& vulkan);
	void RecordRenderablesForNextFrame(VulkanSetup& vulkan, int iNextFrame);

	vector<VkCommandBuffer>	BuffersForFrame(int iFrame) {
		assert (numFrames > 0);
		return buffersByFrame[iFrame].vkCommandBuffers;
	}

	void RecreateBuffers(Framebuffers& framebuffers);
	void RecreateRenderables(VulkanSetup& vulkan);

		// getters
	uint32_t				NumFrames()	{ return numFrames; }
	CommandPool&			getCommandPool() { return commandPool; }

	static GraphicsDevice&	device()	{ return pSingleton->commandPool.device; }
	static VkCommandPool&	vkPool()	{ return pSingleton->commandPool.vkCommandPool; }
};


#endif // CommandObjects_h



/* DEV NOTE

given ARRAYS SIZED:
    commandBufferRecordables[numBufferSets]
    commandBuffersByFrame[numFrames].vkCommandBuffers[numBufferSets]

EXAMPLE INITIALIZATION, graphical representation			numBufferSets = 3
----------------------										NumFrames = 2

       bufferRecordables[┐]  =	 {	{AT_INIT_TIME_ONLY,		{ON_CHANGE_FLAGGED,		{UPON_EACH_FRAME,
						 │			 renderables[0..1]},	 renderables[2]},		 renderables[3..4]}	};
       buffersByFrame[┐] └─> iBufferSet ──> [0]			|		   [1]			|		  [2]			╷
		 ┌─ iFrame <──┘		 ↑	 ╷						|						|						|
		 ↓					 │	 ╵						|						|						╵
		[0].vkCommandBuffers[┘] = {	VkCommandBufferA,	   VkCommandBufferB,	   VkCommandBufferC		};
								 ╷	 {Renderable0 cmds	|	{Renderable2 cmds}	|	{Renderable3 cmds	╷
								 ╵	  Renderable1 cmds}	|						|	 RenderGUI cmds}	╵
		[1].vkCommandBuffers[] = {	VkCommandBufferD,	  VkCommandBufferE,		   VkCommandBufferF		};
								 ╷	 {Renderable0 cmds	|	{Renderable2 cmds}	|	{Renderable3 cmds	╷
								 |	  Renderable1 cmds}	|					 	|	 RenderGUI cmds}	|

											 ↑						↑						  ↑
			These won't ever get re-recorded ┘						│		These Renderables ┘ re-record their
																	│			render commands every frame.
						This one re-records only if its isChanged_ReRecord
						flag gets set, which will probably not coincide with other
						renderables also using this mode, thus require separate additional sets.
*/
