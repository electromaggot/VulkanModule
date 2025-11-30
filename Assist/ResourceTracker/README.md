# Vulkan Resource Tracker

Debug-only automatic resource leak detection for Vulkan objects via macro interception.

## What It Does

Tracks creation and destruction of all Vulkan resources (instances, devices, swapchains, buffers, images, pipelines, shader modules, synchronization primitives, etc.) and reports leaks at application shutdown.

## How It Works

Uses **macro interception** to automatically wrap all `vkCreate*` and `vkDestroy*` calls in debug builds (`#ifndef NDEBUG`):

1. **Macros redirect** Vulkan API calls to tracking wrappers
   - Example: `vkCreateBuffer` → `trkCreateBuffer`
   - Example: `vkDestroyBuffer` → `trkDestroyBuffer`

2. **Wrappers call real Vulkan functions** and track success/failure automatically
   - On success (`VK_SUCCESS`): Increment resource counter
   - On destruction: Decrement resource counter

3. **Shutdown leak report** shows which resource types weren't properly destroyed
   - Reports only leaked resources (non-zero counts)
   - Shows created vs destroyed counts for diagnostics

## Zero Runtime Cost in Release

All tracking code compiles away when `NDEBUG` is defined (release builds):
- Macros not defined → no function redirection
- Wrapper functions not compiled → zero binary size increase
- Tracking logic eliminated → zero performance overhead

## Architecture

**Files:**
- `ResourceTrackerImpl.h/.cpp` - Core tracking singleton with aggregate counters, reporting, and RAII guards
- `ResourceTracker.h` - Macro interception layer (main header to include)
- `ResourceTrackerWrappers.cpp` - Wrapper function implementations (trk* functions)
- `README.md` - This documentation

**Tracked Resources (25 types):**
- Core: VkInstance, VkDevice, VkPhysicalDevice
- Presentation: VkSurfaceKHR, VkSwapchainKHR, VkImageView
- Rendering: VkRenderPass, VkFramebuffer, VkPipeline, VkPipelineLayout, VkShaderModule
- Resources: VkImage, VkBuffer, VkDeviceMemory
- Descriptors: VkDescriptorPool, VkDescriptorSetLayout, VkDescriptorSet
- Samplers: VkSampler
- Synchronization: VkSemaphore, VkFence, VkEvent
- Commands: VkCommandPool, VkCommandBuffer
- Debug: VkDebugReportCallback

## Usage

Resource tracking is **automatically enabled** in debug builds for files that include `ResourceTracker/ResourceTracker.h`.

Key VulkanModule headers already include it (via hybrid approach), so most files get tracking automatically through transitive includes.

**No manual code changes needed!**

## Example Output

**With leaks:**
```
====================================================================
VULKAN RESOURCE LEAK REPORT
====================================================================
VkPipeline:                    2 leaked (5 created, 3 destroyed)
VkBuffer:                      1 leaked (10 created, 9 destroyed)
====================================================================
TOTAL: 3 resources leaked across 2 types
====================================================================
```

**Clean shutdown:**
```
✅ ✅ Vulkan Resource Tracking: No leaks detected (all resources properly cleaned up)
```

## How to Disable

1. **Temporary (debug build):** Comment out `#include "ResourceTracker/ResourceTracker.h"` from VulkanModule headers
2. **Permanent:** Exclude `ResourceTracker/` directory from `CMakeLists.txt`
3. **Release build:** Build with `-DNDEBUG` flag (tracking automatically disabled)

## Design Rationale

**Why macro interception over manual tracking?**

✅ **Non-invasive** - Zero modifications to production VulkanModule code
✅ **Comprehensive** - Automatically tracks ALL Vulkan calls, impossible to forget
✅ **Maintainable** - All tracking logic in one directory (4 files)
✅ **Removable** - Delete directory or exclude from build to completely remove
✅ **Debug-only** - Guaranteed zero impact on release builds via `#ifndef NDEBUG`
✅ **Thread-safe** - Mutex-protected counters for concurrent operations

This approach treats resource tracking as what it is: a **debug diagnostic tool**, not production code.

## Technical Details

**Macro Interception Mechanism:**
```cpp
// In ResourceTracker.h (debug builds only):
#ifndef NDEBUG
    #define vkCreateBuffer trkCreateBuffer
    #define vkDestroyBuffer trkDestroyBuffer
#endif

// In ResourceTrackerWrappers.cpp:
#undef vkCreateBuffer  // Access real function

VkResult trkCreateBuffer(...) {
    VkResult result = vkCreateBuffer(...);  // Call real Vulkan
    if (result == VK_SUCCESS)
        VK_TRACK_CREATE(VK_RESOURCE_BUFFER);
    return result;
}
```

**Aggregate Counting (not per-handle):**
- Tracks counts per resource *type*, not individual handles
- Example: "5 VkBuffers created, 3 destroyed" → 2 leaked
- Lightweight: just 25 integer pairs (created/destroyed)
- No heap allocations, no hash maps, minimal overhead

**Thread Safety:**
- All counter updates protected by `std::mutex`
- Safe for multi-threaded resource creation/destruction
- Lock-free reads in release builds (tracking disabled)

## Extending

To track additional Vulkan resource types:

1. Add enum to `ResourceTrackerImpl.h`: `VK_RESOURCE_MY_TYPE`
2. Add name mapping in `resourceTypeName()`
3. Add wrapper declarations in `ResourceTracker.h`
4. Add wrapper implementations in `ResourceTrackerWrappers.cpp`
5. Add macro redirection: `#define vkCreateMyType trkCreateMyType`

## Validation Layer Integration

This tracker is **complementary** to Vulkan validation layers:
- **Validation layers:** Detailed error checking, per-handle tracking, verbose output
- **ResourceTracker:** Lightweight aggregate counts, shutdown summary only

Use validation layers during development, keep ResourceTracker for quick leak detection during testing.
