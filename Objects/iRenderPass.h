//
// iRenderPass.h
//	Vulkan Setup
//
// Abstract interface for render pass implementations.
//	Allows Liskov substitution of different RenderPass types.
//
// 11/3/24 Created for interface abstraction
//	© 0000 (uncopyrighted; use at will)
//
#ifndef iRenderPass_h
#define iRenderPass_h

#include <vulkan/vulkan.h>


class iRenderPass
{
public:
	virtual ~iRenderPass() = default;

	// Pure virtual methods
	virtual VkRenderPass& getVkRenderPass() = 0;
	virtual void Recreate() = 0;

	virtual bool isDepthBufferUsed()  = 0;
	virtual bool hasColorAttachment() = 0;
};

#endif	// iRenderPass_h
