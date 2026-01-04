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
#include "PrimitiveBuffer.h"	// for updateVertexData() inline method


// A CommandBuffer object needs an array of Renderables that go into recording its VkCommandBuffer, with
//	instructions how to draw them.  Originally we thought it was important "how often" it gets recorded, but
//	as it turns out: 1) the buffer almost always needs to be rerecorded anyway, and 2) it's not very costly.
// Therefore this enum will be retired, especially since option 1 is now done via Secondary CommandBuffer.
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
	virtual ~iRenderableBase() = default;			// Virtual destructor ensures proper cleanup of derived classes

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
	iRenderable(DrawableSpecifier& specified, VulkanSetup& vulkan, iPlatform& platform,
				iRenderPass* pCustomRenderPass = nullptr, VkExtent2D customExtent = { 0, 0 })
		:	shaderModules(	specified.pSharedShaderModules ? *specified.pSharedShaderModules
													: * new ShaderModules(specified.shaders, vulkan.device)),
			addOns(			* new AddOns(specified, vulkan, platform)),
			descriptors(	* new Descriptors(addOns.described, vulkan.swapchain, vulkan.device)),
			pipeline(		* new GraphicsPipeline(shaderModules,
												   (pCustomRenderPass != nullptr) ? *pCustomRenderPass : vulkan.renderPass,
												   vulkan.swapchain, vulkan.device, &specified.mesh.vertexType,
												   &descriptors, specified.customize, customExtent)),
			vertexObject(	specified.mesh),
			customizer(		specified.customize),
			name(			specified.name),
			updateMethod(	specified.updateMethod),
			ownsShaderModules(!specified.pSharedShaderModules),
			pass(			specified.pass)
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
		if (ownsShaderModules)
			delete(&shaderModules);
	}


	ShaderModules&		shaderModules;
	AddOns&				addOns;
	Descriptors&		descriptors;
	GraphicsPipeline&	pipeline;

	MeshObject&			vertexObject;	// (retain for Recreate)
	Customizer			customizer;
	string				name;
	bool				(*updateMethod)(GameClock&);
	bool				ownsShaderModules;	// true if we created it, false if shared
	const char*			pass;				// Render pass type (nullptr for primary, or "transparency"/"lines"/"shadow")

	// Dynamic UBO support for efficient per-object transforms.
	uint32_t			dynamicOffset = 0;
	bool				hasDynamicOffset = false;


	virtual iRenderable* newConcretion(CommandRecording* pRecordingMode) const = 0;
	virtual void IssueBindAndDrawCommands(VkCommandBuffer& commandBuffer, int bufferIndex = 0) = 0;

	// Check if this renderable uses secondary command buffers.
	virtual bool IsSecondaryCommandBuffer() const { return false; }

	// Update vertex buffer with new data.  For dynamic geometry: animated models, particles, waveforms...
	//	CRITICAL: New data must be same size as original buffer, no reallocation.
	//	Uses direct CPU memory mapping (host-visible buffers) → NO command buffers, extremely fast!
	//	Industry-standard for 60fps dynamic geometry updates.
	void updateVertexData(void* pNewVertexData, VkDeviceSize size)
	{
		if (addOns.pVertexBuffer)
			addOns.pVertexBuffer->UpdateVertexBufferMapped(pNewVertexData, size);
	}

	// Get secondary command buffer for a specific frame (only valid if IsSecondaryCommandBuffer() == true).
	virtual VkCommandBuffer GetSecondaryCommandBuffer(int frameIndex) const { return VK_NULL_HANDLE; }

	// Update uniform buffers for this renderable.
	void UpdateUniformBuffers(int iNextImage)
	{
		for (int index = 0; index < addOns.pUniformBuffers.size(); ++index) {
			// Skip dynamic UBOs (updated separately via DynamicUniformBuffer)
			if (addOns.pUniformBuffers[index] == nullptr)
				continue;
			UniformBuffer& unibuf = *addOns.pUniformBuffers[index];
			void* pUboData = addOns.ubos[index].pBytes;
			size_t numBytes = addOns.ubos[index].byteSize;
			unibuf.Update(iNextImage, pUboData, numBytes);
		}
	}

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


// Renderables management class - stores normal and self-managed renderables in separate type-safe vectors.
//
class Renderables
{
	friend class CommandControl;

public:
	~Renderables()
	{
		Clear();
	}

	DrawableSpecifier* ALL = nullptr;

	void Clear() {
		Remove(ALL);
	}

	// Remove renderables - simplified with type-safe vectors: no casting, no defensive checks.
	//
	void Remove(DrawableSpecifier* pObjSpec)
	{
		bool removeALL = (pObjSpec == ALL);

		// Remove from normal renderables (type-safe iteration).
		for (auto it = pNormalRenderables.begin(); it != pNormalRenderables.end(); ) {
			iRenderable* pRenderable = *it;

			if (removeALL || &pRenderable->vertexObject == &pObjSpec->mesh) {
				Log(GOOD, "Removing: %s", pRenderable->name.c_str());
				pRenderable->deleteConcretion();
				delete pRenderable;
				it = pNormalRenderables.erase(it);
				if (!removeALL)
					return;
			} else {
				++it;
			}
		}
		// Self-managed renderables are NEVER removed; they manage own lifecycle.
	}

	// Add methods - public API unchanged for backward compatibility.
	//
	void Add(const iRenderableBase& renderable)
	{
		Add((iRenderableBase*) &renderable, UPON_EACH_FRAME);
	}

	void Add(const iRenderable& renderable)
	{
		CommandRecording recordingMode;
		iRenderable* pRenderable = renderable.newConcretion(&recordingMode);
		Add(pRenderable, recordingMode);
	}

	// Update all renderables, both normal and self-managed.
	//	That is, this is where all Renderables get their Update() methods called.  It is optional, if a Renderable
	//	doesn't move, animate, or otherwise change.  This is custom-set per Renderable and is separate from gxActions
	//	that may have also been applied to the Renderable.  Returning true indicates overall Update "succeeded" and
	//	requests caller to refresh, because at least one Renderable's Update() requested the refresh.
	//
	bool Update(GameClock& time)
	{
		bool requestRefresh = false;

		for (iRenderable* pRenderable : pNormalRenderables) {			// Update normal renderables.
			bool result = pRenderable->Update(time);
			requestRefresh = result || requestRefresh;
		}

		for (iRenderableBase* pRenderable : pSelfManagedRenderables) {	// Update self-managed renderables.
			bool result = pRenderable->Update(time);
			requestRefresh = result || requestRefresh;
		}
		return requestRefresh;
	}

	// Update uniform buffers; only normal renderables have UBOs.
	//
	void UpdateUniformBuffers(int iNextImage)
	{
		for (iRenderable* pRenderable : pNormalRenderables) {
			AddOns& addOns = pRenderable->addOns;
			if (addOns.described.size() > 0) {
				for (int index = 0; index < addOns.pUniformBuffers.size(); ++index) {
					if (addOns.pUniformBuffers[index] == nullptr)
						continue;
					UniformBuffer& unibuf = *addOns.pUniformBuffers[index];
					void* pUboData = addOns.ubos[index].pBytes;
					size_t numBytes = addOns.ubos[index].byteSize;
					assert(numBytes == unibuf.nbytesBufferObject);
					unibuf.Update(iNextImage, pUboData, numBytes);
				}
			}
		}
	}

	// Recreate all renderables (for window resize).
	//
	void Recreate(VulkanSetup& vulkan)
	{
		for (iRenderable* pRenderable : pNormalRenderables)
			pRenderable->Recreate(vulkan);

		for (iRenderableBase* pRenderable : pSelfManagedRenderables)
			pRenderable->Recreate(vulkan);
	}

	// Getters for CommandControl to merge vectors.
	const vector<iRenderable*>& getNormalRenderables() const { return pNormalRenderables; }
	const vector<iRenderableBase*>& getSelfManagedRenderables() const { return pSelfManagedRenderables; }
	size_t getNormalCount() const { return pNormalRenderables.size(); }
	size_t getSelfManagedCount() const { return pSelfManagedRenderables.size(); }

private:
	// Typed vectors - eliminates type confusion and crashes
	vector<iRenderable*> pNormalRenderables;
	vector<iRenderableBase*> pSelfManagedRenderables;

	// Internal routing method - single isSelfManaged check at add-time.
	//
	void Add(iRenderableBase* pRenderable, CommandRecording recordingMode)
	{
		if (pRenderable->isSelfManaged)
			addSelfManaged(pRenderable);
		else
			addNormal(static_cast<iRenderable*>(pRenderable), recordingMode);
	}

	void addNormal(iRenderable* pRenderable, CommandRecording recordingMode)
	{
		pNormalRenderables.push_back(pRenderable);

		if (pRenderable->pass == nullptr)
			Log(RAW, "done: %s SPAWNED.", pRenderable->name.c_str());
		else
			Log(RAW, "done: %s BOUND.", pRenderable->name.c_str());
	}

	void addSelfManaged(iRenderableBase* pRenderable)
	{
		pSelfManagedRenderables.push_back(pRenderable);
		Log(RAW, "done: Self-managed renderable added.");
	}
};


#endif	// iRenderable_h
