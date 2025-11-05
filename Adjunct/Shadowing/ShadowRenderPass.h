//
// ShadowRenderPass.h
//	Vulkan Setup
//
// Encapsulate the process to initialize/create Shadow-specific VkRenderPass.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ShadowRenderPass_h
#define ShadowRenderPass_h

#include "iRenderPass.h"
#include "GraphicsDevice.h"
#include "Swapchain.h"


class ShadowRenderPass : public iRenderPass
{
		// XSTRUCT
public:
	ShadowRenderPass(GraphicsDevice& graphicsDevice, VkFormat depthFormat);
	~ShadowRenderPass();

	void Recreate();

		// MEMBERS
private:
	VkRenderPass renderPass;

	VkDevice&	 device;
				 // Save to destruct as constructed.
	VkFormat	 depthFormat;

		// METHODS
private:
	void	create();
	void	destroy();

		// getters
public:
	VkRenderPass& getVkRenderPass()	{ return renderPass; }

	bool isDepthBufferUsed()		{ return true; }
	bool hasColorAttachment()		{ return false; }
};

#endif	// ShadowRenderPass_h
