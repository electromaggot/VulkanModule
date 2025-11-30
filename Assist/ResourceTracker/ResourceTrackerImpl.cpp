//
// ResourceTrackerImpl.cpp
//	VulkanModule, Assist/ResourceTracker
//
// Implementation of lightweight Vulkan resource leak tracker.
// See header for usage details.
//
// Created 29 Nov 2024 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "ResourceTrackerImpl.h"
#include "Logging.h"
#include <cstdio>


// Singleton instance access
ResourceTracker& ResourceTracker::instance()
{
	static ResourceTracker tracker;
	return tracker;
}


// Constructor
ResourceTracker::ResourceTracker()
{
	// Counters initialized to zero by default
}


// Destructor
ResourceTracker::~ResourceTracker()
{
	// No cleanup needed (aggregate counters only)
}


// Track resource creation
void ResourceTracker::trackCreate(VulkanResourceType type)
{
	if (type >= VK_RESOURCE_TYPE_COUNT)
		return;	// Invalid type

	std::lock_guard<std::mutex> lock(trackingMutex);
	stats[type].created++;
}


// Track resource destruction
void ResourceTracker::trackDestroy(VulkanResourceType type)
{
	if (type >= VK_RESOURCE_TYPE_COUNT)
		return;	// Invalid type

	std::lock_guard<std::mutex> lock(trackingMutex);
	stats[type].destroyed++;
}


// Report only leaked resources (non-zero current counts)
void ResourceTracker::reportLeaks()
{
	std::lock_guard<std::mutex> lock(trackingMutex);

	// Count total leaks and overall statistics
	int totalLeaks = 0;
	int leakedTypes = 0;
	int totalCreated = 0;
	int totalDestroyed = 0;

	for (int i = 0; i < VK_RESOURCE_TYPE_COUNT; i++) {
		int current = stats[i].current();
		if (current > 0) {
			totalLeaks += current;
			leakedTypes++;
		}
		totalCreated += stats[i].created;
		totalDestroyed += stats[i].destroyed;
	}

	// Report results
	if (totalLeaks == 0) {
		Log(GOOD, "✅ ✅ Vulkan Resource Tracking: No leaks detected (all resources properly cleaned up)");
		Log(GOOD, "   Total: %d created, %d destroyed", totalCreated, totalDestroyed);
		return;
	}

	// Leak report header
	Log(RAW, "====================================================================");
	Log(RAW, "VULKAN RESOURCE LEAK REPORT");
	Log(RAW, "====================================================================");

	// List each leaked resource type
	for (int i = 0; i < VK_RESOURCE_TYPE_COUNT; i++) {
		int current = stats[i].current();
		if (current > 0) {
			const char* name = resourceTypeName((VulkanResourceType)i);
			Log(RAW, "%-30s %d leaked (%d created, %d destroyed)",
				name, current, stats[i].created, stats[i].destroyed);
		}
	}

	// Footer
	Log(RAW, "====================================================================");
	Log(RAW, "TOTAL: %d resources leaked across %d types", totalLeaks, leakedTypes);
	Log(RAW, "       %d total created, %d total destroyed", totalCreated, totalDestroyed);
	Log(RAW, "====================================================================");
}


// Report detailed statistics for all resources (including non-leaked)
void ResourceTracker::reportStatistics()
{
	std::lock_guard<std::mutex> lock(trackingMutex);

	int totalCurrent = 0;
	int totalCreated = 0;
	int totalDestroyed = 0;

	// Calculate totals
	for (int i = 0; i < VK_RESOURCE_TYPE_COUNT; i++) {
		totalCurrent += stats[i].current();
		totalCreated += stats[i].created;
		totalDestroyed += stats[i].destroyed;
	}

	// Header
	Log(RAW, "====================================================================");
	Log(RAW, "VULKAN RESOURCE STATISTICS");
	Log(RAW, "====================================================================");

	// List all resource types with non-zero counts
	for (int i = 0; i < VK_RESOURCE_TYPE_COUNT; i++) {
		if (stats[i].created > 0 || stats[i].destroyed > 0) {
			const char* name = resourceTypeName((VulkanResourceType)i);
			int current = stats[i].current();
			Log(RAW, "%-30s %d current (%d created, %d destroyed)",
				name, current, stats[i].created, stats[i].destroyed);
		}
	}

	// Footer
	Log(RAW, "--------------------------------------------------------------------");
	Log(RAW, "TOTAL: %d current (%d created, %d destroyed)",
		totalCurrent, totalCreated, totalDestroyed);
	Log(RAW, "====================================================================");
}


// Query current count for a specific resource type
int ResourceTracker::getCurrentCount(VulkanResourceType type) const
{
	if (type >= VK_RESOURCE_TYPE_COUNT)
		return 0;

	std::lock_guard<std::mutex> lock(trackingMutex);
	return stats[type].current();
}


// Query total created count
int ResourceTracker::getTotalCreated(VulkanResourceType type) const
{
	if (type >= VK_RESOURCE_TYPE_COUNT)
		return 0;

	std::lock_guard<std::mutex> lock(trackingMutex);
	return stats[type].created;
}


// Query total destroyed count
int ResourceTracker::getTotalDestroyed(VulkanResourceType type) const
{
	if (type >= VK_RESOURCE_TYPE_COUNT)
		return 0;

	std::lock_guard<std::mutex> lock(trackingMutex);
	return stats[type].destroyed;
}


// Reset all counters (for testing)
void ResourceTracker::reset()
{
	std::lock_guard<std::mutex> lock(trackingMutex);
	for (int i = 0; i < VK_RESOURCE_TYPE_COUNT; i++) {
		stats[i].created = 0;
		stats[i].destroyed = 0;
	}
}


// Check if tracking is enabled (always true in debug builds)
bool ResourceTracker::isEnabled() const
{
#ifndef NDEBUG
	return true;
#else
	return false;
#endif
}


// Track descriptor set allocation for a specific pool
void ResourceTracker::trackDescriptorSetAllocation(VkDescriptorPool pool, uint32_t count)
{
	std::lock_guard<std::mutex> lock(trackingMutex);
	poolToSetCount[pool] += count;
}


// Get descriptor set count for a pool (and remove from map)
uint32_t ResourceTracker::getDescriptorSetCountForPool(VkDescriptorPool pool)
{
	std::lock_guard<std::mutex> lock(trackingMutex);
	auto it = poolToSetCount.find(pool);
	if (it != poolToSetCount.end()) {
		uint32_t count = it->second;
		poolToSetCount.erase(it);
		return count;
	}
	return 0;  // Pool not found (might not have had any descriptor sets)
}


// Map resource type enum to human-readable name
const char* ResourceTracker::resourceTypeName(VulkanResourceType type) const
{
	switch (type) {
		// Core hierarchy
		case VK_RESOURCE_INSTANCE:				return "VkInstance:";
		case VK_RESOURCE_DEVICE:				return "VkDevice:";
		case VK_RESOURCE_PHYSICAL_DEVICE:		return "VkPhysicalDevice:";

		// Presentation chain
		case VK_RESOURCE_SURFACE:				return "VkSurfaceKHR:";
		case VK_RESOURCE_SWAPCHAIN:				return "VkSwapchainKHR:";
		case VK_RESOURCE_IMAGE_VIEW:			return "VkImageView:";

		// Rendering infrastructure
		case VK_RESOURCE_RENDER_PASS:			return "VkRenderPass:";
		case VK_RESOURCE_FRAMEBUFFER:			return "VkFramebuffer:";
		case VK_RESOURCE_PIPELINE:				return "VkPipeline:";
		case VK_RESOURCE_PIPELINE_LAYOUT:		return "VkPipelineLayout:";
		case VK_RESOURCE_SHADER_MODULE:			return "VkShaderModule:";

		// Resources
		case VK_RESOURCE_IMAGE:					return "VkImage:";
		case VK_RESOURCE_BUFFER:				return "VkBuffer:";
		case VK_RESOURCE_DEVICE_MEMORY:			return "VkDeviceMemory:";

		// Descriptors
		case VK_RESOURCE_DESCRIPTOR_POOL:		return "VkDescriptorPool:";
		case VK_RESOURCE_DESCRIPTOR_SET_LAYOUT:	return "VkDescriptorSetLayout:";
		case VK_RESOURCE_DESCRIPTOR_SET:		return "VkDescriptorSet:";

		// Samplers
		case VK_RESOURCE_SAMPLER:				return "VkSampler:";

		// Synchronization
		case VK_RESOURCE_SEMAPHORE:				return "VkSemaphore:";
		case VK_RESOURCE_FENCE:					return "VkFence:";
		case VK_RESOURCE_EVENT:					return "VkEvent:";

		// Commands
		case VK_RESOURCE_COMMAND_POOL:			return "VkCommandPool:";
		case VK_RESOURCE_COMMAND_BUFFER:		return "VkCommandBuffer:";

		// Debug
		case VK_RESOURCE_DEBUG_REPORT:			return "VkDebugReport:";

		default:								return "Unknown:";
	}
}


//
// ResourceTrackingGuard implementation
//

ResourceTrackingGuard::ResourceTrackingGuard(VulkanResourceType type, bool isCreation)
	: trackedType(type)
	, wasCreation(isCreation)
{
	// Track immediately on construction
	if (wasCreation)
		ResourceTracker::instance().trackCreate(trackedType);
	else
		ResourceTracker::instance().trackDestroy(trackedType);
}


ResourceTrackingGuard::~ResourceTrackingGuard()
{
	// No action needed - tracking happened in constructor
	// This RAII pattern ensures tracking can't be forgotten
}
