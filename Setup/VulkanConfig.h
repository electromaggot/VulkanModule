//
// VulkanConfig.h
//	Vulkan Setup
//
// Per-application settings this module needs, owned BY the module rather than reached up for.
//
// Historically these values were read directly from an `AppConstants.h` that each consuming
//	application had to supply, which inverted the dependency: a reusable library reaching up
//	into its consumer for configuration.  Every field below is defaulted, so an application
//	that supplies nothing still builds and runs; supply a VulkanConfig to override.
//
//	Application usage:
//		VulkanConfig config;
//		config.appName		= "MyApp";
//		config.windowTitle	= "My Window";
//		PlatformSDL platform(config);		// platform publishes it module-wide
//		VulkanSetup vulkan(platform);		// reads it back via platform.Config()
//
//	The application owns the VulkanConfig and must keep it alive for the platform's lifetime
//	(publishing stores a reference, not a copy) -- typically a member alongside the platform.
//
// Created 8/13/26 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef VulkanConfig_h
#define VulkanConfig_h

#include "VulkanPlatform.h"


struct VulkanConfig
{
		// IDENTITY - reported to Vulkan by VulkanInstance
	StrPtr		appName				= "VulkanApp";
	uint32_t	appVersion			= VK_MAKE_VERSION(1, 0, 0);

		// PER-USER STORAGE & DIAGNOSTICS
	StrPtr		companyName			= "Unspecified";
	StrPtr		projectName			= "Unspecified";	// these two resolve the per-user path
	StrPtr		debugLogFileName	= "DebugLog.txt";
	StrPtr		exePath				= "";		// the running executable, usually argv[0];
													//	reported in the startup log.  Empty
													//	rather than null: it reaches printf.

		// WINDOW - initial size and title, before any saved geometry is restored over them
	StrPtr		windowTitle			= "Vulkan";
	int			defaultWindowWidth	= 1280;
	int			defaultWindowHeight	= 1024;

		// RENDERING
	VkClearColorValue clearColor	= { { 0.0f, 0.0f, 0.0f, 1.0f } };	// black
	bool		supportStereo3D		= false;
};


// Module-wide access, for the code that runs before (or without) any platform object exists:
//	Logging's free functions and FileSystem's static path resolution.  The platform publishes
//	on construction; until then -- and in any application that never supplies one -- the
//	defaults above apply.  Prefer iPlatform::Config() wherever a platform is in reach.
//
const VulkanConfig&	TheVulkanConfig();

void publishVulkanConfig(const VulkanConfig& config);


#endif	// VulkanConfig_h
