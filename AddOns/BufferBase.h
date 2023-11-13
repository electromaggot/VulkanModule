//
// BufferBase.h
//	Vulkan Add-ons
//
// Base class for general buffer operations amongst specific buffer implementations.
//	Specifically: creating a general buffer and finding suitable memory for it.
//
// Created 6/14/19 by Tadd Jensen
//	© 2112 (uncopyrighted; use at will)
//
#ifndef BufferBase_h
#define BufferBase_h

#include "VulkanPlatform.h"
#include "GraphicsDevice.h"


class BufferBase
{
protected:
	BufferBase(GraphicsDevice& graphicsDevice)
		:	device(graphicsDevice.getLogical()),
			physicalDevice(graphicsDevice.getGPU())
	{ }

		// MEMBERS
	VkDevice&		  device;
	VkPhysicalDevice& physicalDevice;

		// METHODS

	void createGeneralBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
							 VkMemoryPropertyFlags properties,
							 VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo = {
			.sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext	= nullptr,
			.flags	= 0,
			.size		 = size,
			.usage		 = usage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount	= 0,
			.pQueueFamilyIndices	= nullptr
		};

		call = vkCreateBuffer(device, &bufferInfo, nullALLOC, &buffer);
		if (call != VK_SUCCESS)
			Fatal("Create Buffer FAILURE" + ErrStr(call));						// (**)

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(device, buffer, &memReqs);

		if (memReqs.size == 0)
			Fatal("vkAllocateMemory size ZERO crash-avoid calling createGeneralBuffer with VkDeviceSize %d", size);

		VkMemoryAllocateInfo allocInfo = {
			.sType	= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext	= nullptr,
			.allocationSize	 = memReqs.size,
			.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, properties)
		};

		call = vkAllocateMemory(device, &allocInfo, nullALLOC, &bufferMemory);	// (*)
		if (call != VK_SUCCESS)
			Fatal("Allocate Memory FAILURE" + ErrStr(call));					// (**)

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags reqBits)
	{
		VkPhysicalDeviceMemoryProperties memProps;

		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

		for (uint32_t iProperty = 0; iProperty < memProps.memoryTypeCount; ++iProperty)
			if (typeFilter & (1 << iProperty)
				&& (memProps.memoryTypes[iProperty].propertyFlags & reqBits) == reqBits)
				return iProperty;

		return Fatal("Failed to find suitable memory type.");
	}
};

#endif // BufferBase_h


/* DEV NOTES
 (*) - Note that according to this:
 https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer#page_Conclusion
 calls to vkAllocateMemory should be limited.  Instead of making singular
 allocations, they should be combined/stacked and the .offset parameter used.
 Also refers to: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator

 (**) - "OUT OF MEMORY" failures could instead "fail but not fatally" which is to
 say, log and return serious/internal error, but continue on the full expectation
 that calling code will handle gracefully, WITHOUT CRASHING which is exactly what
 a Fatal Throw does.  Some OOMs are game-enders, like a render buffer or a player
 3D model, but others are not, for example, sprites of a Particle Engine that has
 temporarily gone haywire.  In that case, if the OOM condition truly is temporary
 then failing gracefully will truly save the game from catostrophically crashing.
 So in the future, the handling of these Fatal failures could be reconsidered but
 will require careful thought and thorough testing.
*/
