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

	// Comparison operator for std::map
	// Sort by pass first (preserves opaque→transparent→lines order), then by pipeline.
	bool operator<(const PipelineKey& other) const {
		// Explicit ordering: nullptr (opaque) < "transparency" < "lines"
		// This ensures correct depth ordering.
		int passOrder = getPassOrder(pass);
		int otherPassOrder = getPassOrder(other.pass);

		if (passOrder != otherPassOrder)
			return passOrder < otherPassOrder;

		return pipeline < other.pipeline;	// If passes are equal, compare pipelines.
	}

private:
	static int getPassOrder(const char* pass) {
		if (pass == nullptr)					return 0;	// Opaque first
		if (strcmp(pass, "transparency") == 0)	return 1;	// Transparent second
		if (strcmp(pass, "lines") == 0)			return 2;	// Lines last (always on top)
		return 1;											// Unknown passes with transparency
	}

	bool operator == (const PipelineKey& other) const {
		bool passEqual = (pass == other.pass) ||
						 (pass != nullptr && other.pass != nullptr && strcmp(pass, other.pass) == 0);
		return passEqual && (pipeline == other.pipeline);
	}
};


// A batch of renderables that share the same pipeline.
//
struct RenderableBatch
{
	PipelineKey key;
	vector<iRenderable*> renderables;
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
	vector<iRenderable*> buildBatches(const vector<iRenderable*>& renderables);

	// Record all batches into command buffer with optimized pipeline binding.
	//	selfManagedRenderables: Renderables that manage their own state record last.
	void recordBatches(VkCommandBuffer& commandBuffer, int bufferIndex,
					   const vector<iRenderable*>& selfManagedRenderables = {});

	void clear();			// Clear all batches - call before rebuilding.

	// Get number of batches - useful for performance monitoring.
	size_t getBatchCount() const { return batches.size(); }

private:
	vector<RenderableBatch> batches;
};

#endif	// RenderBatch_h
