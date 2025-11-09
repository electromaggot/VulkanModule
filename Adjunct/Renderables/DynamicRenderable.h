//
// DynamicRenderable.h
//	VulkanModule AddOns
//
// A "dynamic" renderable incorporates draw commands that record upon each frame,
//	as the way it's drawn continually changes, or it changes regularly enough
//	that it is simpler to rerecord it continually.
//
// 3/26/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef DynamicRenderable_h
#define DynamicRenderable_h

#include "iRenderable.h"


struct DynamicRenderable : public iRenderable
{
public:
	DynamicRenderable(DrawableSpecifier& renderable, VulkanSetup& vulkan, iPlatform& platform);

	iRenderable* newConcretion(CommandRecording* pRecordingMode) const
	{
		*pRecordingMode = UPON_EACH_FRAME;

		return new DynamicRenderable(*this);
	}

	void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex);

	virtual bool Update(GameClock& time)  { return true; }

	// Dynamic UBO support
	uint32_t dynamicOffset = 0;
	bool  hasDynamicOffset = false;
};

#endif	// DynamicRenderable_h
