//
// ResourceTrackerImpl.h
//	VulkanModule, Assist/ResourceTracker
//
// Lightweight aggregate-based reference counting system for tracking Vulkan resource leaks.
// Provides shutdown summaries to identify forgotten resources without performance overhead.
//
// Design:
//  - Singleton tracker with per-resource-type counters (not per-handle)
//  - RAII guards for automatic tracking in constructors/destructors
//  - Debug-only (#ifndef NDEBUG) - zero overhead in release builds
//  - Thread-safe counter increments/decrements
//  - Shutdown leak reporting
//
// Usage:
//  Constructor: VK_TRACK_CREATE(VK_RESOURCE_DEVICE);
//  Destructor:  VK_TRACK_DESTROY(VK_RESOURCE_DEVICE);
//  Shutdown:    VK_RESOURCE_REPORT_LEAKS();
//
// Created 29 Nov 2024 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef ResourceTrackerImpl_h
#define ResourceTrackerImpl_h

#include <mutex>
#include <unordered_map>
#include <vulkan/vulkan.h>


// Vulkan resource types for aggregate tracking
enum VulkanResourceType {
	// Core hierarchy (parent → child dependencies)
	VK_RESOURCE_INSTANCE = 0,			// VkInstance
	VK_RESOURCE_DEVICE,					// VkDevice (logical)
	VK_RESOURCE_PHYSICAL_DEVICE,		// VkPhysicalDevice (not created/destroyed by us, but tracked for completeness)

	// Presentation chain
	VK_RESOURCE_SURFACE,				// VkSurfaceKHR
	VK_RESOURCE_SWAPCHAIN,				// VkSwapchainKHR
	VK_RESOURCE_IMAGE_VIEW,				// VkImageView (swapchain images, textures, depth buffers)

	// Rendering infrastructure
	VK_RESOURCE_RENDER_PASS,			// VkRenderPass
	VK_RESOURCE_FRAMEBUFFER,			// VkFramebuffer
	VK_RESOURCE_PIPELINE,				// VkPipeline (graphics, compute)
	VK_RESOURCE_PIPELINE_LAYOUT,		// VkPipelineLayout
	VK_RESOURCE_SHADER_MODULE,			// VkShaderModule

	// Resources
	VK_RESOURCE_IMAGE,					// VkImage (textures, depth buffer, shadow maps)
	VK_RESOURCE_BUFFER,					// VkBuffer (vertex, index, uniform, staging)
	VK_RESOURCE_DEVICE_MEMORY,			// VkDeviceMemory

	// Descriptors
	VK_RESOURCE_DESCRIPTOR_POOL,		// VkDescriptorPool
	VK_RESOURCE_DESCRIPTOR_SET_LAYOUT,	// VkDescriptorSetLayout
	VK_RESOURCE_DESCRIPTOR_SET,			// VkDescriptorSet (freed automatically with pool, but tracked for completeness)

	// Samplers
	VK_RESOURCE_SAMPLER,				// VkSampler

	// Synchronization
	VK_RESOURCE_SEMAPHORE,				// VkSemaphore
	VK_RESOURCE_FENCE,					// VkFence
	VK_RESOURCE_EVENT,					// VkEvent

	// Commands
	VK_RESOURCE_COMMAND_POOL,			// VkCommandPool
	VK_RESOURCE_COMMAND_BUFFER,			// VkCommandBuffer (freed automatically with pool, but tracked for completeness)

	// Debug
	VK_RESOURCE_DEBUG_REPORT,			// VkDebugReportCallbackEXT / VkDebugUtilsMessengerEXT

	VK_RESOURCE_TYPE_COUNT				// Total number of resource types
};


// Singleton tracker for Vulkan resource allocation/deallocation
class ResourceTracker
{
public:
	// Singleton access
	static ResourceTracker& instance();

	// Core tracking API
	void trackCreate(VulkanResourceType type);
	void trackDestroy(VulkanResourceType type);

	// Reporting API
	void reportLeaks();					// Shutdown leak report (only shows leaks)
	void reportStatistics();			// Detailed statistics (all resources)

	// Query API
	int getCurrentCount(VulkanResourceType type) const;
	int getTotalCreated(VulkanResourceType type) const;
	int getTotalDestroyed(VulkanResourceType type) const;

	// Control API
	void reset();						// Clear all counters (for testing)
	bool isEnabled() const;				// Check if tracking is active

	// Descriptor set tracking (for implicit destruction when pool is destroyed)
	void trackDescriptorSetAllocation(VkDescriptorPool pool, uint32_t count);
	uint32_t getDescriptorSetCountForPool(VkDescriptorPool pool);

private:
	ResourceTracker();
	~ResourceTracker();

	// Prevent copying
	ResourceTracker(const ResourceTracker&) = delete;
	ResourceTracker& operator=(const ResourceTracker&) = delete;

	// Resource statistics
	struct ResourceStats {
		int created = 0;				// Total created since startup
		int destroyed = 0;				// Total destroyed
		int current() const { return created - destroyed; }
	};

	ResourceStats stats[VK_RESOURCE_TYPE_COUNT];
	mutable std::mutex trackingMutex;	// Thread-safe tracking

	// Descriptor set tracking (maps pool handle to number of allocated descriptor sets)
	std::unordered_map<VkDescriptorPool, uint32_t> poolToSetCount;

	// Helper methods
	const char* resourceTypeName(VulkanResourceType type) const;
};


// RAII guard for automatic resource tracking
// Instantiated by macros, tracks creation/destruction
class ResourceTrackingGuard
{
public:
	ResourceTrackingGuard(VulkanResourceType type, bool isCreation);
	~ResourceTrackingGuard();

private:
	VulkanResourceType trackedType;
	bool wasCreation;
};


// Macros for compile-time control
// Only enabled in debug builds (NDEBUG not defined)
#ifndef NDEBUG
	// Track resource creation (call in constructor after successful creation)
	#define VK_TRACK_CREATE(type) \
		ResourceTrackingGuard __resource_guard_create_##__LINE__(type, true)

	// Track resource destruction (call in destructor after successful destruction)
	#define VK_TRACK_DESTROY(type) \
		ResourceTrackingGuard __resource_guard_destroy_##__LINE__(type, false)

	// Report leaks at shutdown
	#define VK_RESOURCE_REPORT_LEAKS() \
		ResourceTracker::instance().reportLeaks()

	// Report detailed statistics (optional, for debugging)
	#define VK_RESOURCE_REPORT_STATS() \
		ResourceTracker::instance().reportStatistics()
#else
	// Release builds - macros compile to nothing (zero overhead)
	#define VK_TRACK_CREATE(type)      ((void)0)
	#define VK_TRACK_DESTROY(type)     ((void)0)
	#define VK_RESOURCE_REPORT_LEAKS() ((void)0)
	#define VK_RESOURCE_REPORT_STATS() ((void)0)
#endif


#endif	// ResourceTrackerImpl_h
