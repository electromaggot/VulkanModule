//
// PrimitiveBuffer.h
//	Vulkan Setup
//
// Abstraction for Vertex and Index Buffers, but potentially any bufferable
//	graphics/geometry element related to a primitive or triangle.  Includes
//	intermediate ("staging") buffer for Vulkan-command-queued transfer of
//	data from CPU/main RAM to, e.g., VRAM.
// Passed-in dependency VkCommandPool can either be the same one used for drawing
//	commands (easy) or a separate pool using VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
//	and short-lived buffers that may be optimized for memory allocations.
//
// Created 6/14/19 by Tadd Jensen
//	© 2112 (uncopyrighted; use at will)
//
#ifndef PrimitiveBuffer_h
#define PrimitiveBuffer_h

#include "CommandBufferBase.h"
#include "DeviceQueues.h"
#include "MeshObject.h"


class PrimitiveBuffer : BufferBase, CommandBufferBase
{
public:
	PrimitiveBuffer(VkCommandPool& pool, GraphicsDevice& device);
	PrimitiveBuffer(MeshObject& meshObject, VkCommandPool& pool, GraphicsDevice& device);
	PrimitiveBuffer(IndexBufferDefaultIndexType* pIndices, uint32_t nIndices,
					VkCommandPool& pool, GraphicsDevice& device);
	PrimitiveBuffer(MeshIndexType indexType, void* pIndices, uint32_t nIndices,
					VkCommandPool& pool, GraphicsDevice& device);
	~PrimitiveBuffer();

		// MEMBERS
private:
	// A DEVICE-LOCAL buffer is written once at creation, so one copy serves every frame.  A HOST-VISIBLE one (dynamic
	//	geometry) is rewritten while earlier frames may still be reading it, so it gets one copy per swapchain image
	//	and each frame maps only its own.  Both cases live in these vectors; `buffers.size()` is 1 or N accordingly.
	vector<VkBuffer>		buffers;
	vector<VkDeviceMemory>	buffersMemory;

		// METHODS
public:
	void	 CreateVertexBuffer(vector<VertexAbstract> vertices);
	void	 CreateIndexBuffer(vector<IndexBufferDefaultIndexType> indices);
	// numFrames applies only when hostVisible: how many per-frame copies to allocate.
	void	 CreateVertexBuffer(void* pVertexData, VkDeviceSize bufferSize, bool hostVisible, uint32_t numFrames = 1);
	void	 CreateIndexBuffer(void* pIndexData, VkDeviceSize bufferSize, MeshIndexType indexType,
							   bool hostVisible, uint32_t numFrames = 1);
	void	 UpdateVertexBuffer(void* pNewVertexData, VkDeviceSize size);		// Update existing vertex buffer, only for host-visible buffers.
	// Fast update for host-visible buffers, no command buffers.  Writes ONLY iFrame's copy, so it
	//	must be called for the frame being drawn - never for one still in flight.
	void	 UpdateVertexBufferMapped(void* pNewVertexData, VkDeviceSize size, uint32_t iFrame = 0);
	void	 UpdateIndexBufferMapped(void* pNewIndexData, VkDeviceSize size, uint32_t iFrame = 0);
private:
	void	 createDeviceLocalBuffer(void* pSourceData, VkDeviceSize size, VkBufferUsageFlags usage,
									 VkBuffer& deviceBuffer, VkDeviceMemory& deviceMemory);
	void	 copyBufferViaVulkan(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void	 mapAndCopy(VkDeviceMemory memory, void* pData, VkDeviceSize size, const char* whatFailed);

		// getters
public:
	// The buffer for a given frame.  A single-copy (device-local) buffer answers the same one for
	//	every frame, so callers need not know which kind they hold.
	VkBuffer&	getVk(uint32_t iFrame = 0)	{ return buffers[iFrame < buffers.size() ? iFrame : 0]; }

	uint32_t	NumCopies() const			{ return (uint32_t) buffers.size(); }
};

#endif // PrimitiveBuffer_h
