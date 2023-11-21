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
		debugReport(vulkan),					//	See PROGRAMMER NOTE below.
		windowSurface(vulkan, platform),
		device(windowSurface, vulkan, validation),
		swapchain(device, windowSurface),
		depthBuffer(swapchain, device),
		renderPass(device),
		framebuffers(swapchain, depthBuffer, renderPass, device),
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


void VulkanSetup::RecreateRenderingResources()
{
	vkDeviceWaitIdle(device.getLogical());

	swapchain.Recreate();
	depthBuffer.Recreate(swapchain);
	framebuffers.Recreate(swapchain, depthBuffer, renderPass);

	// note, not needing re-creation: commandPool, syncObjs, renderPass, device, window

	command.RecreateRenderables(*this);
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
