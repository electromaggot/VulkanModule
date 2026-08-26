//
// VulkanConfig.h
//	Vulkan Setup
//
// Per-application settings this module needs, owned BY the module rather than reached up for.
//
// Historically these values were read directly from an `AppConstants.h` that each consuming
//	application had to supply, which inverted the dependency: a reusable library reaching up
//	into its consumer for configuration.
// THE APPLICATION DEFINES AppVulkanConfig(), declared below.  This module only declares and
//	calls it, so the linker requires every consumer to supply one -- there is no silent
//	fallback to defaults.  A definition is short, since the struct defaults every field:
//		const VulkanConfig& AppVulkanConfig()
//		{
//			static VulkanConfig config = [] {
//				VulkanConfig cfg;
//				cfg.appName		= "MyApp";
//				cfg.windowTitle	= "My Window";
//				return cfg;
//			}();
//			return config;
//		}
//	Being a FUNCTION with a function-local static is the point: it resolves correctly
//	whenever it is first called, including during static initialization.
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
	StrPtr		projectName			= "Unspecified";	// These two resolve the per-user path.
	StrPtr		debugLogFileName	= "DebugLog.txt";
	StrPtr		exePath				= "";	// The running executable, usually argv[0]; reported in the
											//	startup log.  Empty rather than null: it reaches printf.
			// NOTE: this one is late-bound.  AppVulkanConfig() may first be called during static
			//	initialization, before main() has seen argv -- so an application whose definition
			//	snapshots its fields once must refresh THIS field on each call (see the examples
			//	in the header comment's referenced applications) or it stays empty.

		// WINDOW - initial size and title, before any saved geometry is restored over them.
	StrPtr		windowTitle			= "Vulkan";
	int			defaultWindowWidth	= 1280;
	int			defaultWindowHeight	= 1024;

		// RENDERING
	VkClearColorValue clearColor	= { { 0.0f, 0.0f, 0.0f, 1.0f } };	// black
	bool		supportStereo3D		= false;
};


// DEFINED BY THE APPLICATION (see the header comment above).  Prefer iPlatform::Config() where
//	a platform is in reach; call this directly from code that runs without one, such as
//	Logging's free functions and FileSystem's static path resolution.
//
const VulkanConfig&	AppVulkanConfig();


#endif	// VulkanConfig_h
