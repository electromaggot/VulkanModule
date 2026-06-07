//
// SyncObjects.cpp
//	Vulkan Setup
//
// See matched header file for definitive main comment.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "SyncObjects.h"
#include "ResourceTracker.h"


SyncObjects::SyncObjects(GraphicsDevice& graphics)
	:	device(graphics.getLogical())
{
	createSyncObjects();
}

SyncObjects::~SyncObjects()
{
	destroySyncObjects();
}


void SyncObjects::createSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo = {
		.sType	= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0
	};

	VkFenceCreateInfo fenceInfo = {
		.sType	= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= VK_FENCE_CREATE_SIGNALED_BIT	// Start Fence in signaled state.
	};

	for (int iFrame = 0; iFrame < MaxFramesInFlight; ++iFrame)
	{
		call = vkCreateSemaphore(device, &semaphoreInfo, nullALLOC, &imageAvailableSemaphores[iFrame]);
		if (call == VK_SUCCESS) {
			call = vkCreateSemaphore(device, &semaphoreInfo, nullALLOC, &renderFinishedSemaphores[iFrame]);
			if (call == VK_SUCCESS) {
				call = vkCreateSemaphore(device, &semaphoreInfo, nullALLOC, &shadowCompleteSemaphores[iFrame]);
				if (call == VK_SUCCESS) {
					call = vkCreateFence(device, &fenceInfo, nullALLOC, &inFlightFences[iFrame]);
					if (call == VK_SUCCESS)
						continue;
				}
			}
		}
		Fatal("Create synchronization object for frame " + to_string(iFrame) + " FAILURE" + ErrStr(call));
	}
}

void SyncObjects::destroySyncObjects()
{
	// Idempotent for device-loss teardown: guard each vkDestroy on a non-null handle, not just null the
	//	handle afterward — vkDestroy(NULL) is a spec no-op but the resource tracker still counts it, so a
	//	following Recreate()'s destroy() would otherwise inflate the destroyed total (false leak/imbalance).
	for (size_t iFrame = 0; iFrame < MaxFramesInFlight; ++iFrame) {
		if (inFlightFences[iFrame] != VK_NULL_HANDLE) {
			vkDestroyFence(device, inFlightFences[iFrame], nullALLOC);
			inFlightFences[iFrame] = VK_NULL_HANDLE;
		}
		if (renderFinishedSemaphores[iFrame] != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, renderFinishedSemaphores[iFrame], nullALLOC);
			renderFinishedSemaphores[iFrame] = VK_NULL_HANDLE;
		}
		if (shadowCompleteSemaphores[iFrame] != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, shadowCompleteSemaphores[iFrame], nullALLOC);
			shadowCompleteSemaphores[iFrame] = VK_NULL_HANDLE;
		}
		if (imageAvailableSemaphores[iFrame] != VK_NULL_HANDLE) {
			vkDestroySemaphore(device, imageAvailableSemaphores[iFrame], nullALLOC);
			imageAvailableSemaphores[iFrame] = VK_NULL_HANDLE;
		}
	}
}


void SyncObjects::Recreate()
{
	destroySyncObjects();
	createSyncObjects();
}
