//
// RenderBatch.cpp
//	VulkanModule AddOns
//
// See header file comment for overview.
//
// Tadd Jensen 9 Nov 2023
//	© 0000 (uncopyrighted; use at will)
//
#include "RenderBatch.h"
#include "Renderable.h"


vector<iRenderableBase*> RenderBatchManager::buildBatches(const vector<iRenderableBase*>& renderables)
{
	clear();

	vector<iRenderableBase*> selfManagedRenderables;

	if (renderables.empty())
		return selfManagedRenderables;

	// Group renderables by pass type AND pipeline using a map for efficient lookups;
	//	preserves correct rendering order: opaque (nullptr) → transparent → lines
	// Collect self-managed renderables separately (like ImGui) to render last.
	std::map<PipelineKey, vector<iRenderableBase*>> pipelineGroups;

	for (auto* pRenderable : renderables) {
		// Separate self-managed renderables - things like ImGui render last with their own state.
		if (pRenderable->isSelfManaged) {
			selfManagedRenderables.push_back(pRenderable);
			continue;
		}

		// Cast to iRenderable to access pipeline and pass, available in all non-self-managed renderables.
		iRenderable* renderable = static_cast<iRenderable*>(pRenderable);

		PipelineKey key = { renderable->pass, renderable->pipeline.getVkPipeline() };
		pipelineGroups[key].push_back(pRenderable);
	}

	// Convert map to vector of batches:
	batches.reserve(pipelineGroups.size());
	for (auto& [key, renderableList] : pipelineGroups) {
		RenderableBatch batch;
		batch.key = key;
		batch.renderables = std::move(renderableList);
		batches.push_back(std::move(batch));
	}
	return selfManagedRenderables;
}

void RenderBatchManager::recordBatches(VkCommandBuffer& commandBuffer, int bufferIndex,
									   const vector<iRenderableBase*>& selfManagedRenderables)
{
	VkPipeline lastBoundPipeline = VK_NULL_HANDLE;

	// First: Record all batched renderables (scene objects).
	for (const auto& batch : batches) {
		// Bind pipeline once per batch, not per renderable.
		if (batch.key.pipeline != lastBoundPipeline) {
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, batch.key.pipeline);
			lastBoundPipeline = batch.key.pipeline;
		}

		// Draw all renderables in this batch.
		for (auto* pRenderable : batch.renderables) {
			// Self-managed renderables should not be in batches, but check just in case.
			if (pRenderable->isSelfManaged) {
				continue;  // Skip self-managed renderables in batched rendering.
			}

			// Safe to cast to iRenderable* now; we've confirmed it's not self-managed.
			iRenderable* renderable = static_cast<iRenderable*>(pRenderable);

			// Check if this is a secondary command buffer renderable.
			if (renderable->IsSecondaryCommandBuffer()) {	// If so, execute the pre-recorded command buffer.
				VkCommandBuffer secondaryCmdBuf = renderable->GetSecondaryCommandBuffer(bufferIndex);
				vkCmdExecuteCommands(commandBuffer, 1, &secondaryCmdBuf);
			} else {	// Otherwise, cast to Renderable for normal batched rendering.
				Renderable* concreteRenderable = static_cast<Renderable*>(renderable);
				// Skip pipeline bind since we already bound it once for the entire batch.
				concreteRenderable->IssueBindAndDrawCommands(commandBuffer, bufferIndex, true);
			}
		}
	}

	// Last: Record self-managed renderables (e.g. ImGui) after all batched renderables.
	//	These manage their own pipeline state and must render last.
	for (auto* pRenderable : selfManagedRenderables) {
		pRenderable->IssueBindAndDrawCommands(commandBuffer, bufferIndex);
	}
}

void RenderBatchManager::clear()
{
	batches.clear();
}
