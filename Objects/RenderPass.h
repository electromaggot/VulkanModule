//
// RenderPass.h
//	Vulkan Setup
//
// Encapsulate the process to initialize/create VkRenderPass.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef RenderPass_h
#define RenderPass_h

#include "iRenderPass.h"
#include "GraphicsDevice.h"
#include "Swapchain.h"


class RenderPass : public iRenderPass
{
		// XSTRUCT
public:
	RenderPass(GraphicsDevice& graphicsDevice);
	~RenderPass();

		// MEMBERS
private:
	VkRenderPass renderPass;

	GraphicsDevice&	 device;		// Save to destruct as constructed.

		// METHODS
private:
	void	create();
	void	create(VkFormat imageFormat, VkFormat depthFormat);

public:
	void	Recreate() override;
	void	destroy();		// Public for device-loss teardown; idempotent (nulls handle).

		// getters
	VkRenderPass& getVkRenderPass() override	{ return renderPass; }

	bool	hasColorAttachment() override		{ return true; }

	bool	isDepthBufferUsed() override		{ return useDepthBuffer; }

	bool	useDepthBuffer = false;	// treated as read-only (i.e.
};									//	set then not re-referenced)

#endif // RenderPass_h
