//
// FrameCapture.cpp
//	VulkanModule Adjunct
//
// See header file comment for overview.
//
// 10/31/26 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "FrameCapture.h"
#include "ResourceTracker.h"


FrameCapture::FrameCapture(GraphicsDevice& graphicsDevice, VkCommandPool commandPool)
	:	ImageResource(graphicsDevice),
		graphicsDevice(graphicsDevice),
		commandPool(commandPool)
{ }


bool FrameCapture::Capture(Swapchain& swapchain, uint32_t iImage, VkSemaphore rendered)
{
	VkFormat format = swapchain.getFormat();
	VkExtent2D extent = swapchain.getExtent();
	if (!swapchain.canCapture() || !graphicsDevice.IsImageFormatSupported(format, VK_IMAGE_TILING_OPTIMAL)) {
		Log(WARN, "FrameCapture: this swapchain's images can't be copied from, or sampled in their format.");
		return false;
	}
	if (existsImage && (format != imageInfo.format || (uint32_t) imageInfo.wide != extent.width
												   || (uint32_t) imageInfo.high != extent.height)) {
		destroy();						// Discard existing frame kept (of incompatible format) and
		captured = false;				//	mark that old capture as gone, early: in case a below call
	}									//	throws e.g. `runtime_error` it still reflects correct state.
	if (!existsImage)
		create(format, extent);

	VkCommandBuffer commands;
	VkCommandBufferAllocateInfo allocInfo = {
		.sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext	= nullptr,
		.commandPool		= commandPool,
		.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount	= 1
	};
	vkAllocateCommandBuffers(device, &allocInfo, &commands);

	VkCommandBufferBeginInfo beginInfo = {
		.sType	= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext	= nullptr,
		.flags	= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = nullptr
	};
	vkBeginCommandBuffer(commands, &beginInfo);
	recordCopy(commands, swapchain.getImage(iImage), extent);
	vkEndCommandBuffer(commands);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;	// (The copy waits for the rendering.)
	VkSubmitInfo submitInfo = {
		.sType	= VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext	= nullptr,
		.waitSemaphoreCount	= 1,
		.pWaitSemaphores	= &rendered,
		.pWaitDstStageMask	= &waitStage,
		.commandBufferCount	= 1,
		.pCommandBuffers	= &commands,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores	  = nullptr
	};
	VkQueue& queue = graphicsDevice.Queues.getCurrent();
	call = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	if (call == VK_SUCCESS)
		vkQueueWaitIdle(queue);			// (Once in a while, so simply: its presentation then waits on nothing.)
	else
		Log(ERROR, "FrameCapture: submit of the copy FAILURE" + ErrStr(call));
	vkFreeCommandBuffers(device, commandPool, 1, &commands);

	captured = (call == VK_SUCCESS);	// Set outcome of new capture.
	return captured;
}

void FrameCapture::create(VkFormat format, VkExtent2D extent)
{
	imageInfo.format = format;
	imageInfo.wide = (int) extent.width;
	imageInfo.high = (int) extent.height;
	createImage(extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	// A frame as presented is opaque, whatever alpha its rendering left: so it's drawn, not blended.
	createImageView(VK_IMAGE_ASPECT_COLOR_BIT, { .a = VK_COMPONENT_SWIZZLE_ONE });
}


static VkImageMemoryBarrier layoutBarrier(VkImage image, VkImageLayout from, VkImageLayout to,
										  VkAccessFlags accessBefore, VkAccessFlags accessAfter)
{
	return {
		.sType	= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext	= nullptr,
		.srcAccessMask	= accessBefore,
		.dstAccessMask	= accessAfter,
		.oldLayout		= from,
		.newLayout		= to,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image	= image,
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
	};
}

// The swapchain image leaves presentation's layout for the copy and returns to it;
//	the capture's previous contents are discarded (UNDEFINED) and it ends ready to be sampled.
//
void FrameCapture::recordCopy(VkCommandBuffer commands, VkImage source, VkExtent2D extent)
{
	VkImageMemoryBarrier before[] = {
		layoutBarrier(source, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					  0, VK_ACCESS_TRANSFER_READ_BIT),
		layoutBarrier(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					  0, VK_ACCESS_TRANSFER_WRITE_BIT)
	};
	vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
						 0, 0, nullptr, 0, nullptr, 2, before);

	VkImageCopy region = {
		.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		.srcOffset		= { 0, 0, 0 },
		.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		.dstOffset		= { 0, 0, 0 },
		.extent			= { extent.width, extent.height, 1 }
	};
	vkCmdCopyImage(commands, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	VkImageMemoryBarrier after[] = {
		layoutBarrier(source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					  VK_ACCESS_TRANSFER_READ_BIT, 0),
		layoutBarrier(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, LAYOUT,
					  VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT)
	};
	vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
						 0, 0, nullptr, 0, nullptr, 2, after);
}
