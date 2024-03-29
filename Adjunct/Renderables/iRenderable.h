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
// 3/26/20 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef iRenderable_h
#define iRenderable_h

#include "VulkanPlatform.h"

#include "GraphicsPipeline.h"

#include "AddOns.h"
#include "DrawableSpecifier.h"


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

	virtual bool Update(GameClock& time) { return false; }	// Be sure to override if re-recording command buffers each frame!

	virtual void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex = 0) = 0;

	virtual void Recreate(VulkanSetup& vulkan, bool reloadMesh = false) { }
};
//
// Encapsulates a rendered-object's Draw commands, but also its Vulkan components like Pipeline.
//	...but not command Buffer details like how often it has to re-record (see CommandRecordable for that).
//
struct iRenderable : iRenderableBase
{
	iRenderable(DrawableSpecifier& specified, VulkanSetup& vulkan, iPlatform& platform)
		:	shaderModules(	* new ShaderModules(specified.shaders, vulkan.device)),
			addOns(			* new AddOns(specified, vulkan, platform)),
			descriptors(	* new Descriptors(addOns.described, vulkan.swapchain, vulkan.device)),
			pipeline(		* new GraphicsPipeline(shaderModules, vulkan.renderPass, vulkan.swapchain, vulkan.device,
												   &specified.mesh.vertexType, &descriptors, specified.customize)),
			vertexObject(	specified.mesh),
			customizer(		specified.customize),
			name(			specified.name),
			updateMethod(	specified.updateMethod)
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

	MeshObject&			vertexObject;	// (retain for Recreate)
	Customizer			customizer;
	string&				name;
	bool				(*updateMethod)(GameClock&);


	virtual iRenderable* newConcretion(CommandRecording* pRecordingMode) const = 0;
	virtual void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex = 0) = 0;

	virtual bool Update(GameClock& time)
	{
		if (updateMethod)
			return updateMethod(time);	// (see Renderables.Update() below)
		return false;
	}

	virtual void Recreate(VulkanSetup& vulkan, bool reloadMesh = false)
	{
		// also reload shaderModules if different shader(s) specified?

		if (reloadMesh)		// otherwise keep the same AddOns (e.g. VertexBuffer) we already have loaded
		{
			addOns.Recreate(vertexObject);
			addOns.RecreateDescribables();
			descriptors.Recreate(addOns.reDescribe(), vulkan.swapchain);
		}

		pipeline.Recreate(shaderModules, vulkan.renderPass, vulkan.swapchain, &vertexObject.vertexType, &descriptors, customizer);
	}
};


// A CommandBuffer object needs an array of Renderables that go into recording its VkCommandBuffer.
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
		Clear();	// (in case one or more objects are self-managed, don't blanket: delete[] pRenderable;)
	}

	DrawableSpecifier* ALL = nullptr;

	void Clear() {
		Remove(ALL);
	}

	void Remove(DrawableSpecifier* pObjSpec)
	{
		bool removeALL = pObjSpec == ALL;
		for (auto& recordable : recordables) {
			auto& renderables = recordable.pRenderables;
			for (auto ppRenderable = renderables.begin();
					  ppRenderable < renderables.end(); ++ppRenderable) {		// Must ITERATE over vector...
				iRenderable& renderable = **ppRenderable;
				if ((!removeALL && &renderable.vertexObject == &pObjSpec->mesh)
				  || (removeALL && !renderable.isSelfManaged)) {
					renderable.deleteConcretion();
					renderables.erase(ppRenderable);							//	...in order to .erase().
					return;//on 1st match. To never iterate past .end().
				}			// Also, don't:  delete *ppRenderable;
			}				//	since it came in as a reference.
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
		Log(RAW, "done: %s SPAWNED.", pRenderable->name.c_str());
	}


	// This is where all Renderables get their Update() methods called.  It is optional, if a Renderable doesn't
	//	move, animate, or otherwise change.  This is custom-set per Renderable and is separate from any gxActions
	//	that may have also been applied to the Renderable.
	//	Returning true indicates overall Update "succeeded" and requests caller to refresh, because
	//	at least one Renderable's Update() requested the refresh.
	//
	bool Update(GameClock& time)
	{
		bool result, requestRefresh = false;
		for (CommandRecordable& recordable : recordables)
			for (iRenderable* pRenderable : recordable.pRenderables) {
				result = pRenderable->Update(time);
				requestRefresh = result || requestRefresh;
			}
		return requestRefresh;
	}

	void UpdateUniformBuffers(int iNextImage)
	{
		for (auto& recordable : recordables)
			for (auto& pRenderable : recordable.pRenderables) {
				if (!pRenderable->isSelfManaged) {
					AddOns& addOns = pRenderable->addOns;
					if (addOns.described.size() > 0)
						for (int index = 0; index < addOns.pUniformBuffers.size(); ++index) {
							UniformBuffer& unibuf = *addOns.pUniformBuffers[index];
							void* pUboData = addOns.ubos[index].pBytes;
							size_t numBytes = addOns.ubos[index].byteSize;
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
