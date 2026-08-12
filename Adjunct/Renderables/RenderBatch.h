//
// RenderBatch.h
//	VulkanModule AddOns
//
// Optimizes command buffer recording by grouping renderables that share the same pipeline.
//	Instead of binding a new pipeline for every renderable (expensive), we:
//		1. Group renderables by pipeline.
//		2. Bind pipeline once.
//		3. Draw all renderables using that pipeline.
//		4. Move to next pipeline group.
//
// This reduces pipeline binds from O(N renderables) to O(M unique pipelines), where M << N.
// For a scene with 100 objects using 5 different shaders, this reduces from 100 binds to 5.
//
// Tadd Jensen 9 Nov 2023
//	© 0000 (uncopyrighted; use at will)
//
#ifndef RenderBatch_h
#define RenderBatch_h

#include "VulkanPlatform.h"
#include "iRenderable.h"
#include <map>
#include <cstring>


// Key for grouping renderables by shared pipeline state AND render pass,
//	ensures correct depth ordering: opaque → transparent → lines
struct PipelineKey
{
	const char* pass;		// Render pass type (nullptr, "transparency", "lines")
	VkPipeline pipeline;
	int renderOrder = 0;	// Stable sort order within same pass (lower = rendered first)

	// Comparison operator for std::map
	// Sort by pass first (preserves opaque → transparent → lines order), then
	//	by renderOrder (stable, deterministic), then by pipeline (groups same shader).
	bool operator<(const PipelineKey& other) const {
		int passOrder = getPassOrder(pass);
		int otherPassOrder = getPassOrder(other.pass);

		if (passOrder != otherPassOrder)
			return passOrder < otherPassOrder;

		if (renderOrder != other.renderOrder)			// Stable tiebreaker within same pass.
			return renderOrder < other.renderOrder;

		return pipeline < other.pipeline;	// Groups renderables sharing the same pipeline.
	}

private:
							//TODO: This little static is now quite application-specific; be open to reassess approach.
	static int getPassOrder(const char* pass) {
		if (pass == nullptr)					return 3;	// Opaque fourth: Vehicle, objects - test against terrain depth.
		if (strcmp(pass, "skybox") == 0)		return 0;	// Skybox first: background.
		if (strcmp(pass, "floor") == 0)			return 1;	// Floor second: splitGrid renders before terrain.
		if (strcmp(pass, "terrain") == 0)		return 2;	// Terrain third: alpha-blends on top of floor.
		if (strcmp(pass, "lines") == 0)			return 4;	// Lines fifth: overlays on top of opaque geometry.
		if (strcmp(pass, "transparency") == 0)	return 5;	// Transparent last: alpha blending on top.
		return 5;											// Unknown passes render last, with transparency.
	}

	bool operator == (const PipelineKey& other) const {
		bool passEqual = (pass == other.pass) ||
						 (pass != nullptr && other.pass != nullptr && strcmp(pass, other.pass) == 0);
		return passEqual && (renderOrder == other.renderOrder) && (pipeline == other.pipeline);
	}
};


// A batch of renderables that share the same pipeline.
//
struct RenderableBatch
{
	PipelineKey key;
	vector<iRenderableBase*> renderables;
};


// Manage batching and optimized recording of renderables.
//
class RenderBatchManager
{
public:
	RenderBatchManager() = default;

	// Build batches from a list of renderables;
	//	maintains relative order within each batch for correct transparency rendering.
	// Returns self-managed renderables (like ImGui) that should be rendered last.
	vector<iRenderableBase*> buildBatches(const vector<iRenderableBase*>& renderables);

	// Record all batches into command buffer with optimized pipeline binding.
	//	selfManagedRenderables: Renderables that manage their own state record last.
	void recordBatches(VkCommandBuffer& commandBuffer, int bufferIndex,
					   const vector<iRenderableBase*>& selfManagedRenderables = {});

	void clear();			// Clear all batches - call before rebuilding.

	// Get number of batches - useful for performance monitoring.
	size_t getBatchCount() const { return batches.size(); }

private:
	vector<RenderableBatch> batches;
};

#endif	// RenderBatch_h
