//
// VulkanConfig.cpp
//	Vulkan Setup
//
// See header file comment for overview.
//
// Created 8/13/26 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "VulkanConfig.h"


static const VulkanConfig	defaultConfig;			// every field defaulted in the struct itself

static const VulkanConfig*	pPublishedConfig = nullptr;


// Returns whatever the application published, else module defaults.  Never null, so callers
//	need no fallback of their own.
//
const VulkanConfig& TheVulkanConfig()
{
	return pPublishedConfig ? *pPublishedConfig : defaultConfig;
}

// Stores a REFERENCE: the application owns the VulkanConfig and must outlive the platform.
//	(Copying instead would silently diverge from an application that mutates its own copy.)
//
void publishVulkanConfig(const VulkanConfig& config)
{
	pPublishedConfig = &config;
}
