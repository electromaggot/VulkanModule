# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Known Design Debt

### AppConstants coupling
VulkanModule internals directly `#include "AppConstants.h"`, a header that each consuming application must provide. Affected files include `PlatformSDL.cpp`, `VulkanSingleton.h`, `Logging.cpp`, `VulkanInstance.cpp`, `Framebuffers.cpp`, `Swapchain.cpp`, `FileSystemSDL.h`, and `iPlatform.h`.

This inverts the correct dependency direction: a reusable library should not reach up into its consumer for configuration. The right fix is to define an `AppConfig` or `VulkanConfig` struct (or equivalent) inside VulkanModule, accept it through the `VulkanSetup` constructor, and remove all direct references to `AppConstants`. Consuming projects would then populate that struct from their own `AppConstants` (or any other source) before calling `VulkanSetup`. The struct contains consumer-specific per-app values like window title, clear color, dimensions, app name, etc. which are meant to be overridden but have typical default values.

Deferred because multiple projects currently depend on VulkanModule and rely on the existing `AppConstants` convention.

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

### Coordinate System and Winding Order

**Coordinate System:**
- The module defaults to **standard Vulkan**: right-handed, `+Z` out of the screen, CCW front faces
- Optional **left-handed** mode (`+Z` into the screen, forward) is opted into *per application*:
  ```cmake
  target_compile_definitions(MyApp PRIVATE INVERT_Z_SETTING=true)
  ```
- `Assist/VulkanMath.h` reads that as `constexpr bool INVERT_Z`, defaulting to `false`
- **Scope**: `INVERT_Z` has exactly one effect — it reverses `GraphicsPipeline`'s default
  `VkFrontFace` (`GraphicsPipeline.cpp`), because left-handed geometry winds opposite. It does
  **not** alter projection or view math; the module never calls `glm::perspectiveLH_ZO()` or
  `glm::lookAtLH()`. Applications build their own MVP matrices and choose handedness there.
- Consequently an app that sets `INVERT_Z_SETTING=true` must also supply left-handed
  projection/view matrices itself — the setting only keeps culling consistent with them.
- **Consumers read `INVERT_Z` too.** Applications commonly branch on this same symbol as their
  coordinate-system contract — LevelEdit/TuneTrip keys projection (`glm::perspectiveLH_ZO`),
  OBJ winding order, Z-negation at model load, and camera math off it. So while its effect
  *within this module* is the single `VkFrontFace` flip, changing the effective value is
  engine-wide for such an app. Treat it as a coordinate-system contract, not a local toggle.

**Winding Order Convention:**
- **Counter-clockwise (CCW)** is the standard for front-facing triangles
- Pipeline configured with `VK_FRONT_FACE_COUNTER_CLOCKWISE` by default
- All geometry (manually-defined or procedurally-generated) must use CCW winding
- OBJ files follow CCW standard (Wavefront specification)
- See `Objects/GraphicsPipeline.cpp` lines 204-219 for detailed rationale

**Override for Direct3D/Unity Models:**
- Use `FRONT_CLOCKWISE` or `MODELED_FOR_DIRECT3D` Customizer flags for CW-wound models
- See "Rendering Customizations" section below for details

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
- `Renderables/` - Unified renderable system with pipeline batching optimization
  - `Renderable` - Single unified class for dynamic geometry (defaults to `UPON_EACH_FRAME` recording)
  - `SecondaryRenderable` - Optimized for static geometry using secondary command buffers recorded once at initialization
  - `RenderBatchManager` - Pipeline batching system to minimize state changes
  - `iRenderableBase` - Base for self-managed renderables (ImGui)
  - `iRenderable` - Extends base with full pipeline/descriptor management
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
- `OSAbstraction/MacOSFullScreen.mm` - macOS native fullscreen detection/control (Obj-C++)
- `FileSystem/` - File I/O with conventions for shader/texture/model paths
- `ImageHandling/` - Image loading via SDL_image or STB

### Key Design Patterns

1. **RAII Resource Management**: Objects manage their Vulkan resources through constructors/destructors
2. **Recreate Pattern**: Objects support `Recreate()` for handling window resize/minimize events
3. **Dependency Order**: VulkanSetup instantiates objects in strict dependency order
4. **Platform Abstraction**: iPlatform interface allows different windowing systems (SDL2, GLFW, XCB)
5. **Resource Sharing**: ShaderCache enables sharing ShaderModules across renderables to eliminate redundant loads
6. **Dynamic Uniform Buffers**: Support for VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC with per-object offsets for efficient multi-object rendering
7. **Pipeline Batching**: RenderBatchManager groups renderables by pass and pipeline, reducing state changes from O(N) to O(M)
8. **Pass-Based Rendering**: Explicit render order (opaque → transparent → lines → self-managed) ensures correct depth sorting

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
Renderable renderable(*drawable, vulkan, platform);
renderable.hasDynamicOffset = true;
renderable.dynamicOffset = dynamicUBO->getDynamicOffset(objectIndex);

// 4. Update per-object transforms each frame
dynamicUBO->updateObjectTransform(frameIndex, objectIndex, modelMatrix);
```

**Benefits:**
- Renders thousands of objects with minimal overhead
- Single buffer allocation instead of hundreds/thousands of individual buffers
- Reduced descriptor set updates
- Lower memory fragmentation
- Works seamlessly with pipeline batching optimization

**Implementation Details:**
- Automatically adds `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC` to descriptor pool
- `Renderable` checks `hasDynamicOffset` and passes offset array to `vkCmdBindDescriptorSets()`
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
Renderable renderable(*drawable, vulkan, platform);

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

### Secondary Command Buffers

VulkanModule supports secondary command buffers for optimal rendering of static geometry that never changes. This provides significant performance benefits by recording draw commands once at initialization time instead of every frame.

**Core Component:**
- `SecondaryRenderable` (`Adjunct/Renderables/SecondaryRenderable.{h,cpp}`) - Renderable using secondary command buffers

**When to Use:**

**Use `SecondaryRenderable` for:**
- Static geometry that never changes (skyboxes, static environment elements)
- Geometry recorded once and reused every frame
- Objects where command buffer recording CPU cost is significant

**Use `Renderable` for:**
- Dynamic geometry that changes frequently
- Objects requiring per-frame updates
- Most scene objects (default choice)

**Usage Example:**

```cpp
// Define static geometry (e.g., skybox)
DrawableSpecifier* drawable = new DrawableSpecifier(
    skyboxMesh,
    "Skybox",
    "skybox"  // Render pass name
);
drawable->shaders = {
    { VERTEX,   "skybox-vert.spv" },
    { FRAGMENT, "skybox-frag.spv" }
};

// Create SecondaryRenderable (records commands once at init)
SecondaryRenderable* skybox = new SecondaryRenderable(*drawable, vulkan, platform);

// No per-frame recording needed - command buffer reused automatically
```

**Benefits:**
- **Zero CPU overhead per frame** - command buffers recorded once, reused forever
- **Optimal for static geometry** - skyboxes, static backgrounds, UI elements
- **Automatic reuse** - no manual management required
- **Full Recreate() support** - handles window resize/display changes automatically

**Implementation Details:**
- Inherits from `iRenderable` for compatibility with existing infrastructure
- Records secondary command buffers at initialization time
- Command buffers marked as `VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT`
- Automatically re-recorded during `Recreate()` for window resize events
- Works seamlessly with pipeline batching and render pass ordering

**Performance Characteristics:**
- **First frame**: Slight overhead for secondary command buffer recording
- **Subsequent frames**: Near-zero CPU cost (only `vkCmdExecuteCommands()` call)
- **Best for**: Geometry with 100+ draw calls that never changes
- **Trade-off**: Slightly higher initial memory usage vs. primary command buffers

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
- `IsEventQUIT()` - Application quit detection
  - **Shift+ESC** - Quit application (requires modifier to prevent accidental quits)
  - **SDL_QUIT** - Window manager quit event
  - **Window close button** - User closes window
- Keyboard handling:
  - **ESC** (unshifted) - Exits fullscreen back to windowed mode
  - **Shift+ESC** - Quits application

**macOS-Specific:**
- MoltenVK configuration for maximum performance
- vsync disabled at Metal layer when possible
- Async queue submits enabled
- Note: macOS compositor may still enforce vsync on external displays

### Window Geometry Persistence

PlatformSDL automatically saves and restores window position, size, and fullscreen state across sessions. The app's `AppSettings` class provides the persistence backend.

**How it works:**
- `PlatformSDL::recordWindowGeometry()` is called automatically on window move/resize events
- Saves to `AppConstants.Settings` (the app's `AppSettings` instance) and calls `Settings.Save()`
- On startup, `PlatformSDL::createVulkanCompatibleWindow()` reads saved values from `AppSettings` to position/size the window, and validates against all connected displays
- Window position is validated across all monitors; falls back to centered on primary if the saved position is off-screen

**iPlatform public member:**
- `bool isFullScreen` - Tracks current fullscreen state (updated automatically)

**What gets saved (fields required in the app's `AppSettings` class):**
```cpp
int startingWindowWidth;       // Window width (windowed mode)
int startingWindowHeight;      // Window height (windowed mode)
int startingWindowX;           // Window X position (windowed mode)
int startingWindowY;           // Window Y position (windowed mode)
bool isFullScreen = false;     // Whether to restore fullscreen on next launch
```

When fullscreen is active, only `isFullScreen` is updated -- the windowed dimensions are preserved so the correct window size is restored when leaving fullscreen.

### Fullscreen Support

PlatformSDL provides borderless fullscreen (`SDL_WINDOW_FULLSCREEN_DESKTOP`) with automatic save/restore and macOS native fullscreen interception.

**Behavior:**

| Action | Result |
|--------|--------|
| Green button (macOS) | Native fullscreen detected, automatically switched to borderless fullscreen |
| ESC (unshifted) | Exits fullscreen back to windowed mode |
| Shift+ESC | Quits application (from any state) |
| Quit while fullscreen | Saves `isFullScreen: true` + last windowed geometry |
| Launch with saved fullscreen | Borderless fullscreen applied on first idle poll |

**macOS native fullscreen detection** (`Platform/OSAbstraction/MacOSFullScreen.mm`):

SDL2 does not track macOS native fullscreen (green title-bar button) with its own flags. On notched MacBooks, the window size is identical in windowed-maximized and native fullscreen, making size-based detection impossible. The solution queries `NSWindow styleMask & NSWindowStyleMaskFullScreen` directly via `SDL_GetWindowWMInfo`.

When native fullscreen is detected, PlatformSDL:
1. Exits native fullscreen via `[NSWindow toggleFullScreen:]`
2. Applies borderless fullscreen (`SDL_WINDOW_FULLSCREEN_DESKTOP`) on the next idle poll

This provides consistent behavior: ESC always exits fullscreen, no menu bar appears on mouse hover, and the state is properly saved/restored.

**Deferred fullscreen restore:**

`SDL_SetWindowFullscreen()` requires the macOS Cocoa run loop to be active, so it cannot be called during window creation. PlatformSDL sets a `pendingFullScreen` flag during `createVulkanCompatibleWindow()` and applies fullscreen on the first idle `PollEvent()` call (when the event queue is empty and the run loop is pumping).

**CMake requirements for consuming projects:**

```cmake
# Enable Obj-C++ compilation (required for MacOSFullScreen.mm)
project(MyProject CXX OBJCXX)

# Add the .mm source file (Apple only)
if(APPLE)
    list(APPEND Platform_OSAbstraction
        "Vulkan/Platform/OSAbstraction/MacOSFullScreen.mm"
    )
endif()

# Link the Cocoa framework (Apple only)
if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES ${COCOA_FRAMEWORK})
endif()

# Compile Obj-C++ with ARC — see "Objective-C memory management" below.
if(APPLE)
    set_source_files_properties(
        "Vulkan/Platform/OSAbstraction/MacOSFullScreen.mm"
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
    )
endif()
```

### Objective-C memory management (ARC) — convention for `.mm` files

**Build with ARC, and keep the source ARC-neutral anyway.** Both halves matter:

**1. Both build systems must agree.** An Xcode target defaults to `CLANG_ENABLE_OBJC_ARC = YES`, while
CMake compiles Obj-C++ *without* ARC unless told otherwise. The same `.mm` compiled one way in Xcode and
the other in CMake broke the build once (Aug 2026, `macOS_SetSustainedPerformance`). Hence the
`set_source_files_properties(... "-fobjc-arc")` above — consuming projects should mirror it.

The non-ARC direction is the dangerous one, because it fails *silently*: a method returning an
**autoreleased** object (`beginActivityWithOptions:reason:`, most `+arrayWith…`/`+stringWith…`
constructors) stored in a static or long-lived variable dies at the next pool drain, leaving a dangling
pointer and a quietly inert feature — no compile error, no crash until much later. ARC cannot get this
wrong; hand-written retain/release can.

**2. Write ARC-neutral source regardless.** VulkanModule is consumed by multiple projects whose build
settings we don't control, so memory management here should compile correctly either way:

```objc
static id<NSObject> s_token = nil;      // __strong under ARC; hand-retained otherwise.

s_token = [[NSProcessInfo processInfo] beginActivityWithOptions: opts reason: why];
#if !__has_feature(objc_arc)
    [s_token retain];                   // ARC does this implicitly via the strong static.
#endif
...
#if !__has_feature(objc_arc)
    [s_token release];
#endif
s_token = nil;                          // Under ARC, this assignment is what releases it.
```

Runtime behaviour is identical in both modes, so there is no divergence to reason about — only the
spelling differs. ARC governs Objective-C object pointers only; **C++ code is entirely unaffected**, so
enabling it costs a mixed C++/Obj-C codebase nothing.

**AppSettings requirements:**

The app's `AppSettings` class must have:
- `bool isFullScreen = false;` member
- `Save()` that persists `isFullScreen` to storage
- `Retrieve()` that loads `isFullScreen` from storage (called from constructor)

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
  - `SHADOW_CAMERA_CUSTOM_DIRECTION`: Uses custom direction vector (e.g. directional/sun light, spotlights). Default direction is -Y (straight down).
  - `SHADOW_CAMERA_LOOK_AT_TARGET`: Looks from light position toward a specified target point. Defaults to origin for backward compatibility with Scenes.
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
ShadowCameraMode cameraMode = SHADOW_CAMERA_CUSTOM_DIRECTION;
uint32_t optimalResolution = ShadowProjection::calculateRecommendedResolution(
    shadowFOV, cameraMode, SHADOW_PERSPECTIVE);
// Returns 4096x4096 for 170° FOV with CUSTOM_DIRECTION mode

// 2. Create shadow map with dynamic resolution
ShadowMap* shadowMap = new ShadowMap(vulkan.device, commandPool,
                                      optimalResolution, optimalResolution);
ShadowPass* shadowPass = new ShadowPass(vulkan, *shadowMap);

// 3. Each frame, calculate light-space matrix with camera mode
vec3 lightPos = light.getPosition();
vec3 target = vec3(0.0f, 0.0f, 0.0f);   // Scene focus point (origin, or camera position for Trips)
float fov = glm::radians(170.0f);
float farPlane = 60.0f;

shadowUBO.lightSpaceMatrix = ShadowProjection::calculateLightSpaceMatrix(
    lightPos, target, SHADOW_PERSPECTIVE,
    SHADOW_CAMERA_LOOK_AT_TARGET,        // Look toward target from light position
    glm::vec3(0.0f, -1.0f, 0.0f),       // customDirection (for CUSTOM_DIRECTION mode)
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
    SHADOW_CAMERA_CUSTOM_DIRECTION,   // Uses custom direction vector (default -Y)
    SHADOW_CAMERA_LOOK_AT_TARGET      // Looks from light toward target point (default origin)
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

- **SHADOW_CAMERA_CUSTOM_DIRECTION**:
  - Camera uses custom direction vector (default -Y, i.e. straight down)
  - Best for directional/sun lighting with orthographic projection
  - Direction stays constant regardless of scene movement (parallel rays)
  - Specify direction via `customDirection` parameter

- **SHADOW_CAMERA_LOOK_AT_TARGET**:
  - Camera looks from light position toward a specified target point
  - Target defaults to origin for backward compatibility with Scenes
  - For Trips, pass the scene focus point (e.g. camera XZ position) so shadows track the action
  - Best for point-light shadows with perspective projection

**Dynamic Resolution Scaling:**

`ShadowProjection::calculateRecommendedResolution()` automatically selects optimal shadow map resolution based on:
- **FOV**: Wider FOV requires higher resolution to prevent pixelation
- **Camera mode**: CUSTOM_DIRECTION mode benefits from higher resolution at wide FOV
- **Projection mode**: Perspective typically needs higher resolution than orthographic

Resolution guidelines (for CUSTOM_DIRECTION):
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
- Two-layer approach recommended: hardware depth bias on shadow pass pipeline (`DEPTH_BIAS` customizer flag) plus small fragment shader bias
- Hardware bias (constant + slope factors) prevents self-shadowing at the GPU level
- Fragment shader bias catches residuals (0.0003–0.0005 typical)
- Too much total bias: "peter panning" (detached shadows)

FOV Selection:
- 90° or less: Standard, good quality with 2K shadow map
- 90-120°: Standard-wide, may need 2-3K shadow map
- 120-150°: Wide, requires 3K shadow map to prevent pixelation
- 150°+: Very wide, requires 4K shadow map for sharp shadows

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

### Rendering Customizations (Customizer Flags)

VulkanModule provides bitfield flags (`Adjunct/Renderables/Customizer.h`) for per-renderable customization of the graphics pipeline. Applications can extend this enum with additional flags.

**Built-in Flags:**

```cpp
enum Customizer
{
    NONE                = 0,
    WIREFRAME           = 0b00000001,  // VK_POLYGON_MODE_LINE instead of _FILL
    SHOW_BACKFACES      = 0b00000010,  // VK_CULL_MODE_NONE instead of _BACK_BIT
    FRONT_CLOCKWISE     = 0b00000100,  // not VK_FRONT_FACE_COUNTER_CLOCKWISE (Vulkan native)
    MODELED_FOR_DIRECT3D = 0b00001000,  // Model uses D3D/Unity conventions (vs OpenGL/Vulkan)
    ALPHA_BLENDING      = 0b00010000,  // Enable alpha blending + disable depth writes
    LINE_TOPOLOGY       = 0b00100000   // VK_PRIMITIVE_TOPOLOGY_LINE_LIST
};
```

**How To Use:**

```cpp
DrawableSpecifier* drawable = object->createDrawable();

// Single flag
drawable->customize = SHOW_BACKFACES;

// Multiple flags (bitwise OR)
drawable->customize = static_cast<Customizer>(LINE_TOPOLOGY | SHOW_BACKFACES | ALPHA_BLENDING);

// Create renderable
Renderable renderable(*drawable, vulkan, platform);
```

**Flag Details:**

- **WIREFRAME**: Renders triangle edges as lines (debug/visualization)
  - Sets `VK_POLYGON_MODE_LINE` in rasterization state
  - Does NOT affect primitive topology (still triangle list)

- **SHOW_BACKFACES**: Disables backface culling
  - Sets `VK_CULL_MODE_NONE` in rasterization state
  - Essential for transparent geometry where both sides are visible
  - Useful for line rendering to show away-facing edges

- **FRONT_CLOCKWISE**: Uses clockwise winding for front faces
  - Sets `VK_FRONT_FACE_CLOCKWISE`
  - Direct3D/Unity native (OpenGL uses counter-clockwise by default)
  - Use `MODELED_FOR_DIRECT3D` for models created with D3D or Unity conventions

- **ALPHA_BLENDING**: Enables alpha blending and disables depth writes
  - Configures blend state for transparency (src_alpha, one_minus_src_alpha)
  - Disables depth writes (allows see-through rendering)
  - Required for billboards, particles, and transparent geometry

- **LINE_TOPOLOGY**: Renders as line list instead of triangles
  - Sets `VK_PRIMITIVE_TOPOLOGY_LINE_LIST` in input assembly
  - Indices interpreted as pairs: (v0,v1), (v2,v3), etc.
  - Perfect for glowing edges, wireframe overlays, debug visualization
  - Typically combined with `SHOW_BACKFACES` and `ALPHA_BLENDING`

**Extending Customizer:**

Applications can extend the enum with additional flags (use bits 0b01000000 and higher):

```cpp
// In application-level Customizer.h override
enum Customizer
{
    // ... built-in flags ...
    LINE_TOPOLOGY       = 0b00100000,
    MY_CUSTOM_FLAG      = 0b01000000,  // Application-specific
    ANOTHER_FLAG        = 0b10000000
};
```

**Pipeline Configuration:**

The `GraphicsPipeline` class automatically applies customizations during pipeline creation:

- Rasterization state: `polygonMode`, `cullMode`, `frontFace`
- Input assembly: `topology` (triangle list vs line list)
- Depth/stencil state: `depthWriteEnable` (disabled with ALPHA_BLENDING)
- Color blend state: Blend equation (configured with ALPHA_BLENDING)

**Common Combinations:**

```cpp
// Transparent geometry (windows, glass)
drawable->customize = static_cast<Customizer>(SHOW_BACKFACES | ALPHA_BLENDING);

// Glowing line edges (TRON-style)
drawable->customize = static_cast<Customizer>(LINE_TOPOLOGY | SHOW_BACKFACES | ALPHA_BLENDING);

// Debug wireframe overlay
drawable->customize = WIREFRAME;

// Direct3D-native model
drawable->customize = static_cast<Customizer>(MODELED_FOR_DIRECT3D | FRONT_CLOCKWISE);
```
