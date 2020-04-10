//
// iRenderable.h
//	VulkanModule AddOns
//
// CommandBuffer encompasses a RenderPass...
//	1) ...which binds a Pipeline (which is assigned Shaders).
//	2) It then binds Descriptors, Buffers etc., and finally issues Draw commands.
// 1 and 2 comprise a "Renderable" of which many exist in a non-trivial Vulkan program.
// A Renderable defines its own Shaders.  If two or more objects-to-be-rendered
//	share the same shaders/pipeline, each should repeat step #2.
//	The idea is to pack as many Renderables into a single RenderPass, hence fill the
//	same CommandBuffer, which is then Submitted to the GPU all at once,
//	limiting/optimizing those Submits.
// Separate CommandBuffers/Submits may be required whenever an existing CommandBuffer
//	has not changed, so need not be re-recorded.  In that case, multiple Submits are
//	preferable to needlessly recording redundant commands.
//
// Created 3/26/20 by Tadd
//	© 2020 Megaphone Studios
//
#ifndef iRenderable_h
#define iRenderable_h

#include "VulkanPlatform.h"
#include "VulkanSetup.h"

#include "GraphicsPipeline.h"
#include "CommandObjects.h"

#include "VertexBasedObject.h"
#include "UniformBufferLiterals.h"
#include "UniformBuffer.h"
#include "TextureImage.h"
#include "CommandObjects.h"

#include "AddOns.h"


struct Renderable {
	Shaders				shaders;
	VertexBasedObject&	vertexSpec;
	vector<UBO>			pUBOs;
	vector<TextureSpec>	textureSpecs;
	RecordCommands		recordCommands;
};


struct iRenderable
{
	iRenderable(Renderable& renderable, VulkanSetup& vulkan, iPlatform& platform)
		:	shaderModules(	* new ShaderModules(renderable.shaders, vulkan.device)),
			addOns(			* new AddOns(renderable, vulkan, platform)),
			descriptors(	* new Descriptors(addOns.described, vulkan.swapchain, vulkan.device)),
			pipeline(		* new GraphicsPipeline(shaderModules, vulkan.renderPass, vulkan.swapchain, vulkan.device,
												   &renderable.vertexSpec.vertexType, &descriptors)),
			vertexObject(	renderable.vertexSpec),
			commandObjects(	* new CommandObjects(vulkan.framebuffers, vulkan.renderPass, vulkan.swapchain, vulkan.device))
	{ }

	virtual ~iRenderable() {
		delete(&commandObjects);
		delete(&pipeline);			//TJ_TODO: make sure these can't be destroyed
		delete(&descriptors);		//	automatically when iRenderable destroys.
		delete(&addOns);
		delete(&shaderModules);
	}


	ShaderModules&		shaderModules;
	AddOns&				addOns;
	Descriptors&		descriptors;
	GraphicsPipeline&	pipeline;
	CommandObjects&		commandObjects;

	VertexBasedObject&	vertexObject;	/// (retain for Recreate)


	virtual iRenderable* const newConcretion2(const iRenderable& abstraction) const = 0;

	virtual iRenderable* newConcretion() const = 0;


	virtual void Update(float deltaSeconds) { }		/// Be sure to override if re-recording command buffers each frame!

	virtual void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer) = 0;

	virtual void Recreate(VulkanSetup& vulkan, bool reloadMesh = false)
	{
		pipeline.Recreate(shaderModules, vulkan.renderPass, vulkan.swapchain, &vertexObject.vertexType, &descriptors);
		commandObjects.Recreate(vulkan.framebuffers, vulkan.renderPass, vulkan.swapchain, reloadMesh);
	}
};


class Renderables
{
public:
	~Renderables()
	{
		for (auto& pRenderable : pRenderables)
			delete pRenderable;
	}

	void Add(const iRenderable& renderable)
	{
		Add(renderable.newConcretion());
	}
	void Add(iRenderable* pRenderable)
	{
		pRenderables.push_back(pRenderable);
	}

	void Update(float deltaTime)
	{
		for (auto& pRenderable : pRenderables)
			pRenderable->Update(deltaTime);
	}

private:
	static vector<iRenderable*> pRenderables;
};


#endif	// iRenderable_h
