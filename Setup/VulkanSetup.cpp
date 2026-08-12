//
// VulkanSetup.cpp
//	Vulkan Module
//
// See matched header file for definitive main comment.
//
// Note that ValidationLayers and DebugReport are initialized as early as possible
//	to enable and begin, especially initialization-related, debug reporting.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "VulkanSetup.h"
#include "CommandObjects.h"


VulkanSetup::VulkanSetup(iPlatform& platform,
						 SteerSetup directive)
	:	validation(),							// Initializer list: instantiate components
		vulkan(validation, platform),			//	in ascending order of explicit dependencies.
		debugReport(vulkan),					//	See PROGRAMMER NOTE below.
		windowSurface(vulkan, platform),
		device(windowSurface, vulkan, validation),
		swapchain(device, windowSurface),
		depthBuffer(swapchain, device, !(directive & NO_DEPTH_BUFFER)),
		renderPass(device),
		framebuffers(swapchain, depthBuffer, renderPass, device),
		syncObjects(device),
		command(* new CommandControl(framebuffers, device))		// initialize CommandPool
{
	Log(GOAL, "----V-U-L-K-A-N---R-E-A-D-Y----");
}

// On the other hand, these later child objects will NOT leave scope and self-destruct,
//	so:
VulkanSetup::~VulkanSetup()
{
	delete &command;
}


void VulkanSetup::RecreateRenderingResources()
{
	vkDeviceWaitIdle(device.getLogical());

	swapchain.Recreate();
	depthBuffer.Recreate(swapchain);
	framebuffers.Recreate(swapchain, depthBuffer, renderPass);

	// note, not needing re-creation: commandPool, syncObjs, renderPass, device, window

	command.RecreateRenderables(*this);
}


FrameRecovery VulkanSetup::RecoverFromPresentResult(VkResult result)
{
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {		// Window resized /
		RecreateRenderingResources();											//	moved monitors — rebuild
		syncObjects.Recreate();													//	the swapchain chain.
		return FrameRecovery::Recreated;
	}

	if (result == VK_ERROR_DEVICE_LOST)		// Logical device gone (e.g. GPU reset after sleep).  Do NOT
		return FrameRecovery::DeviceLost;	//	rebuild here — vkCreateSwapchainKHR aborts on a dead
											//	device.  The app decides how to recover (e.g. relaunch).

	return FrameRecovery::None;		// VK_SUCCESS, or unhandled (e.g. VK_TIMEOUT) — caller logs context.
}


// Phase 1 of two-phase recovery — call at macOS WillSleep, while the device is STILL VALID, so the
//	teardown (incl. vkDestroyDevice) is clean (no UB).  The app's destroyAppResources MUST destroy
//	EVERY device child it owns first, else vkDestroyDevice is undefined (validation layers will flag
//	exactly which).  Pairs with RebuildAfterSleep at DidWake.
void VulkanSetup::TeardownForSleep(const std::function<void()>& destroyAppResources)
{
	Log(WARN, "Sleep: tearing down GPU resources (device still valid).");

	vkDeviceWaitIdle(device.getLogical());

	if (destroyAppResources)				// App destroys ITS device children (renderables, UBOs,
		destroyAppResources();				//	shadow, ImGui backend, font atlas, particles, textures).

	command.Destroy();						// CPU buffer-set array (VkCommandBuffers freed with the pool)
	command.getCommandPool().destroy();
	syncObjects.destroy();
	framebuffers.destroy();
	renderPass.destroy();
	depthBuffer.destroy();
	swapchain.destroy();

	device.DestroyLogicalDevice();			// Clean: all children gone, device still valid.
}

// Phase 2 — call at macOS DidWake.  Recreates the logical device + VulkanModule device-level objects,
//	then the app rebuilds its GPU resources, and command buffers are re-recorded.
void VulkanSetup::RebuildAfterSleep(const std::function<void()>& rebuildAppResources)
{
	Log(WARN, "Wake: recreating GPU resources on new device...");

	device.createLogicalDevice(validation);

	command.getCommandPool().create();
	swapchain.Recreate();					// Recreate() = destroy() (no-op on nulled handle) + create().
	depthBuffer.Recreate(swapchain);
	renderPass.Recreate();
	framebuffers.Recreate(swapchain, depthBuffer, renderPass);
	syncObjects.Recreate();
	command.Create(framebuffers);

	if (rebuildAppResources)
		rebuildAppResources();

	command.PostInitPrepBuffers(*this);

	Log(GOAL, "Wake: GPU resources rebuilt — rendering resumed.");
}

// Reactive single-call recovery (secondary safety net for device loss with no sleep warning).
//	NOTE: when the device is already lost, the teardown's vkDestroyDevice is UB — prefer the
//	WillSleep/DidWake two-phase path above, which tears down while the device is still valid.
void VulkanSetup::RecoverFromDeviceLoss(const std::function<void()>& destroyAppResources,
										const std::function<void()>& rebuildAppResources)
{
	TeardownForSleep(destroyAppResources);		// Destroys children + device.
	RebuildAfterSleep(rebuildAppResources);		// Recreates device + children + app resources.
}


/* PROGRAMMER NOTE
The Initializer List of Vulkan Objects passes along REFERENCES to previously made objects.
 Always make sure those are received as REFERENCES, and be careful if they are stored for
 access later, that they are STORED AS REFERENCES and that those target objects persist.
 If you make a mistake or forget to mark your Reference& with an &mpersand, note that an
 instance of the object may be (shallow) copied instead, and once your containing object
 destroys, that Vulkan Object may be destroyed too.  So if you ever see an error like:
	libc++abi: terminating with uncaught exception of type
							std::__1::system_error: mutex lock failed: Invalid argument
 that may just be the reason.
*/
