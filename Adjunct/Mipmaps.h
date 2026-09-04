//
// Mipmaps.h
//	Vulkan Add-ons
//
// Encapsulate mipmapping, including explicit creation.
//
// Created 7/7/19 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef Mipmaps_h
#define Mipmaps_h

#include "CommandBufferBase.h"
#include <cmath>


class Mipmaps : CommandBufferBase
{
public:
	Mipmaps(VkCommandPool& pool, GraphicsDevice& device)
		:	CommandBufferBase(pool, device),
			numLevels(1)		// i.e. DISABLED: just the one full-size image, no chain.
	{ }							//	Nothing gets a chain it did not explicitly ask for.

		// MEMBER
private:
	uint32_t	numLevels;

		// METHODS
public:
	// How many levels a full chain WOULD have, for these dimensions.  Pure -- static, and touches
	//	no state, so it is safe to call when merely sizing or reporting.
	//
	static uint32_t LevelsToFit(int32_t textureWidth, int32_t textureHeight)
	{
		return static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1;
	}

	// OPT IN to mipmapping.  The one and only mutator, and deliberately a verb: an earlier
	//	CalculateNumberOfLevels() both computed AND assigned while reading like a query, so callers
	//	(ImageResource::createImage among them) silently enabled mipmapping just by asking how many
	//	levels there were.  See the DEV NOTE at the end of Objects/ImageResource.cpp.
	//
	void UseFullChain(int32_t textureWidth, int32_t textureHeight)
	{
		numLevels = LevelsToFit(textureWidth, textureHeight);
	}

	// Ask THIS, not "numLevels > 1", so that allocating levels and filling them cannot disagree:
	//	whoever sizes the image and whoever generates the chain read the same single answer.
	//
	bool		IsEnabled() const	{ return numLevels > 1; }

	uint32_t	NumLevels()  const	{ return numLevels; }


	void Generate(VkImage image, VkFormat imageFormat, int32_t textureWide, int32_t textureHigh)
	{
		if (! IsEnabled()) {	// Nothing to blit into -- the image was sized for a single level.
			Log(WARN, "Generate() called without UseFullChain() -- no mipmaps to generate.");
			return;
		}

		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(graphicsDevice.getGPU(), imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			Fatal("Texture image format does not support linear blitting.");

		VkCommandBuffer commandBuffer = beginSingleSubmitCommands();

		VkImageMemoryBarrier barrier = {
			.sType	= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext	= nullptr,
			.srcAccessMask	= VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask	= VK_ACCESS_TRANSFER_READ_BIT,
			.oldLayout			= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout			= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED,
			.image			= image,
			.subresourceRange = {
				.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel	= 0,
				.levelCount		= 1,
				.baseArrayLayer	= 0,
				.layerCount		= 1
			}
		};

		int32_t mipWide  = textureWide;
		int32_t mipHigh  = textureHigh;

		for (uint32_t iLevel = 1; iLevel < numLevels; ++iLevel) {

			vkCmdPipelineBarrier(commandBuffer,
								 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
								 0, nullptr,
								 0, nullptr,
								 1, &barrier);

			VkImageBlit blit = {
				.srcSubresource = {
					.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel		= iLevel - 1,
					.baseArrayLayer	= 0,
					.layerCount		= 1
				},
				.srcOffsets =	{	{ 0, 0, 0 },						// [0]
									{ mipWide, mipHigh, 1 }				// [1]
								},
				.dstSubresource = {
					.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel		= iLevel,
					.baseArrayLayer	= 0,
					.layerCount		= 1
				},
				.dstOffsets =	{	{ 0, 0, 0 },						// [0]
									{ mipWide > 1 ? mipWide / 2 : 1,	 //
									  mipHigh > 1 ? mipHigh / 2 : 1,	// [1]
									  1 }							   //
								}
			};

			vkCmdBlitImage(commandBuffer,
						   image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						   1, &blit,
						   VK_FILTER_LINEAR);

			barrier.oldLayout	= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask	= VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask	= VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
								 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
								 0, nullptr,
								 0, nullptr,
								 1, &barrier);

			if (mipWide > 1)  mipWide /= 2;
			if (mipHigh > 1)  mipHigh /= 2;

			barrier.subresourceRange.baseMipLevel = iLevel;
			barrier.oldLayout	= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout	= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask	= VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask	= VK_ACCESS_TRANSFER_READ_BIT;
		}

		barrier.newLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.dstAccessMask	  = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
							 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
							 0, nullptr,
							 0, nullptr,
							 1, &barrier);

		endAndSubmitCommands(commandBuffer);
	}
};

#endif	// Mipmaps_h
