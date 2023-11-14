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
		bufferMemory(0),
		buffer(0)	// these are opaque, but assume 0 may indicate "uninitialized"
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
}


void PrimitiveBuffer::CreateVertexBuffer(vector<VertexAbstract> vertices)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	createDeviceLocalBuffer(vertices.data(), bufferSize,
							VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							buffer, bufferMemory);
}

void PrimitiveBuffer::CreateIndexBuffer(vector<IndexBufferDefaultIndexType> indices)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	createDeviceLocalBuffer(indices.data(), bufferSize,
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							buffer, bufferMemory);
}


void PrimitiveBuffer::createDeviceLocalBuffer(void* pSourceData, VkDeviceSize size, VkBufferUsageFlags usage,
											  VkBuffer& deviceBuffer, VkDeviceMemory& specificMemory)
{
	VkBuffer cpuSideBuffer;
	VkDeviceMemory cpuSideBufferMemory;
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
