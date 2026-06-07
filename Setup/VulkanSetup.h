//
// VulkanVars.h
//	Vulkan Setup
//
// RAII (resource allocation is initialization) approach to setting-up
//	Vulkan's various components encapsulated in objects that initialize on
//	construction, incrementally accept dependencies, and release on destruction.
//
// 2/1/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef VulkanSetup_h
#define VulkanSetup_h

#include "iPlatform.h"
#include "VulkanInstance.h"
#include "ValidationLayers.h"
#include "DebugReport.h"
#include "GraphicsDevice.h"
#include "WindowSurface.h"
#include "Swapchain.h"
#include "DepthBuffer.h"
#include "RenderPass.h"
#include "Framebuffers.h"
#include "SyncObjects.h"
#include <functional>

class CommandControl;


enum SteerSetup {	// minor tailoring of the setup process
	BASIC			= 0,
	NO_DEPTH_BUFFER = 0b00000001
};


// Outcome of feeding a frame's acquire/present VkResult to RecoverFromPresentResult().
enum class FrameRecovery {
	None,		// VK_SUCCESS (or an unhandled result the caller should log itself).
	Recreated,	// Swapchain resources were rebuilt — caller must reset its per-image tracking.
	DeviceLost	// Logical device is gone (can't rebuild here) — caller owns the policy (e.g. relaunch).
};


class VulkanSetup
{
public:
	VulkanSetup(iPlatform& platform, SteerSetup setup = BASIC);
	~VulkanSetup();

	// Also public, as owner should have access to these:
	//	(literal "order of appearance" is significant)

	ValidationLayers	validation;
	VulkanInstance		vulkan;
	DebugReport	 		debugReport;
	WindowSurface		windowSurface;
	GraphicsDevice		device;
	Swapchain			swapchain;
	DepthBuffer			depthBuffer;
	RenderPass			renderPass;
	Framebuffers		framebuffers;
	SyncObjects			syncObjects;
	CommandControl&		command;


	void RecreateRenderingResources();

	// Reusable render-loop recovery policy.  Feed the latest acquire/present VkResult each frame:
	//	OUT_OF_DATE/SUBOPTIMAL → recreate the swapchain chain now (window resize / monitor move),
	//	recreating syncObjects too, and return Recreated (caller resets its own per-image tracking).
	//	DEVICE_LOST → return DeviceLost WITHOUT touching the swapchain: when the logical device is
	//	truly gone (e.g. a GPU reset after sleep) vkCreateSwapchainKHR fails on the dead device, so
	//	rebuilding here would abort.  Recovery from device loss is the app's policy (e.g. relaunch).
	FrameRecovery RecoverFromPresentResult(VkResult result);

	// In-process recovery from a lost logical device.  Tears down all device-level objects (after
	//	`destroyAppResources` lets the app release ITS GPU resources on the old device), recreates the
	//	logical device in place, recreates the device-level objects, then calls `rebuildAppResources`
	//	so the app rebuilds its GPU resources on the new device, and re-records command buffers.
	//	Instance, surface, validation, and the window all survive.
	void RecoverFromDeviceLoss(const std::function<void()>& destroyAppResources,
							   const std::function<void()>& rebuildAppResources);

	// Two-phase recovery driven by macOS sleep/wake (the robust path): TeardownForSleep at WillSleep
	//	(device still valid → clean teardown), RebuildAfterSleep at DidWake.  destroyAppResources must
	//	destroy every app-owned device child; rebuildAppResources recreates them on the new device.
	void TeardownForSleep(const std::function<void()>& destroyAppResources);
	void RebuildAfterSleep(const std::function<void()>& rebuildAppResources);
};

#endif	// VulkanSetup_h
