//
// FixedRenderable.h
//	VulkanModule AddOns
//
// Encapsulates a rendered-object's draw commands, but also its pipeline,
//	and... commandBuffer details like how often it has to re-record
// (TJ: The above seems abstract like should be in iRenderable.)
//
// A "fixed" renderable has draw commands that only have to be recorded once, as the way
//	it's drawn doesn't change.  (This excludes something like a model-view-projection
//	UBO to change its 3D object's view angle, which applies separately.)
// Therefore, the CommandBuffer is recorded at construction time.  Not per each frame,
//	which would happen in the Update() method, which here is thus empty.
//
// Created 3/26/20 by Tadd
//	© 2020 Megaphone Studios
//
#ifndef FixedRenderable_h
#define FixedRenderable_h

#include "iRenderable.h"


struct FixedRenderable : public iRenderable
{
public:
	FixedRenderable(Renderable& renderable, VulkanSetup& vulkan, iPlatform& platform);
	FixedRenderable(const iRenderable& base) : iRenderable(base) { }
	//~FixedRenderable();

	iRenderable* const newConcretion2(const iRenderable& abstraction) const {
		//FixedRenderable* pClone = new FixedRenderable(abstract.renderable, abstract.vulkan, abstract.platform);
		//new FixedRenderable(abstract);
		return new FixedRenderable(abstraction);
	}
	iRenderable* newConcretion() const
	{
		return new FixedRenderable(*this);
	}


	void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer);

};

#endif	// FixedRenderable_h
