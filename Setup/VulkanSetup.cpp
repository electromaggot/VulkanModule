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


VulkanSetup::VulkanSetup(iPlatform& platform)
	:	validation(),							// Initializer list: instantiate components
		vulkan(validation, platform),			//	in ascending order of explicit dependencies.
		debugReport(vulkan),
		windowSurface(vulkan, platform),
		device(windowSurface, vulkan, validation),
		renderPass(device),
		swapchain(device, windowSurface),
		framebuffers(swapchain, renderPass, device),
		syncObjects(device),
		command(* new CommandControl(framebuffers, device))		// initialize CommandPool
{
	Log(NOTE, "----V-U-L-K-A-N---R-E-A-D-Y----");
}

// On the other hand, these later child objects will NOT leave scope and self-destruct,
//	so:
VulkanSetup::~VulkanSetup()
{
	delete &command;
}


void VulkanSetup::RecreateRenderingRudiments()
{
	vkDeviceWaitIdle(device.getLogical());

	swapchain.Recreate();
	framebuffers.Recreate(swapchain, renderPass);

	// note, not needing re-creation: commandPool, syncObjs, renderPass, device, window

	command.RecreateRenderables(*this);
}
