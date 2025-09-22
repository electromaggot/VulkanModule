# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Building and Running

### Build the TestHarness
```bash
cd TestHarness/build
cmake ..
make -j$(nproc)
```

The built executable will be located at `TestHarness/build/VulkanTester`.

### Project Structure
TestHarness uses organized directory structure:
- `TestHarness/src/` - Source files (.cpp)
- `TestHarness/include/` - Header files (.h)
- `TestHarness/src/shaders/` - SPIR-V compiled shaders (.spv)

### Shader Compilation
The build system automatically copies pre-compiled SPIR-V shaders from `src/shaders/*.spv` to `build/compiledShaders/` during the build process.

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
- `VertexTypes/` - Various vertex format definitions
- `TextureImage`, `UniformBuffer` - Resource management

**Platform/** - Platform abstraction layer:
- `OSAbstraction/PlatformSDL` - SDL2 window/input handling (primary platform)
- `FileSystem/` - File I/O with conventions for shader/texture/model paths
- `ImageHandling/` - Image loading via SDL_image or STB

### Key Design Patterns

1. **RAII Resource Management**: Objects manage their Vulkan resources through constructors/destructors
2. **Recreate Pattern**: Objects support `Recreate()` for handling window resize/minimize events
3. **Dependency Order**: VulkanSetup instantiates objects in strict dependency order
4. **Platform Abstraction**: iPlatform interface allows different windowing systems (SDL2, GLFW, XCB)

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