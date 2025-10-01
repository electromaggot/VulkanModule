# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Building and Running

### Prerequisites

#### All Platforms
- CMake 3.16+
- Vulkan SDK
- SDL2 and SDL2_image libraries
- GLM (OpenGL Mathematics library)

#### Platform-Specific Setup

**macOS:**
```bash
# Install via Homebrew
brew install vulkan-headers vulkan-loader molten-vk sdl2 sdl2_image glm
```

**Windows:**
- Install Vulkan SDK from https://vulkan.lunarg.com/
- Use vcpkg for dependencies:
```cmd
vcpkg install sdl2 sdl2-image glm vulkan
```

**Linux/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install vulkan-tools libvulkan-dev vulkan-validationlayers-dev
sudo apt-get install libsdl2-dev libsdl2-image-dev libglm-dev
sudo apt-get install build-essential cmake pkg-config
```

**Raspberry Pi 5:**
```bash
# Same as Linux, plus ARM64-specific packages
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

### Build the TestHarness

**All Platforms:**
```bash
cd TestHarness
mkdir -p build && cd build
cmake ..
cmake --build . -j$(nproc)
```

**Windows (Visual Studio):**
```cmd
cd TestHarness
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

The built executable will be located at:
- **macOS/Linux:** `TestHarness/build/VulkanTester`
- **Windows:** `TestHarness/build/Release/VulkanTester.exe`

### Project Structure
TestHarness uses organized directory structure:
- `TestHarness/src/` - Source files (.cpp)
- `TestHarness/include/` - Header files (.h)
- `TestHarness/src/shaders/` - SPIR-V compiled shaders (.spv)

### Cross-Platform Notes
- **macOS:** Uses MoltenVK for Vulkan support, requires portability extensions
- **Windows:** Direct Vulkan support, uses Win32 platform extensions
- **Linux:** Direct Vulkan support, uses XCB platform extensions
- **Raspberry Pi 5:** ARM64 optimizations enabled automatically

### Shader Compilation
The build system automatically copies pre-compiled SPIR-V shaders from `src/shaders/*.spv` to the appropriate runtime directory during the build process.

Note: This module is typically used as a submodule by projects like HelloVulkanSDL which provide additional shader compilation infrastructure.

## Architecture Overview

VulkanModule is a reusable foundation for Vulkan graphics projects, providing object-oriented encapsulation of Vulkan's initialization and setup requirements.

### Core Components

**Objects/** - Encapsulates Vulkan subsystems in RAII fashion:
- `VulkanInstance` - Manages VkInstance
- `GraphicsDevice` - Manages VkDevice/VkPhysicalDevice selection and queues
- `Swapchain` - Handles swapchain creation and recreation on resize
- `RenderPass`, `GraphicsPipeline`, `Framebuffers` - Core rendering pipeline
- `WindowSurface` - Platform surface abstraction
- `SyncObjects` - Semaphores and fences for GPU/CPU synchronization

**Setup/** - Orchestrates initialization:
- `VulkanSetup` - Main initialization class that creates all Vulkan objects in dependency order
- `VulkanConfigure` - Configuration management
- `Shader` - Shader module loading

**Adjunct/** - Higher-level abstractions:
- `Renderables/` - Base classes for drawable objects (FixedRenderable, DynamicRenderable)
  - `ShaderCache` - Shared shader management with reference counting to eliminate redundant shader loading
- `VertexTypes/` - Various vertex format definitions
- `TextureImage`, `UniformBuffer` - Resource management
- `DynamicUniformBuffer` - Efficient per-object uniform data using dynamic offsets for rendering thousands of objects

**Platform/** - Platform abstraction layer:
- `OSAbstraction/PlatformSDL` - SDL2 window/input handling (primary platform)
- `FileSystem/` - File I/O with conventions for shader/texture/model paths
- `ImageHandling/` - Image loading via SDL_image or STB

### Key Design Patterns

1. **RAII Resource Management**: Objects manage their Vulkan resources through constructors/destructors
2. **Recreate Pattern**: Objects support `Recreate()` for handling window resize/minimize events
3. **Dependency Order**: VulkanSetup instantiates objects in strict dependency order
4. **Platform Abstraction**: iPlatform interface allows different windowing systems (SDL2, GLFW, XCB)
5. **Resource Sharing**: ShaderCache enables sharing ShaderModules across renderables to eliminate redundant loads
6. **Dynamic Uniform Buffers**: Support for VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC with per-object offsets for efficient multi-object rendering

### Dependencies

- Vulkan SDK (headers and loader)
- SDL2 and SDL2_image
- GLM (math library)
- Optional: Dear ImGui for debug UI (stubbed if not used)

### TestHarness

The TestHarness directory contains a minimal test application demonstrating module usage. The main class `VulkanTester` shows the typical initialization pattern:
1. Create platform (SDL)
2. Instantiate VulkanSetup with platform and configuration
3. Store references to key objects for rendering loop

## Advanced Features

### Dynamic Uniform Buffers

Dynamic Uniform Buffers enable efficient rendering of large numbers of objects by using a single shared buffer with dynamic offsets instead of creating individual UniformBuffer objects per renderable.

**Usage:**

```cpp
// 1. Create DynamicUniformBuffer (typically in application initialization)
const uint32_t MAX_OBJECTS = 1000;
const uint32_t FRAMES_IN_FLIGHT = swapchain.getNumImages();
DynamicUniformBuffer* dynamicUBO = new DynamicUniformBuffer(MAX_OBJECTS, FRAMES_IN_FLIGHT, device);

// 2. Configure drawable to use dynamic UBO
drawable->pUBOs = {
    camera.uboMVP,          // Binding 0: Static camera matrices
    UBO(dynamicUBO)         // Binding 1: Dynamic per-object transforms
};

// 3. Set dynamic offset on renderable
FixedRenderable fixedRenderable(*drawable, vulkan, platform);
fixedRenderable.hasDynamicOffset = true;
fixedRenderable.dynamicOffset = dynamicUBO->getDynamicOffset(objectIndex);

// 4. Update per-object transforms each frame
dynamicUBO->updateObjectTransform(frameIndex, objectIndex, modelMatrix);
```

**Benefits:**
- Renders thousands of objects with minimal overhead
- Single buffer allocation instead of hundreds/thousands of individual buffers
- Reduced descriptor set updates
- Lower memory fragmentation

**Implementation Details:**
- Automatically adds `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC` to descriptor pool
- `FixedRenderable` checks `hasDynamicOffset` and passes offset array to `vkCmdBindDescriptorSets()`
- Update transforms before recording command buffers each frame

### Shader Caching

ShaderCache eliminates redundant shader loading when multiple renderables use the same shaders, crucial for scenes with thousands of objects.

**Usage:**

```cpp
// 1. Create ShaderCache (typically in application initialization)
ShaderCache* shaderCache = new ShaderCache(vulkan.device);

// 2. Get or create shared shaders for a drawable
ShaderModules* pSharedShaders = shaderCache->getOrCreate(drawable->shaders);
drawable->pSharedShaderModules = pSharedShaders;
shaderCache->addRef(pSharedShaders);

// 3. Create renderable (will use shared shaders)
FixedRenderable fixedRenderable(*drawable, vulkan, platform);

// 4. ShaderCache handles cleanup automatically via reference counting
// When last renderable using a shader set is destroyed, shaders are freed
```

**Benefits:**
- Shaders loaded once and shared across all renderables using the same shader set
- Automatic reference counting prevents premature deletion
- Reduced I/O and memory usage
- Faster scene initialization

**Implementation Details:**
- Shaders are cached by a key generated from shader file names and types
- `iRenderable` checks `pSharedShaderModules` and uses it if provided, otherwise creates its own
- `ownsShaderModules` flag prevents double-deletion
- Reference counting ensures shaders persist until last user is destroyed