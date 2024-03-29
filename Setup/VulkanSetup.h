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

class CommandControl;


enum SteerSetup {	// minor tailoring of the setup process
	BASIC			= 0,
	NO_DEPTH_BUFFER = 0b00000001
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
};

#endif	// VulkanSetup_h
