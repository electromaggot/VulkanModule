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
		buffers(1, VK_NULL_HANDLE),			// One copy until a host-visible Create() asks for more.
		buffersMemory(1, VK_NULL_HANDLE)	//	(These are opaque; null indicates "uninitialized.")
{ }

PrimitiveBuffer::PrimitiveBuffer(MeshObject& meshObject, VkCommandPool& pool, GraphicsDevice& device)
	:	PrimitiveBuffer(pool, device)
{
	createDeviceLocalBuffer(meshObject.vertices, meshObject.vertexBufferSize(),
							VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							buffers[0], buffersMemory[0]);
}

PrimitiveBuffer::PrimitiveBuffer(IndexBufferDefaultIndexType* pIndices, uint32_t nIndices, VkCommandPool& pool, GraphicsDevice& device)
	:	PrimitiveBuffer(pool, device)
{
	createDeviceLocalBuffer(pIndices, nIndices * sizeof(pIndices[0]),
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							buffers[0], buffersMemory[0]);
}

PrimitiveBuffer::PrimitiveBuffer(MeshIndexType indexType, void* pIndices, uint32_t nIndices, VkCommandPool& pool, GraphicsDevice& device)
	:	PrimitiveBuffer(pool, device)
{
	createDeviceLocalBuffer(pIndices, nIndices * MeshIndexByteSizes[indexType],
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							buffers[0], buffersMemory[0]);
}

PrimitiveBuffer::~PrimitiveBuffer()
{
	for (size_t i = 0; i < buffers.size(); ++i) {
		if (buffers[i]) {
			vkDestroyBuffer(device, buffers[i], nullALLOC);
			buffers[i] = VK_NULL_HANDLE;
		}
		if (buffersMemory[i]) {
			vkFreeMemory(device, buffersMemory[i], nullALLOC);
			buffersMemory[i] = VK_NULL_HANDLE;
		}
	}
	Log(DEAD, "Destroyed: PrimitiveBuffer (%zu buffer(s), memory)", buffers.size());
}


// Refuse a copy larger than what was allocated.  Fatal on purpose: the alternative is vkMapMemory returning a shorter
//	region than asked for (or nothing at all) and the following memcpy running off the end - a SIGSEGV whose backtrace
//	points at memmove, far from whoever supplied the oversized data.  Named sizes here let that caller be identified
//																	immediately instead of inferred from a crash dump.
void PrimitiveBuffer::verifyFitsAllocation(VkDeviceSize size, const char* whatFailed)
{
	if (size > allocatedSize)
		Fatal(string(whatFailed) + " OVERRUN: asked to write " + std::to_string(size) + " bytes into a "
			  + std::to_string(allocatedSize) + "-byte allocation.  Dynamic geometry must not outgrow"
			  + " its initial allocation - reallocate (AddOns::Recreate) rather than updating in place.");
}

// Map one host-visible allocation, copy into it, unmap.  Shared by the create/update paths below.
//
void PrimitiveBuffer::mapAndCopy(VkDeviceMemory memory, void* pData, VkDeviceSize size, const char* whatFailed)
{
	void* pMapped;
	call = vkMapMemory(device, memory, 0, size, 0, &pMapped);
	if (call != VK_SUCCESS)
		Fatal(string(whatFailed) + " Map Memory FAILURE" + ErrStr(call));

	if (pData)
		memcpy(pMapped, pData, (size_t) size);
	else
		memset(pMapped, 0, (size_t) size);

	vkUnmapMemory(device, memory);		// HOST_COHERENT, so no manual flush needed.
}

void PrimitiveBuffer::CreateVertexBuffer(vector<VertexAbstract> vertices)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	createDeviceLocalBuffer(vertices.data(), bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, buffers[0], buffersMemory[0]);
}

// Create host-visible vertex buffer for dynamic geometry, e.g. particles, animated models, waveforms...
//	hostVisible = true:  CPU-accessible, fast updates via mapping → no command buffers!
//	hostVisible = false: GPU-only, requires staging buffer for updates → one-time initialization.
//
void PrimitiveBuffer::CreateVertexBuffer(void* pVertexData, VkDeviceSize bufferSize, bool hostVisible,
										 uint32_t numFrames, VkDeviceSize dataSize)
{
	if (dataSize == 0 || dataSize > bufferSize)		// Copy only what the SOURCE holds; see the header.
		dataSize = bufferSize;

	if (!hostVisible) {		// Use standard device-local staging buffer approach:
		createDeviceLocalBuffer(pVertexData, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, buffers[0],
								buffersMemory[0], dataSize);
		return;				//	(one copy: written once here, never again, so no frame can race it)
	}

	// Host-visible means REWRITTEN while earlier frames may still be reading - so allocate one copy per frame, each mapped
	//	only by the frame that owns it.  A single shared copy gets overwritten out from under already-submitted frames; see
	buffers.assign(numFrames > 0 ? numFrames : 1, VK_NULL_HANDLE);								//	DEV NOTE at end of file.
	buffersMemory.assign(buffers.size(), VK_NULL_HANDLE);
	allocatedSize = bufferSize;				// Each copy is this size (see verifyFitsAllocation).

	for (size_t i = 0; i < buffers.size(); ++i) {
		createGeneralBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							buffers[i], buffersMemory[i]);
		mapAndCopy(buffersMemory[i], pVertexData, dataSize, "CreateVertexBuffer (host-visible)");
	}
}

void PrimitiveBuffer::CreateIndexBuffer(vector<IndexBufferDefaultIndexType> indices)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	createDeviceLocalBuffer(indices.data(), bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, buffers[0], buffersMemory[0]);
}

// Create host-visible index buffer for dynamic geometry (same pattern as CreateVertexBuffer).
//	hostVisible = true:  CPU-accessible, fast updates via mapping → no command buffers!
//	hostVisible = false: GPU-only, requires staging buffer for updates.
//
void PrimitiveBuffer::CreateIndexBuffer(void* pIndexData, VkDeviceSize bufferSize, MeshIndexType indexType,
										bool hostVisible, uint32_t numFrames, VkDeviceSize dataSize)
{
	if (dataSize == 0 || dataSize > bufferSize)		// Copy only what the SOURCE holds; see the header.
		dataSize = bufferSize;

	if (!hostVisible) {		// Use standard device-local staging buffer approach:
		createDeviceLocalBuffer(pIndexData, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, buffers[0],
								buffersMemory[0], dataSize);
		return;
	}

	buffers.assign(numFrames > 0 ? numFrames : 1, VK_NULL_HANDLE);	// One copy per frame, exactly
	buffersMemory.assign(buffers.size(), VK_NULL_HANDLE);			//	as CreateVertexBuffer above.
	allocatedSize = bufferSize;				// Each copy is this size (see verifyFitsAllocation).

	for (size_t i = 0; i < buffers.size(); ++i) {
		createGeneralBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							buffers[i], buffersMemory[i]);
		mapAndCopy(buffersMemory[i], pIndexData, dataSize, "CreateIndexBuffer (host-visible)");
	}
}

// Update existing vertex buffer with new data, for dynamic geometry like animated models, particles, waveforms.
//	Efficient for frequent updates - uses staging buffer to transfer new data to device-local buffer.
//	CRITICAL: size must match original buffer size → no reallocation, just data update.
//
void PrimitiveBuffer::UpdateVertexBuffer(void* pNewVertexData, VkDeviceSize size)
{
	verifyFitsAllocation(size, "UpdateVertexBuffer");	// (the device-local copy below is the one sized)

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
	copyBufferViaVulkan(stagingBuffer, buffers[0], size);	// (device-local, so a single copy)

	vkDestroyBuffer(device, stagingBuffer, nullALLOC);		// Clean up staging buffer.
	vkFreeMemory(device, stagingMemory, nullALLOC);			//
}


void PrimitiveBuffer::createDeviceLocalBuffer(void* pSourceData, VkDeviceSize size, VkBufferUsageFlags usage,
											  VkBuffer& deviceBuffer, VkDeviceMemory& specificMemory,
											  VkDeviceSize dataSize)
{
	allocatedSize = size;		// Remember what we own (see verifyFitsAllocation).

	if (dataSize == 0 || dataSize > size)		// Read only what the source holds, write all we allocated.
		dataSize = size;

	VkBuffer cpuSideBuffer;
	VkDeviceMemory cpuSideBufferMemory = 0;
	createGeneralBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						cpuSideBuffer, cpuSideBufferMemory);
	void* pData;
	call = vkMapMemory(device, cpuSideBufferMemory, 0, size, 0, &pData);
	if (call != VK_SUCCESS)												// seems unusual for this to fail since create()
		Fatal("Primitive Buffer Map Memory FAILURE" + ErrStr(call));	//	succeeded; see (**) Dev Note in BufferBase.h

	memcpy(pData, pSourceData, (size_t) dataSize);			// Fill the main RAM block that Vulkan provided —
	vkUnmapMemory(device, cpuSideBufferMemory);				//	only as far as the SOURCE actually goes.

	createGeneralBuffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						deviceBuffer, specificMemory);

	copyBufferViaVulkan(cpuSideBuffer, deviceBuffer, size);

	vkDestroyBuffer(device, cpuSideBuffer, nullALLOC);
	vkFreeMemory(device, cpuSideBufferMemory, nullALLOC);
}

// Fast update for host-visible vertex buffers, dynamic geometry like waveforms, particles, animated models.  Direct
//	CPU memory mapping → NO staging buffers, NO command buffers!  Industry standard for 60fps dynamic geometry updates.
//
void PrimitiveBuffer::UpdateVertexBufferMapped(void* pNewVertexData, VkDeviceSize size, uint32_t iFrame)
{
	verifyFitsAllocation(size, "UpdateVertexBufferMapped");

	// Only THIS frame's copy - the others may be mid-flight.  Map, copy, unmap: extremely fast,
	//	no GPU involvement, and HOST_COHERENT means no manual flush.
	mapAndCopy(buffersMemory[iFrame < buffersMemory.size() ? iFrame : 0], pNewVertexData, size,
			   "UpdateVertexBufferMapped");
}

// Fast update for host-visible index buffers, dynamic terrain like visibility window scrolling.
//	Direct CPU memory mapping → NO staging buffers, NO command buffers!
//	Mirrors UpdateVertexBufferMapped() for index buffer updates.
//
void PrimitiveBuffer::UpdateIndexBufferMapped(void* pNewIndexData, VkDeviceSize size, uint32_t iFrame)
{
	verifyFitsAllocation(size, "UpdateIndexBufferMapped");

	mapAndCopy(buffersMemory[iFrame < buffersMemory.size() ? iFrame : 0], pNewIndexData, size,
			   "UpdateIndexBufferMapped");		// This frame's copy only (see the vertex version).
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


/* DEV NOTE - why host-visible buffers are per-frame, and device-local ones are not

   DEVICE-LOCAL buffer is filled once, at creation, through a staging copy, and never written
	again.  Nothing can race it, so one copy serves every frame.
   HOST-VISIBLE buffer exists precisely to be rewritten - dynamic geometry: waveforms, laser and tracer shots, scrolling
	terrain, text that changes.  Until mid-2026 there was still only ONE of those, and UpdateVertexBufferMapped()
	memcpy'd straight into it whenever the application asked.  With frames in flight that memcpy lands in memory, the
	GPU may still be reading for a frame already submitted, so that frame draws a mixture of old and new geometry.
   Symptom: a tear in SHAPE rather than position, it is intermittent - depending on where the GPU happens to be - so it
	reads as "occasionally glitchy" rather than broken.  It was known: LevelEdit's terrain path guards its uploads with
	vkDeviceWaitIdle() & says why.  That works, but stalls the entire device, & only tolerable because terrain scrolls
	rarely.  Geometry updated every frame or two cannot pay that price, so the waveform simply did not, and glitched.
   Now each host-visible buffer has one copy per swapchain image and a frame maps only its own.  The awkward part
	is that applications may hand over new data at any point in their frame - often before the swapchain image is
	acquired, when the frame is not yet known - so AddOns STAGES the data and marks every frame stale; each frame
	copies into its own buffer the first time it draws (AddOns::uploadStagedGeometry, called from the draw path).
	Hence one CPU-side copy per update, in exchange for no stalls and no tearing.
   A consequence worth collecting later: the vkDeviceWaitIdle() calls guarding terrain uploads in
	LevelEdit are now redundant, and removing them should measurably shorten terrain-scroll frames.
*/
