# VulkanModule

**VulkanModule** provides a foundation upon which to build a Vulkan graphics project or game.

Vulkan itself requires a large amount of code and data structures to initialize and configure before any graphics can be displayed.  VulkanModule answers this need with two core parts:

1. **VulkanModule/Objects** encapsulate the various subsystems in an object-oriented fashion.  For example, the **`VulkanInstance`** class serves `VkInstance`, while **`GraphicsDevice`** administers `VkDevice` or `VkPhysicalDevice`, and so on.  Reasonable assumptions can be made from the names of 
the classes about their purpose, but if still in question, further detail is provided below.</br>
These classes not only focus on Creation and clean Destruction, but also reuse those to **`Recreate()`**, which is essential if the window resizes/minimizes/drags-across-displays, etc.  (The modules has other niceties too: like live resizing, where rendering continues during resize, or clean transitions upon phone rotation.)

2. **VulkanModule/Setup** combines the above Objects to set up the Vulkan system in an RAII fashion, the highlight being the **`VulkanSetup`** class to initialize all the base pieces in dependency order. 
This means first, progressively instantiating components that "won't change much" after they are constructed, such as:

   <table border="0">
     <tr>
       <td>
         <ol type="1">
           <li>vulkan instance</li>
           <li>window & surface</li>
           <li>physical & logical device</li>
           <li>swapchain</li>
         </ol>
       </td>
       <td>
         <ol  type="1" start="5">
           <li>depth buffer</li>
           <li>render pass</li>
           <li>frame buffers</li>
           <li>synchronization objects (like semaphores, fences)</li>
         </ol>
       </td>
     </tr>
   </table>

After the above core initialization, further Vulkan components incorporate to customize rendering and start defining the 3D Objects to be drawn:

3. **VulkanModule/Adjunct** includes these.  Some specify access to subsequent files, like to define model meshes or load textures.  Also here are various VertexTypes to fill a VertexBuffer, the format of which a corresponding Vertex Shader will expect.

________________________________

# Technical Details and Insights

**VulkanModule** is a comprehensive, object-oriented foundation for building Vulkan graphics applications. It abstracts away Vulkan's notoriously complex initialization process into manageable, reusable components.

## Architecture Philosophy

Vulkan requires extensive boilerplate code and careful orchestration of dependencies before rendering begins. VulkanModule solves this through a four-tier architecture that separates concerns and enables robust resource management:

## Core Components

### 1. **Objects/** - Vulkan Resource Encapsulation
Low-level Vulkan objects wrapped in RAII classes with intelligent lifecycle management:

- **`VulkanInstance`** - Vulkan instance creation with validation layers and debug reporting.
- **`GraphicsDevice`** - Physical/logical device selection with queue family management and device ranking.
- **`WindowSurface`** - Cross-platform surface creation (SDL2, GLFW, XCB support).
- **`Swapchain`** - Swapchain management with automatic recreation on window events.
- **`RenderPass`** - Render pass configuration with depth/stencil support.
- **`GraphicsPipeline`** - Pipeline state objects with shader module integration.
- **`Framebuffers`** - Framebuffer creation tied to swapchain lifecycle.
- **`SyncObjects`** - Semaphores, fences, and GPU/CPU synchronization primitives.
- **`CommandObjects`** - Command pool and buffer allocation strategies.
- **`DepthBuffer`** - Depth testing support with format selection.
- **`ImageResource`** - Flexible image resource management.

**Key Innovation**: Each object implements a `Recreate()` pattern enabling seamless handling of window resize, minimize, display changes, and device rotation while maintaining rendering continuity.

### 2. **Setup/** - Orchestrated Initialization
High-level composition layer that manages dependency order and configuration:

- **`VulkanSetup`** - Master initialization class following strict dependency order:
  1. Vulkan instance & validation
  2. Window surface creation
  3. Physical device selection & logical device
  4. Swapchain configuration
  5. Depth buffer allocation
  6. Render pass definition
  7. Framebuffer creation
  8. Synchronization object setup

- **`VulkanConfigure`** - Centralized configuration management.
- **`Shader`** - Shader module loading with SPIR-V support.
- **`RenderSettings`** - Render pipeline configuration.
- **`VulkanSingleton`** - Global state management.

### 3. **Adjunct/** - High-Level Abstractions
Application-focused components for content creation:

#### Renderables System
- **`iRenderable`** - Base interface for drawable objects.
- **`FixedRenderable`** - Static geometry with optimized vertex buffers.
- **`DynamicRenderable`** - Mutable geometry supporting real-time updates.
- **`MeshObject`** - 3D model representation with material support.
- **`DrawableSpecifier`** - Rendering configuration and state.
- **`Customizer`** - Bitfield flags for per-renderable pipeline customization:
  - `WIREFRAME` - Polygon line mode for debug visualization.
  - `SHOW_BACKFACES` - Disable backface culling for transparent geometry.
  - `FRONT_CLOCKWISE` - Vulkan-native clockwise winding.
  - `ALPHA_BLENDING` - Enable transparency with depth write disable.
  - `LINE_TOPOLOGY` - Render as line list instead of triangles (perfect for glowing edges, wireframe overlays).
  - Extensible for application-specific rendering modes.

#### Vertex Pipeline
- **`VertexAbstract`** - Base vertex interface with attribute binding.
- **`Vertex2DTypes`** - 2D rendering vertex formats (sprites, UI).
- **`Vertex3DTypes`** - 3D rendering with normals, textures, lighting.
- **`VertexDescription`** - Vulkan pipeline vertex input configuration.
- **`VerticesDynamic`** - Runtime vertex buffer management.

#### Resource Management
- **`TextureImage`** - Texture loading with mipmap generation.
- **`UniformBuffer`** - Shader uniform data with automatic layout.
- **`DynamicUniformBuffer`** - Efficient per-object uniform data using VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC with dynamic offsets for rendering thousands of objects.
- **`ShaderCache`** - Shared shader module management with reference counting to eliminate redundant shader loading when multiple renderables use the same shaders.
- **`BufferBase`** - Memory allocation strategies and buffer utilities.
- **`CommandBufferBase`** - Command recording abstractions.

#### Shadow Mapping System
- **`ShadowSystem`** (`Adjunct/Shadowing/`) - Complete shadow mapping infrastructure with **zero VRAM cost** when disabled:
  - **`ShadowSystem`** - Unified shadow management with per-frame shadow maps and automatic synchronization.
  - **`ShadowMap`** - Depth-only shadow map resources with configurable resolution and PCF sampling.
  - **`ShadowPass`** - Shadow pass command buffer recording with proper layout transitions.
  - **`ShadowProjection`** - Reusable light-space matrix calculations with multiple projection and camera modes.
  - **`ShadowMappingTypes`** - Modular type definitions for shadow techniques, projection modes, and camera orientations.

### 4. **Platform/** - Abstraction Layer
Cross-platform compatibility and I/O systems:

#### OS Abstraction
- **`iPlatform`** - Platform interface for window/input management.
- **`PlatformSDL`** - SDL2 implementation (primary platform).
- **`PlatformGLFW`** - GLFW alternative implementation.
- **`PlatformXCB`** - Direct X11/XCB support.

#### Subsystems
- **`FileSystem`** - Resource path management with conventions for shaders/textures/models.
- **`ImageHandling`** - Image I/O via SDL_image and STB libraries.
- **`Logger`** - Centralized logging with debug/release configurations.
- **`DearImGui`** - Optional debug UI integration (gracefully stubbed when unused).

#### Support Systems
- **`GameClock`** - Frame timing and delta calculations.
- **`Utility`** - Math helpers and common algorithms.
- **`VulkanMath`** - GLM integration with Vulkan conventions.

## Design Patterns & Benefits

### RAII Resource Management
Every Vulkan object is automatically managed through constructor/destructor pairs, eliminating resource leaks and ensuring proper cleanup order.

### Dependency Injection
`VulkanSetup` uses constructor initialization lists to enforce proper dependency order while maintaining clean separation of concerns.

### Platform Agnostic Design
The `iPlatform` interface enables deployment across Windows, macOS, Linux, iOS, and Android with minimal code changes.

### Live Resizing Support
Unlike typical Vulkan applications that freeze during window operations, VulkanModule continues rendering throughout resize/minimize/maximize events.

### Extensible Architecture
The modular design allows applications to use only needed components, from minimal "hello triangle" examples to full game engines.

### Resource Sharing and Scalability
ShaderCache enables efficient sharing of shader modules across renderables, eliminating redundant file I/O and memory usage when rendering scenes with thousands of objects using the same shaders. Combined with DynamicUniformBuffer's dynamic offset support, VulkanModule efficiently handles large-scale scenes with minimal per-object overhead.

## TestHarness Integration

The included TestHarness demonstrates minimal integration:
```cpp
class VulkanTester {
    PlatformSDL platform;
    VulkanSetup vulkan{platform, NO_DEPTH_BUFFER};
    // All Vulkan objects now ready for use
};
```

This single initialization replaces hundreds of lines of typical Vulkan setup code while providing enterprise-grade resource management and cross-platform compatibility.

</br>
________________________________

***DOCUMENT REVISIONS***

**Latest Updates (2025):**
- Added **Customizer** bitfield flags system documentation for per-renderable pipeline customization.
- Documented `LINE_TOPOLOGY` flag for line list rendering (glowing edges, wireframe overlays).
- Enhanced transparency support documentation (`SHOW_BACKFACES`, `ALPHA_BLENDING`).

**Previous Enhancements:**

Added to TECHNICAL DETAILS section: comprehensive architectural insights and
  detailed component descriptions. The updated documentation now provides:

Key Enhancements Added:

1. Structured Architecture Overview - Clear four-tier breakdown (Objects, Setup, Adjunct, Platform).
2. Detailed Component Descriptions - Each major class explained with its specific purpose and capabilities.
3. Design Pattern Documentation - RAII, dependency injection, platform abstraction, and live resizing explained.
4. Practical Benefits - Real-world advantages like cross-platform deployment and resource leak prevention.
5. Integration Example - Concrete code showing the simplicity achieved.
6. Technical Depth - Specific features like device ranking, queue family management, mipmap generation, etc.

The documentation now serves as both a technical reference for developers and a comprehensive
overview for project stakeholders, highlighting the sophisticated engineering that makes Vulkan
development accessible through this module.
