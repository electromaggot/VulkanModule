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
  - `iRenderable::UpdateUniformBuffers()` - Public method for uploading UBO data to GPU
- `VertexTypes/` - Various vertex format definitions
- `TextureImage`, `UniformBuffer` - Resource management
- `DynamicUniformBuffer` - Efficient per-object uniform data using dynamic offsets for rendering thousands of objects
- `Shadowing/` - Complete shadow mapping infrastructure
  - `ShadowMap` - Shadow map image, render pass, framebuffer, and sampler
  - `ShadowPass` - Shadow pass command buffer recording and management

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

### GameClock - Frame Timing and FPS Tracking

`Assist/GameClock.h` provides frame timing utilities for time-dependent rendering and game logic.

**Features:**
- **Delta time calculation**: Precise per-frame time for smooth animations
- **Elapsed time tracking**: Total time since application start
- **FPS counter**: Automatic calculation and tracking, updated every second

**Usage:**

```cpp
GameClock gameClock;

// In main loop
gameClock.BeginNewFrame();  // Call once per frame

float deltaTime = gameClock.deltaSeconds();     // Time since last frame
float elapsed = gameClock.secondsElapsed();     // Total elapsed time
int fps = gameClock.getFPS();                   // Current FPS (0 until first second)
bool fpsUpdated = gameClock.wasFPSUpdated();    // True if FPS recalculated this frame
```

**FPS Counter Details:**
- Calculates FPS every second based on actual frame count
- Returns 0 until first full second has elapsed
- `wasFPSUpdated()` allows checking when to update UI/logs
- Useful for performance diagnostics and monitoring

### Platform Utilities

**PlatformSDL Extensions:**
- `SetWindowTitle()` - Update window title (useful for FPS display)
- Display information logging at startup:
  - Monitor index and name
  - Resolution and refresh rate
  - Performance hints for built-in vs external displays

**macOS-Specific:**
- MoltenVK configuration for maximum performance
- vsync disabled at Metal layer when possible
- Async queue submits enabled
- Note: macOS compositor may still enforce vsync on external displays

### Shadow Mapping

VulkanModule provides complete shadow mapping infrastructure for realistic shadow rendering. The system supports both orthographic (directional/sun) and perspective (point light) projections, with flexible camera orientation modes and automatic resolution scaling.

**Core Components:**

**ShadowSystem** (`Adjunct/Shadowing/ShadowSystem.h`) - **Primary Interface**
- Unified management of shadow mapping resources and rendering
- **Zero VRAM allocation** when `SHADOW_TECHNIQUE_NONE` is specified
- Encapsulates multiple `ShadowMap` instances (one per swapchain image) and `ShadowPass`
- Single, clean API for initialization and per-frame recording
- Automatic cross-frame synchronization support
- Methods:
  - `recordFrame()` - Returns true if shadows were recorded, false if disabled
  - `isEnabled()` - Check if shadow system is active
  - `getPerFrameDescriptorInfo()` - Get shadow map descriptors for all frames
  - `getRenderPass()`, `getExtent()`, `getCommandBuffer()` - Resource accessors

**ShadowMap** (`Adjunct/Shadowing/ShadowMap.h`)
- Encapsulates all shadow map resources:
  - Depth-only image with configurable resolution (default 2048x2048)
  - Image view, render pass, and framebuffer
  - Depth sampler with PCF (Percentage Closer Filtering)
- Supports `Recreate()` for window resize handling
- `RecreateWithNewResolution()` for dynamic resolution changes
- RAII resource management

**ShadowPass** (`Adjunct/Shadowing/ShadowPass.h`)
- Manages shadow pass command buffer recording
- Depth-only rendering from light's perspective
- Proper layout transitions and synchronization
- Records all shadow-casting renderables
- **Critical**: Uses actual shadow map dimensions via `shadowMap.getWidth()/getHeight()` (not hardcoded constants)

**ShadowProjection** (`Adjunct/Shadowing/ShadowProjection.h`)
- Reusable utility for calculating light-space projection matrices
- **Projection modes**:
  - `SHADOW_ORTHOGRAPHIC`: Parallel light rays (sun/directional)
  - `SHADOW_PERSPECTIVE`: Radial light rays (point light)
- **Camera orientation modes** (`ShadowCameraMode`):
  - `SHADOW_CAMERA_STRAIGHT_DOWN`: Points straight down (-Y axis), prevents clipping with wide FOV
  - `SHADOW_CAMERA_CUSTOM_DIRECTION`: Uses custom direction vector for spotlights
  - `SHADOW_CAMERA_LOOK_AT_ORIGIN`: Looks from light toward scene center
- **Dynamic resolution calculation**: `calculateRecommendedResolution()` scales shadow map based on FOV and camera mode
- Handles gimbal lock avoidance automatically
- Configurable ortho size, FOV, near/far planes

**Usage Example:**

```cpp
#include "Shadowing/ShadowMap.h"
#include "Shadowing/ShadowPass.h"
#include "Shadowing/ShadowProjection.h"

// 1. Calculate optimal shadow map resolution based on FOV and camera mode
float shadowFOV = glm::radians(170.0f);  // Wide FOV for maximum coverage
ShadowCameraMode cameraMode = SHADOW_CAMERA_STRAIGHT_DOWN;
uint32_t optimalResolution = ShadowProjection::calculateRecommendedResolution(
    shadowFOV, cameraMode, SHADOW_PERSPECTIVE);
// Returns 4096x4096 for 170° FOV with STRAIGHT_DOWN mode

// 2. Create shadow map with dynamic resolution
ShadowMap* shadowMap = new ShadowMap(vulkan.device, commandPool,
                                      optimalResolution, optimalResolution);
ShadowPass* shadowPass = new ShadowPass(vulkan, *shadowMap);

// 3. Each frame, calculate light-space matrix with camera mode
vec3 lightPos = light.getPosition();
vec3 sceneCenter = vec3(0.0f, 0.0f, 0.0f);
float fov = glm::radians(170.0f);
float farPlane = 60.0f;

shadowUBO.lightSpaceMatrix = ShadowProjection::calculateLightSpaceMatrix(
    lightPos, sceneCenter, SHADOW_PERSPECTIVE,
    SHADOW_CAMERA_STRAIGHT_DOWN,         // Camera orientation
    glm::vec3(0.0f, -1.0f, 0.0f),       // customDirection (for CUSTOM_DIRECTION)
    15.0f,                               // orthoSize (for ORTHOGRAPHIC)
    fov,                                 // Field of view
    0.1f,                                // nearPlane
    farPlane);                           // farPlane

// 4. Record shadow pass
shadowPass->recordShadowPass(shadowRenderables, frameIndex);

// 5. Shadow map sampler automatically bound at descriptor binding 4
// Main pass shaders sample shadow map for shadow calculations
```

**Configuration (VulkanConfigure.h):**

```cpp
// Shadow projection mode
enum ShadowProjectionMode {
    SHADOW_ORTHOGRAPHIC,    // Directional/sun light (parallel rays)
    SHADOW_PERSPECTIVE      // Point light source (radial rays)
};

// Shadow camera orientation mode
enum ShadowCameraMode {
    SHADOW_CAMERA_STRAIGHT_DOWN,      // Points down -Y axis (default, prevents clipping)
    SHADOW_CAMERA_CUSTOM_DIRECTION,   // Uses custom direction vector
    SHADOW_CAMERA_LOOK_AT_ORIGIN      // Looks from light toward scene center
};

// Quality/performance tunables (defaults)
const uint32_t SHADOW_MAP_WIDTH = 2048;   // Default resolution (scales with FOV)
const uint32_t SHADOW_MAP_HEIGHT = 2048;
const int PCF_KERNEL_RADIUS = 1;          // Shadow softness (1=3x3, 2=5x5, 3=7x7)
const float SHADOW_BIAS = 0.0015f;        // Prevents shadow acne
```

**Shadow Projection Modes:**

- **SHADOW_ORTHOGRAPHIC**:
  - Parallel light rays across entire scene
  - Uniform shadow quality regardless of distance
  - Best for outdoor scenes with sun/moon lighting
  - Uses `glm::ortho()` projection

- **SHADOW_PERSPECTIVE**:
  - Radial light rays from light position
  - Matches Phong lighting for accurate shadows
  - Shadow detail decreases with distance
  - Best for indoor scenes with point lights
  - Uses `glm::perspective()` with configurable FOV (default 90°, up to 170° for wide coverage)

**Shadow Camera Orientation Modes:**

- **SHADOW_CAMERA_STRAIGHT_DOWN** (default):
  - Camera points straight down (-Y axis)
  - Prevents shadow clipping at light's extreme positions
  - Best for overhead lighting with wide FOV
  - Natural for scenes with ground planes

- **SHADOW_CAMERA_CUSTOM_DIRECTION**:
  - Camera uses custom direction vector
  - Useful for spotlights or directed lighting effects
  - Specify direction via `customDirection` parameter

- **SHADOW_CAMERA_LOOK_AT_ORIGIN**:
  - Camera looks from light toward scene center
  - Good for focused lighting on central scene elements
  - May clip shadows at extremes with wide FOV

**Dynamic Resolution Scaling:**

`ShadowProjection::calculateRecommendedResolution()` automatically selects optimal shadow map resolution based on:
- **FOV**: Wider FOV requires higher resolution to prevent pixelation
- **Camera mode**: STRAIGHT_DOWN and CUSTOM_DIRECTION modes benefit from higher resolution
- **Projection mode**: Perspective typically needs higher resolution than orthographic

Resolution guidelines (for STRAIGHT_DOWN/CUSTOM_DIRECTION):
- FOV 150°+: 4096x4096 (4K shadow map)
- FOV 120-150°: 3072x3072 (3K shadow map)
- FOV 90-120°: 2048x2048 (2K shadow map, default)
- FOV < 90°: 2048x2048

**Quality Tuning:**

Shadow Map Resolution:
- 1024x1024: Fast, softer shadows (low-end hardware)
- 2048x2048: Balanced (default for narrow FOV)
- 3072x3072: High quality (wide FOV 120-150°)
- 4096x4096: Very high quality (wide FOV 150°+, high-end hardware)
- Use `calculateRecommendedResolution()` for automatic selection
- Override via `ShadowMap` constructor or `RecreateWithNewResolution()`

PCF Kernel Radius:
- 1 (3x3): 9 samples, fast, sharper edges
- 2 (5x5): 25 samples, balanced
- 3 (7x7): 49 samples, slow, very soft shadows

Shadow Bias:
- Too low: "shadow acne" (dotted artifacts)
- Too high: "peter panning" (detached shadows)
- Default 0.0015f suitable for most scenes

FOV Selection:
- 90° or less: Standard, good quality with 2K shadow map
- 90-120°: Standard-wide, may need 2-3K shadow map
- 120-150°: Wide, requires 3K shadow map to prevent pixelation
- 150°+: Very wide, requires 4K shadow map for sharp shadows
- Wide FOV with STRAIGHT_DOWN mode prevents shadow clipping

**UBO Bindings:**
Applications using shadow mapping must follow these descriptor bindings:
- Binding 0: Camera MVP matrices
- Binding 1: Dynamic UBO (per-object transforms)
- Binding 2: Lighting UBO
- Binding 3: Shadow UBO (light-space matrix)
- Binding 4: Shadow map sampler (texture)

**Implementation Details:**
- Uses `GLM_FORCE_DEPTH_ZERO_TO_ONE` for [0,1] depth range
- Two-pass rendering: shadow pass → main pass
- PCF sampling in fragment shader for soft shadows
- Adaptive bias based on surface angle to light
- Shadows affect diffuse/specular only, not ambient
