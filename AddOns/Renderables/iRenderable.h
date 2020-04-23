//
// iRenderable.h
//	VulkanModule AddOns
//
// CommandBuffer encloses a RenderPass...
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

#include "GraphicsPipeline.h"

#include "AddOns.h"
#include "VertexBasedObject.h"
#include "UniformBufferLiterals.h"
#include "UniformBuffer.h"
#include "TextureImage.h"


struct Renderable {
	Shaders				shaders;
	VertexBasedObject&	vertexSpec;
	vector<UBO>			pUBOs;
	vector<TextureSpec>	textureSpecs;
};


enum CommandRecording {		// i.e. Request this CommandBuffer to be recorded:
	AT_INIT_TIME_ONLY,		//  - once at initialization-time and not re-recorded before frames.
	UPON_EACH_FRAME,		//  - repeatedly on every frame, and Reset before the next one.
	ON_CHANGE_FLAGGED		//	- only upon boolean set indicating change.
};


struct iRenderableBase
{
//protected:	//TODO: make these both classes (structs are too open)
	bool	isSelfManaged;	// if renderable object independently stores/handles its own pipeline/shaders/vertices/etc.

	iRenderableBase() : isSelfManaged(true) { }		// (which defaults to true until specifically unset (see below))

	virtual iRenderableBase* newConcretion(CommandRecording* pRecordingMode) const = 0;

	virtual void Update(float deltaSeconds) { }		// Be sure to override if re-recording command buffers each frame!

	virtual void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex = 0) = 0;

	virtual void Recreate(VulkanSetup& vulkan, bool reloadMesh = false) { }
};
//
// Encapsulates a rendered-object's Draw commands, but also its Vulkan components like Pipeline.
//	...but not command Buffer details like how often it has to re-record (see CommandRecordable for that).
//
struct iRenderable : iRenderableBase
{
	iRenderable(Renderable& renderable, VulkanSetup& vulkan, iPlatform& platform)
		:	shaderModules(	* new ShaderModules(renderable.shaders, vulkan.device)),
			addOns(			* new AddOns(renderable, vulkan, platform)),
			descriptors(	* new Descriptors(addOns.described, vulkan.swapchain, vulkan.device)),
			pipeline(		* new GraphicsPipeline(shaderModules, vulkan.renderPass, vulkan.swapchain, vulkan.device,
												   &renderable.vertexSpec.vertexType, &descriptors)),
			vertexObject(	renderable.vertexSpec)
	{
		isSelfManaged = false;
	}

	virtual ~iRenderable()	// Destroys this interim object immediately after construction if not 'new'ed pointer,
	{ }						//	however the objects just-created above are referenced by the CONCRETION, so must persist
							//	until destroyed there... actually in Renderables::~Renderables() at bottom, which calls
	void deleteConcretion()	//	<-- this method for each concrete renderable object.
	{
		delete(&pipeline);
		delete(&descriptors);
		delete(&addOns);
		delete(&shaderModules);
	}


	ShaderModules&		shaderModules;
	AddOns&				addOns;
	Descriptors&		descriptors;
	GraphicsPipeline&	pipeline;

	VertexBasedObject&	vertexObject;	// (retain for Recreate)


	virtual iRenderable* newConcretion(CommandRecording* pRecordingMode) const = 0;
	virtual void Update(float deltaSeconds) { }
	virtual void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex = 0) = 0;

	virtual void Recreate(VulkanSetup& vulkan, bool reloadMesh = false)
	{
		// also reload shaderModules if different shader(s) specified?

		if (reloadMesh)		// otherwise keep the same AddOns (e.g. VertexBuffer) we already have loaded
		{
			addOns.Recreate(vertexObject);
			addOns.RecreateDescribables();
			descriptors.Recreate(addOns.reDescribe(), vulkan.swapchain);
		}

		pipeline.Recreate(shaderModules, vulkan.renderPass, vulkan.swapchain, &vertexObject.vertexType, &descriptors);
	}
};


// A CommandBuffer needs an array of Renderables that go into recording its VkCommandBuffer.
//	Also an indicator as to how often it gets (re)recorded.
//	Note that because one Buffer may share contributions from multiple Renderables, those must be
//	tracked until all their initializations complete, before the VkCommandBuffer can be recorded.
//
struct CommandRecordable {
	CommandRecording	 recordMode;
	vector<iRenderable*> pRenderables;
};


class Renderables
{
	friend class CommandControl;

public:
	~Renderables()
	{
		for (auto& recordable : recordables)
			for (auto& pRenderable : recordable.pRenderables) {
				if (!pRenderable->isSelfManaged) {
					pRenderable->deleteConcretion();
					delete pRenderable;
				}
			}
	}

	void Add(const iRenderableBase& renderable)
	{
		Add((iRenderable*) &renderable, UPON_EACH_FRAME);
	}
	void Add(const iRenderable& renderable)
	{
		CommandRecording recordingMode;
		iRenderable* pRenderable = renderable.newConcretion(&recordingMode);
		Add(pRenderable, recordingMode);
	}
	void Add(iRenderable* pRenderable, CommandRecording recordingMode)
	{
		size_t numRecordables = recordables.size();
		int iRecordable = -1;
		do {
			++iRecordable;
			if (iRecordable >= numRecordables) {
				recordables.emplace_back();
				recordables.back().recordMode = recordingMode;
				break;
			}
		} while (recordables[iRecordable].recordMode != recordingMode
				 && recordingMode != ON_CHANGE_FLAGGED);  // <-- always gets its own exclusive CommandBuffer

		recordables[iRecordable].pRenderables.push_back(pRenderable);
	}

	void Update(float deltaTime)
	{
		for (auto& recordable : recordables)
			for (auto& pRenderable : recordable.pRenderables)
				pRenderable->Update(deltaTime);
	}

	void UpdateUniformBuffers(int iNextImage)
	{
		for (auto& recordable : recordables)
			for (auto& pRenderable : recordable.pRenderables) {
				if (!pRenderable->isSelfManaged) {
					AddOns& addOns = pRenderable->addOns;
					if (addOns.described.size() > 0) {
						UniformBuffer& unibuf = *addOns.pUniformBuffer;
						void* pUboData = addOns.ubo.pBytes;
						size_t numBytes = addOns.ubo.byteSize;
						assert(numBytes == unibuf.nbytesBufferObject);
						unibuf.Update(iNextImage, pUboData, numBytes);
					}
				}
			}
	}

	void Recreate(VulkanSetup& vulkan)
	{
		for (auto& recordable : recordables)
			for (auto& pRenderable : recordable.pRenderables)
				pRenderable->Recreate(vulkan);
	}

private:
	vector<CommandRecordable> recordables;
};


#endif	// iRenderable_h
