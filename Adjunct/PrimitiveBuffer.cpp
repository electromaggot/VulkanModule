//
// PrimitiveBuffer.cpp
//	Vulkan Setup
//
// See header description.
//
// TODO: Vertex and Index Buffers, and really any other such buffers, should
//	SHARE singular Allocations (vkAllocateMemory calls) and fully leverage, e.g.
//	vkBindBufferMemory.memoryOffset.  As mentioned in the last paragraph of this:
//	https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer#page_Using-an-index-buffer
//
// Created 6/14/19 by Tadd Jensen
//	© 2112 (uncopyrighted; use at will)
//
#include "PrimitiveBuffer.h"


PrimitiveBuffer::PrimitiveBuffer(VkCommandPool& pool, GraphicsDevice& device)
	:	BufferBase(device),
		CommandBufferBase(pool, device),
		buffer(0),		// These buffers are opaque, but
		bufferMemory(0)	//	assume 0 indicates "uninitialized."
{ }

PrimitiveBuffer::PrimitiveBuffer(MeshObject& meshObject, VkCommandPool& pool, GraphicsDevice& device)
	:	PrimitiveBuffer(pool, device)
{
	createDeviceLocalBuffer(meshObject.vertices, meshObject.vertexBufferSize(),
							VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							buffer, bufferMemory);
}

PrimitiveBuffer::PrimitiveBuffer(IndexBufferDefaultIndexType* pIndices, uint32_t nIndices, VkCommandPool& pool, GraphicsDevice& device)
	:	PrimitiveBuffer(pool, device)
{
	createDeviceLocalBuffer(pIndices, nIndices * sizeof(pIndices[0]),
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							buffer, bufferMemory);
}

PrimitiveBuffer::PrimitiveBuffer(MeshIndexType indexType, void* pIndices, uint32_t nIndices, VkCommandPool& pool, GraphicsDevice& device)
	:	PrimitiveBuffer(pool, device)
{
	createDeviceLocalBuffer(pIndices, nIndices * MeshIndexByteSizes[indexType],
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							buffer, bufferMemory);
}

PrimitiveBuffer::~PrimitiveBuffer()
{
	if (buffer) {
		vkDestroyBuffer(device, buffer,  nullALLOC);
		buffer = 0;
	}
	if (bufferMemory) {
		vkFreeMemory(device, bufferMemory, nullALLOC);
		bufferMemory = 0;
	}
	Log(DEAD, "Destroyed: PrimitiveBuffer (buffer, memory)");
}


void PrimitiveBuffer::CreateVertexBuffer(vector<VertexAbstract> vertices)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	createDeviceLocalBuffer(vertices.data(), bufferSize,
							VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							buffer, bufferMemory);
}

// Create host-visible vertex buffer for dynamic geometry, e.g. particles, animated models, waveforms...
//	hostVisible = true:  CPU-accessible, fast updates via mapping → no command buffers!
//	hostVisible = false: GPU-only, requires staging buffer for updates → one-time initialization.
//
void PrimitiveBuffer::CreateVertexBuffer(void* pVertexData, VkDeviceSize bufferSize, bool hostVisible)
{
	if (!hostVisible) {		// Use standard device-local staging buffer approach:
		createDeviceLocalBuffer(pVertexData, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, buffer, bufferMemory);
		return;
	}

	// Create host-visible buffer, CPU-accessible for direct updates.
	createGeneralBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						buffer, bufferMemory);

	void* pData;			// Map, copy initial data, unmap:
	call = vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &pData);
	if (call != VK_SUCCESS)
		Fatal("CreateVertexBuffer (host-visible) Map Memory FAILURE" + ErrStr(call));

	if (pVertexData)
		memcpy(pData, pVertexData, (size_t) bufferSize);
	else
		memset(pData, 0, (size_t) bufferSize);
	vkUnmapMemory(device, bufferMemory);
}

void PrimitiveBuffer::CreateIndexBuffer(vector<IndexBufferDefaultIndexType> indices)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	createDeviceLocalBuffer(indices.data(), bufferSize,
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							buffer, bufferMemory);
}

// Create host-visible index buffer for dynamic geometry (same pattern as CreateVertexBuffer).
//	hostVisible = true:  CPU-accessible, fast updates via mapping → no command buffers!
//	hostVisible = false: GPU-only, requires staging buffer for updates.
//
void PrimitiveBuffer::CreateIndexBuffer(void* pIndexData, VkDeviceSize bufferSize, MeshIndexType indexType, bool hostVisible)
{
	if (!hostVisible) {		// Use standard device-local staging buffer approach:
		createDeviceLocalBuffer(pIndexData, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, buffer, bufferMemory);
		return;
	}

	// Create host-visible buffer, CPU-accessible for direct updates.
	createGeneralBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						buffer, bufferMemory);

	void* pData;			// Map, copy initial data, unmap:
	call = vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &pData);
	memcpy(pData, pIndexData, (size_t) bufferSize);
	vkUnmapMemory(device, bufferMemory);
}

// Update existing vertex buffer with new data, for dynamic geometry like animated models, particles, waveforms.
//	Efficient for frequent updates - uses staging buffer to transfer new data to device-local buffer.
//	CRITICAL: size must match original buffer size → no reallocation, just data update.
//
void PrimitiveBuffer::UpdateVertexBuffer(void* pNewVertexData, VkDeviceSize size)
{
	VkBuffer stagingBuffer;		// Create temporary staging buffer. (CPU-accessible)
	VkDeviceMemory stagingMemory;
	createGeneralBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						stagingBuffer, stagingMemory);

	void* pData;				// Map staging buffer, copy new vertex data, unmap:
	call = vkMapMemory(device, stagingMemory, 0, size, 0, &pData);
	if (call != VK_SUCCESS)
		Fatal("UpdateVertexBuffer Map Memory FAILURE" + ErrStr(call));

	memcpy(pData, pNewVertexData, (size_t)size);
	vkUnmapMemory(device, stagingMemory);

	// Copy staging buffer to existing device-local vertex buffer via Vulkan command:
	copyBufferViaVulkan(stagingBuffer, buffer, size);

	vkDestroyBuffer(device, stagingBuffer, nullALLOC);		// Clean up staging buffer.
	vkFreeMemory(device, stagingMemory, nullALLOC);			//
}


void PrimitiveBuffer::createDeviceLocalBuffer(void* pSourceData, VkDeviceSize size, VkBufferUsageFlags usage,
											  VkBuffer& deviceBuffer, VkDeviceMemory& specificMemory)
{
	VkBuffer cpuSideBuffer;
	VkDeviceMemory cpuSideBufferMemory = 0;
	createGeneralBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						cpuSideBuffer, cpuSideBufferMemory);
	void* pData;
	call = vkMapMemory(device, cpuSideBufferMemory, 0, size, 0, &pData);
	if (call != VK_SUCCESS)												// seems unusual for this to fail since create()
		Fatal("Primitive Buffer Map Memory FAILURE" + ErrStr(call));	//	succeeded; see (**) Dev Note in BufferBase.h

	memcpy(pData, pSourceData, (size_t) size);						// fill the main RAM block
	vkUnmapMemory(device, cpuSideBufferMemory);						//	that Vulkan provided

	createGeneralBuffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						deviceBuffer, specificMemory);

	copyBufferViaVulkan(cpuSideBuffer, deviceBuffer, size);

	vkDestroyBuffer(device, cpuSideBuffer, nullALLOC);
	vkFreeMemory(device, cpuSideBufferMemory, nullALLOC);
}

// Fast update for host-visible vertex buffers, dynamic geometry like waveforms, particles, animated models.
//	Direct CPU memory mapping → NO staging buffers, NO command buffers!
//	Industry standard for 60fps dynamic geometry updates.
//
void PrimitiveBuffer::UpdateVertexBufferMapped(void* pNewVertexData, VkDeviceSize size)
{
	void* pData;					// Map GPU memory directly to CPU address space.
	call = vkMapMemory(device, bufferMemory, 0, size, 0, &pData);
	if (call != VK_SUCCESS)
		Fatal("UpdateVertexBufferMapped Map Memory FAILURE" + ErrStr(call));

	// Copy new vertex data directly → extremely fast, no GPU involvement.
	memcpy(pData, pNewVertexData, (size_t)size);

	// Unmap...  HOST_COHERENT flag means no manual flush needed → driver handles it.
	vkUnmapMemory(device, bufferMemory);
}

// Fast update for host-visible index buffers, dynamic terrain like visibility window scrolling.
//	Direct CPU memory mapping → NO staging buffers, NO command buffers!
//	Mirrors UpdateVertexBufferMapped() for index buffer updates.
//
void PrimitiveBuffer::UpdateIndexBufferMapped(void* pNewIndexData, VkDeviceSize size)
{
	void* pData;					// Map GPU memory directly to CPU address space.
	call = vkMapMemory(device, bufferMemory, 0, size, 0, &pData);
	if (call != VK_SUCCESS)
		Fatal("UpdateIndexBufferMapped Map Memory FAILURE" + ErrStr(call));

	// Copy new index data directly → extremely fast, no GPU involvement.
	memcpy(pData, pNewIndexData, (size_t)size);

	// Unmap...  HOST_COHERENT flag means no manual flush needed → driver handles it.
	vkUnmapMemory(device, bufferMemory);
}

void PrimitiveBuffer::copyBufferViaVulkan(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commands = beginSingleSubmitCommands();

		VkBufferCopy copyRegion = {
			.srcOffset	= 0,
			.dstOffset	= 0,
			.size		= size
		};
		const uint32_t	nCopyRegions = 1;

		vkCmdCopyBuffer(commands, srcBuffer, dstBuffer, nCopyRegions, &copyRegion);

	endAndSubmitCommands(commands);
}
