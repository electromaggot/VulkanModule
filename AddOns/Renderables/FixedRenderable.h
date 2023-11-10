//
// FixedRenderable.h
//	VulkanModule AddOns
//
// A "fixed" renderable has draw commands that only need to be recorded once, as the way
//	it's drawn doesn't change.  (This excludes something like a model-view-projection
//	UBO to change its 3D object's view angle, which applies separately.)
// Therefore, the CommandBuffer is recorded just after construction time... Not per each
//	frame, which would happen in the Update() method, here being thus empty.
//
// 3/26/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef FixedRenderable_h
#define FixedRenderable_h

#include "iRenderable.h"


struct FixedRenderable : public iRenderable
{
public:
	FixedRenderable(DrawableSpecifier& drawable, VulkanSetup& vulkan, iPlatform& platform);

	iRenderable* newConcretion(CommandRecording* pRecordingMode) const
	{
		*pRecordingMode = AT_INIT_TIME_ONLY;

		return new FixedRenderable(*this);
	}

	void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex);
};

#endif	// FixedRenderable_h
