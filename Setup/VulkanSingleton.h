//
// VulkanSingleton.h
//	Vulkan Setup
//
// ...
//
// 2/1/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef VulkanSingleton_h
#define VulkanSingleton_h

#include "VulkanConfig.h"


class VulkanSingleton
{
public:
	const char* EngineName		 = "None";		// (would perhaps have special significance for an
	const uint32_t EngineVersion = VK_MAKE_VERSION(0, 0, 0);	// engine like unreal/unity/etc.)

	VkClearColorValue ClearColor = TheVulkanConfig().clearColor;
				// Read once, when instance() is first called -- which, being a function-local
				//	static, happens lazily at first use, by which point the platform has
				//	published the application's config.  Assign this member directly to change
				//	the clear color later at runtime.


// CONSTRUCTION & INSTANCING
private:										// Make external construction impossible:
	VulkanSingleton() { }
	VulkanSingleton(VulkanSingleton const& copy);				// and copy-constructors
	VulkanSingleton& operator = (VulkanSingleton const& copy);	//	non-implemented too.
public:
	static VulkanSingleton& instance()			// The only instance.
	{											// Guaranteed to be lazy-initialized,
		static VulkanSingleton instance;						// and destroyed correctly.
		return instance;
	}
};

#endif // VulkanSingleton_h
