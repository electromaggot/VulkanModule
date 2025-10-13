//
// ShadowMap.h
//	VulkanModule Adjunct
//
// Manages shadow mapping resources: depth framebuffer, render pass, and shadow map texture.
// Provides infrastructure for rendering scene from light's perspective to generate shadow maps.
// Note: Does not derive from BufferBase (which is VkBuffer-specific); creates VkImage instead.
//
// Created 3 Oct 2025 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ShadowMap_h
#define ShadowMap_h

#include "VulkanPlatform.h"

class GraphicsDevice;
class CommandPool;


class ShadowMap
{
public:
	ShadowMap(GraphicsDevice& graphicsDevice, CommandPool& commandPool,
			  uint32_t shadowMapWidth, uint32_t shadowMapHeight);
	~ShadowMap();

	void Recreate();

	// Getters
	VkImageView getImageView() const { return shadowImageView; }
	VkSampler getSampler() const { return shadowSampler; }
	VkRenderPass getRenderPass() const { return shadowRenderPass; }
	VkFramebuffer getFramebuffer() const { return shadowFramebuffer; }
	VkImage getImage() const { return shadowImage; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }

private:
	void createShadowImage();
	void createShadowImageView();
	void createShadowSampler();
	void createShadowRenderPass();
	void createShadowFramebuffer();
	void destroy();

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	GraphicsDevice& device;
	CommandPool& commandPool;

	uint32_t width;
	uint32_t height;

	VkImage shadowImage;
	VkDeviceMemory shadowImageMemory;
	VkImageView shadowImageView;
	VkSampler shadowSampler;
	VkRenderPass shadowRenderPass;
	VkFramebuffer shadowFramebuffer;

	VkFormat depthFormat;
};

#endif // ShadowMap_h
