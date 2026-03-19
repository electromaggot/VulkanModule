# VulkanModule: AI Quick-Start Guide

Guide for an AI assistant bootstrapping a new graphics project using VulkanModule.
Optimized for fast, correct rendering on first attempt.

**Do NOT follow the TestHarness as a template.** TestHarness uses right-handed coordinates (`glm::lookAt`), no depth buffer, and shader-defined vertices. It is a minimal "hello Vulkan" that sidesteps most engine conventions. Any real 3D project will break if you copy its patterns.

---

## Critical Rules (violating any of these produces invisible geometry)

1. **Left-handed coordinate system.** `INVERT_Z = true` in `Assist/VulkanMath.h`. +Z is into the screen.
2. **Must use LH GLM functions:**
   ```cpp
   VP.view = glm::lookAtLH(eye, target, up);
   VP.proj = glm::perspectiveLH_ZO(fov, aspect, near, far);
   VP.proj[1][1] *= -1.0f;  // Vulkan NDC Y-flip — always required
   ```
   `glm::lookAt()` and `glm::perspective()` are right-handed and WILL produce a black screen. No crash, no error — just nothing visible.
3. **Camera at negative Z, looking toward positive Z.** Example: `eye = (0, 1, -3)`, `target = (0, 0, 10)`.
4. **CCW winding = front face** (with `INVERT_Z`, the engine swaps to CW internally via `VK_FRONT_FACE_CLOCKWISE`).
5. **Use `BASIC` SteerSetup** (not `NO_DEPTH_BUFFER`) for any 3D content that needs depth testing.
6. **Use a visible clear color during development.** Near-black is indistinguishable from "nothing rendering." Use something like `{ 0.1, 0.15, 0.35, 1.0 }`.

---

## Project Skeleton

### Directory Structure

```
MyProject/
├── VulkanModule -> (symlink to VulkanModule)
├── CMakeLists.txt
├── include/
│   ├── AppConstants.h        # REQUIRED — engine includes this from consuming app
│   ├── AppSettings.h         # REQUIRED — settings persistence
│   ├── PlatformSpecifics.h   # REQUIRED — copy from TestHarness/include/
│   ├── PlatformConstants.h   # REQUIRED — copy from TestHarness/include/
│   └── MyApp.h               # Your app class
└── src/
    ├── main.cpp
    ├── AppSettings.cpp        # Copy from TestHarness/src/
    ├── MyApp.cpp
    └── shaders/
        ├── myshader.vert
        ├── myshader.frag
        ├── myshader-vert.spv  # Pre-compiled SPIR-V
        └── myshader-frag.spv
```

### Required Boilerplate Files (copy from TestHarness, then customize)

**`AppConstants.h`** — The engine `#include`s this file. Every project must provide it.
```cpp
#ifndef AppConstants_h
#define AppConstants_h

#ifdef INSTANTIATE
  #define extern
#endif

#include "PlatformConstants.h"
#include "VulkanPlatform.h"
#include "AppSettings.h"

extern struct Constants
{
    const StrPtr AppName            = "MyProject";
    const uint32_t AppVersion       = VK_MAKE_VERSION(1, 0, 0);
    const StrPtr WindowTitle        = "My Window Title";
    const StrPtr SettingsFileName   = "Settings.json";
    const StrPtr DebugLogFileName   = "DebugLog.txt";
    const StrPtr CompanyName        = "GitHubProject";
    const StrPtr ProjectName        = "MyProject";

    const int DefaultWindowWidth    = 1280;
    const int DefaultWindowHeight   = 720;
    const int MaxSaneScreenWidth    = 7680 * 2;
    const int MaxSaneScreenHeight   = 4320 * 2;

    // USE A VISIBLE COLOR during development:
    VkClearColorValue DefaultClearColor = { { 0.1f, 0.15f, 0.35f, 1.0f } };

    const bool SupportStereo3D      = false;

    StrPtr  getExePath() const  { return exePath; }
    void    setExePath(StrPtr p){ if (exePath == nullptr) exePath = p; }
private:
    StrPtr  exePath = nullptr;
public:
    AppSettings  Settings;
} AppConstants;

#ifdef extern
  #undef extern
#endif
#endif
```

**`PlatformSpecifics.h`** and **`PlatformConstants.h`** — Copy verbatim from `TestHarness/include/`. These are app-level files the engine expects to find in the consumer's include path.

**`AppSettings.h` / `AppSettings.cpp`** — Copy verbatim from TestHarness. Handles window geometry persistence.

### main.cpp Template

```cpp
#include "MyApp.h"
#include "AppConstants.h"

int main(int argc, char* argv[])
{
    AppConstants.setExePath(argv[0]);

    MyApp app;
    try {
        app.Init();
        app.Run();
    } catch (const exception& e) {
        app.DialogBox(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
```

---

## App Class Template (renders 3D content immediately)

### Header

```cpp
#ifndef MyApp_h
#define MyApp_h

#include "AppSettings.h"
#include "PlatformSDL.h"
#include "VulkanSetup.h"
#include "DrawableSpecifier.h"
#include "GameClock.h"
#include "iControlScheme.h"    // if you want mouse/touch input

const SteerSetup withDepth = BASIC;  // USE THIS for 3D. NOT NO_DEPTH_BUFFER.

class MyApp : public iControlScheme  // inherit if you need input
{
private:
    PlatformSDL     platform;
    VulkanSetup     vulkan;

public:
    MyApp()
        :   platform(),
            vulkan(platform, withDepth),
            device(vulkan.device.getLogical()),
            swapchain(vulkan.swapchain.getVkSwapchain()),
            deviceQueue(vulkan.device.Queues.getCurrent()),
            syncObjects(vulkan.syncObjects),
            swapchainExtent(vulkan.swapchain.getExtent())
    { }

    ~MyApp() { vkDeviceWaitIdle(device); }

private:
    int iCurrentFrame = 0;
    VkDevice&       device;
    VkSwapchainKHR& swapchain;
    VkQueue&        deviceQueue;
    SyncObjects&    syncObjects;
    VkExtent2D&     swapchainExtent;

    typedef uint64_t NanosecondTimeout;
    const NanosecondTimeout FAILSAFE_TIMEOUT = 100'000'000;
    GameClock gameClock;
    VkResult  call;

    UBO_VP  VP;  // or UBO_MVP if you need per-object model matrices

public:
    void Init();
    void Run();
    void DialogBox(const char* message) { platform.DialogBox(message); }

    // iControlScheme overrides (if using input)
    void handlePrimaryPressDown(int atX, int atY) override;

private:
    void prepareForMainLoop();
    void updateRender();
    void updateScene();
    void draw();
    void recalculateProjectionIfChanged();
    void setPerspectiveProjection();
    static void ForceUpdateRender(void* pOurself);
};
#endif
```

### Implementation — Key Methods

```cpp
#include "MyApp.h"
#include "CommandObjects.h"
#include "Renderable.h"

void MyApp::Init()
{
    prepareForMainLoop();

    // --- Create your renderables here ---
    // Example with real vertex data:
    //   DrawableProperties props = {
    //       .mesh = myMeshObject,
    //       .name = "MyObject",
    //       .shaders = { { VERTEX,   "myshader-vert.spv" },
    //                    { FRAGMENT, "myshader-frag.spv" } },
    //       .pUBOs = { UBO(VP) },
    //       .textures = { myTexSpec },       // optional
    //       .customize = SHOW_BACKFACES      // optional flags
    //   };
    //   DrawableSpecifier specifier(props);
    //   vulkan.command.renderables.Add(Renderable(specifier, vulkan, platform));

    vulkan.command.PostInitPrepBuffers(vulkan);

    platform.RegisterForceRenderCallback(MyApp::ForceUpdateRender, this);
}

void MyApp::prepareForMainLoop()
{
    // LEFT-HANDED: camera at -Z looking toward +Z
    const vec3 eyePosition  = vec3(0.0f, 1.0f, -3.0f);
    const vec3 lookAtTarget = vec3(0.0f, 0.0f, 10.0f);
    const vec3 upVector     = vec3(0.0f, 1.0f, 0.0f);   // +Y is up

    VP.view = glm::lookAtLH(eyePosition, lookAtTarget, upVector);
    setPerspectiveProjection();
}

void MyApp::setPerspectiveProjection()
{
    const float nearPlane = 0.1f;
    const float farPlane  = 100.0f;
    const float fov = glm::radians(45.0f);
    float aspect = swapchainExtent.width / (float)swapchainExtent.height;

    VP.proj = glm::perspectiveLH_ZO(fov, aspect, nearPlane, farPlane);
    VP.proj[1][1] *= -1.0f;   // Vulkan NDC Y-flip
}

void MyApp::Run()
{
    platform.ClearEvents();
    for (bool quit = false; !quit; )
    {
        while (platform.PollEvent(this))    // pass 'this' for iControlScheme input
            quit = platform.IsEventQUIT();
        if (platform.IsWindowMinimizedOrHidden())
            platform.AwaitEvent();
        updateRender();
    }
}

void MyApp::updateRender()
{
    gameClock.BeginNewFrame();
    updateScene();
    draw();
}

void MyApp::ForceUpdateRender(void* pOurself)
{
    MyApp* pSelf = static_cast<MyApp*>(pOurself);
    if (pSelf) {
        if (pSelf->platform.isWindowResized)
            pSelf->vulkan.RecreateRenderingResources();
        pSelf->updateRender();
    }
}

void MyApp::updateScene()
{
    float dt = gameClock.deltaSeconds();    // NOTE: camelCase, not DeltaTimeSeconds()
    if (dt > 0.1f) dt = 0.1f;

    // Update your objects here

    if (vulkan.command.renderables.Update(gameClock))
        recalculateProjectionIfChanged();
}

void MyApp::recalculateProjectionIfChanged()
{
    if (swapchainExtent.width != platform.LastSavedPixelsWide
     || swapchainExtent.height != platform.LastSavedPixelsHigh)
        setPerspectiveProjection();
}

void MyApp::draw()
{
    uint32_t iNextImage;

    vkWaitForFences(device, 1, &syncObjects.inFlightFences[iCurrentFrame],
                    VK_TRUE, FAILSAFE_TIMEOUT);
    call = vkAcquireNextImageKHR(device, swapchain, FAILSAFE_TIMEOUT,
                                  syncObjects.imageAvailableSemaphores[iCurrentFrame],
                                  VK_NULL_HANDLE, &iNextImage);
    const char* called = "Acquire Next Image";

    vkResetFences(device, 1, &syncObjects.inFlightFences[iCurrentFrame]);

    if (call == VK_SUCCESS)
    {
        vulkan.command.RecordRenderablesForNextFrame(vulkan, iNextImage);
        vulkan.command.renderables.UpdateUniformBuffers(iNextImage);

        VkPipelineStageFlags waitStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        vector<VkCommandBuffer> cmdBuffers = vulkan.command.BuffersForFrame(iNextImage);

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &syncObjects.imageAvailableSemaphores[iCurrentFrame],
            .pWaitDstStageMask = &waitStageFlags,
            .commandBufferCount = (uint32_t) cmdBuffers.size(),
            .pCommandBuffers = cmdBuffers.data(),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &syncObjects.renderFinishedSemaphores[iCurrentFrame]
        };
        VkSubmitInfo submits[] = { submitInfo };

        call = vkQueueSubmit(deviceQueue, N_ELEMENTS_IN_ARRAY(submits), submits,
                             syncObjects.inFlightFences[iCurrentFrame]);
        called = "Queue Submit";

        if (call == VK_SUCCESS)
        {
            VkPresentInfoKHR presentInfo = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &syncObjects.renderFinishedSemaphores[iCurrentFrame],
                .swapchainCount = 1,
                .pSwapchains = &swapchain,
                .pImageIndices = &iNextImage,
                .pResults = nullptr
            };
            call = vkQueuePresentKHR(deviceQueue, &presentInfo);
            called = "Queue Present";
        }
    }
    if (call == VK_ERROR_OUT_OF_DATE_KHR || call == VK_SUBOPTIMAL_KHR)
    {
        vulkan.RecreateRenderingResources();
        syncObjects.Recreate();
    }
    if (call != VK_SUCCESS && call != VK_SUBOPTIMAL_KHR)
        Log(ERROR, called + ErrStr(call));

    iCurrentFrame = (iCurrentFrame + 1) % syncObjects.MaxFramesInFlight;
}

void MyApp::handlePrimaryPressDown(int atX, int atY)
{
    // Handle mouse click / touch
}
```

---

## Renderable Setup Patterns

### Static geometry (triangle list, with texture)

```cpp
#include "Vertex3DTypes.h"

Vertex3DTexture vertices[] = {
    { vec3(-1, 0, 5), vec2(0, 0) },
    { vec3(-1, 2, 5), vec2(0, 1) },
    { vec3( 1, 0, 5), vec2(1, 0) },
    { vec3( 1, 2, 5), vec2(1, 1) },
    // ... (6 vertices for 2 triangles, or use an index buffer)
};

VertexDescription<Vertex3DTexture> vertexDesc;
MeshObject mesh { vertexDesc, vertices, N_ELEMENTS_IN_ARRAY(vertices) };

TextureSpec texSpec = {
    .fileName   = "myTexture.png",     // looked up via ExeAccompaniedFullPath + "textures/"
    .filterMode = LINEAR,
    .wrapMode   = REPEAT
};

DrawableProperties props = {
    .mesh     = mesh,
    .name     = "MyObject",
    .shaders  = { { VERTEX, "myshader-vert.spv" }, { FRAGMENT, "myshader-frag.spv" } },
    .pUBOs    = { UBO(VP) },
    .textures = { texSpec }
};
DrawableSpecifier specifier(props);
vulkan.command.renderables.Add(Renderable(specifier, vulkan, platform));
vulkan.command.PostInitPrepBuffers(vulkan);
```

### Dynamic geometry (CPU-updated vertices every frame)

```cpp
DrawableProperties props = {
    .mesh      = mesh,
    .name      = "DynamicObject",
    .shaders   = { { VERTEX, "shader-vert.spv" }, { FRAGMENT, "shader-frag.spv" } },
    .pUBOs     = { UBO(VP) },
    .textures  = { texSpec },
    .customize = DYNAMIC_GEOMETRY    // host-visible vertex buffer for CPU writes
};

// After adding to renderables, retrieve the actual pointer for updates:
const auto& list = vulkan.command.renderables.getNormalRenderables();
iRenderable* pActual = list[list.size() - 1];

// Per frame:
pActual->updateVertexData(vertices, totalBytes, strideBytes);
```

### Procedural texture (no file on disk)

```cpp
ImageInfo imageInfo = {
    .pPixels  = pixelData.data(),
    .numBytes = (VkDeviceSize) pixelData.size(),
    .format   = VK_FORMAT_R8G8B8A8_UNORM,
    .wide     = width,
    .high     = height
};
TextureSpec texSpec = {
    .fileName   = nullptr,       // null = use pImageInfo
    .filterMode = LINEAR,
    .wrapMode   = CLAMP,
    .pImageInfo = &imageInfo
};
// IMPORTANT: pixelData and imageInfo must remain alive for the texture's lifetime.
```

---

## Vertex Types Available

Defined in `Adjunct/VertexTypes/Vertex3DTypes.h`:

| Type | Fields | Use Case |
|------|--------|----------|
| `Vertex3D` | position | Position-only geometry |
| `Vertex3DNormal` | position, normal | Lit geometry |
| `Vertex3DTexture` | position, texCoord | Textured geometry |
| `Vertex3DColor` | position, color | Per-vertex colored |
| `Vertex3DNormalTexture` | position, normal, texCoord | Lit + textured |
| `Vertex3DNormalColor` | position, normal, color | Lit + per-vertex color |
| `Vertex3DTextureColor` | position, texCoord, color | Textured + per-vertex |
| `Vertex3DNormalTextureColor` | position, normal, texCoord, color | Everything |

Each has a static `layout[]` array for automatic vertex attribute binding.

---

## UBO Types Available

Defined in `Adjunct/UniformBufferLiterals.h`:

| Type | Fields | Shader Stage |
|------|--------|-------------|
| `UBO_MVP` | model, view, proj (mat4 each) | Vertex |
| `UBO_VP` | view, proj (mat4 each) | Vertex |
| `UBO_rtm` | resolution (vec4), time (float), mouse (vec4) | Fragment |
| `UBO_Light` | position, color (vec4), ambientStrength (float) | Fragment |
| `UBO_Shadow` | lightSpaceMatrix (mat4) | Vertex |

Wrap with `UBO(myStruct)` in the `.pUBOs` list.

---

## Customizer Flags

Combine with `|` in `.customize`:

| Flag | Effect |
|------|--------|
| `WIREFRAME` | Wireframe rendering |
| `SHOW_BACKFACES` | Disable back-face culling |
| `FRONT_CLOCKWISE` | Swap front-face winding |
| `ALPHA_BLENDING` | Standard alpha blend (src=SRC_ALPHA, dst=ONE_MINUS_SRC_ALPHA) |
| `ADDITIVE_BLENDING` | Additive glow (src=SRC_ALPHA, dst=ONE); disables depth writes |
| `LINE_TOPOLOGY` | Line list primitive |
| `POINT_TOPOLOGY` | Point list primitive |
| `STRIP_TOPOLOGY` | Triangle strip primitive |
| `DYNAMIC_GEOMETRY` | Host-visible vertex buffer for CPU per-frame updates |
| `DEPTH_LEQUAL` | Less-or-equal depth test (default is less) |
| `DISABLE_DEPTH_TEST` | No depth testing |
| `DISABLE_DEPTH_WRITE` | Read depth but don't write |

---

## Shader Conventions

### Vertex shader template (for UBO_VP)

```glsl
#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;    // if using Vertex3DTexture
// layout(location = 1) in vec3 inNormal;   // if using Vertex3DNormal
// etc. — must match your vertex type's layout[] order

layout(location = 0) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}
```

### Fragment shader template (textured)

```glsl
#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
```

### Compiling shaders

```bash
glslangValidator -V myshader.vert -o myshader-vert.spv
glslangValidator -V myshader.frag -o myshader-frag.spv
```

SPV files go in `src/shaders/` and must be copied to `compiledShaders/` next to the executable at runtime. The engine calls `Fatal()` if a shader file is missing (crash = shader not found). A stale SPV from a previous compile will NOT crash — it just renders wrong.

---

## CMakeLists.txt Template

Key differences from TestHarness to note:
- Add `IMGUI_DISABLE` compile definition (unless you have imgui-src)
- Add `Assist/ResourceTracker/` to both sources AND include paths
- Add `Adjunct/Renderables/` via `AUX_SOURCE_DIRECTORY` (not explicit file list)
- Use `file(GLOB)` for shader copy (cmake -E copy doesn't expand wildcards)
- The `VULKAN_MODULE` variable should point to your VulkanModule symlink/directory

```cmake
cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

# Platform detection (copy from TestHarness CMakeLists.txt)
# ... (if/elseif block for WIN32/APPLE/UNIX)

# Compiler settings for APPLE (copy from TestHarness)
# ...

project(MyProject CXX)

set(VULKAN_MODULE "${CMAKE_CURRENT_SOURCE_DIR}/../VulkanModule")
set(CMAKE_PLATFORM_NAME "x64")
set(CMAKE_CONFIGURATION_TYPES "Debug" "Release" CACHE STRING "" FORCE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PROJECT_NAME MyProject)
set(EXECUTABLE_OUTPUT_PATH build)

#################### Source groups ######################

set(MyProject_src
    "src/main.cpp"
    "src/MyApp.cpp"
    "src/AppSettings.cpp"
    # ... your additional source files
)

set(MyProject_include
    "include/AppConstants.h"
    "include/AppSettings.h"
    "include/MyApp.h"
    # ... your additional headers
)

AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Platform/" Platform)
AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Platform/FileSystem/" Platform_FileSystem)
AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Platform/GUISystem/stubs/" Platform_GUISystem)
AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Platform/ImageHandling/" Platform_ImageHandling)
AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Platform/Logger/" Platform_Logger)

set(Platform_OSAbstraction
    "${VULKAN_MODULE}/Platform/OSAbstraction/PlatformSDL.cpp"
)

AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Adjunct/" Vulkan_Adjunct)
AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Adjunct/Renderables/" Vulkan_Adjunct_Renderables)
AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Adjunct/VertexTypes/" Vulkan_Adjunct_VertexTypes)
AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Assist/" Vulkan_Assist)
AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Assist/ResourceTracker/" Vulkan_Assist_ResourceTracker)
AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Objects/" Vulkan_Objects)
AUX_SOURCE_DIRECTORY("${VULKAN_MODULE}/Setup/" Vulkan_Setup)

set(ALL_FILES
    ${MyProject_src}
    ${MyProject_include}
    ${Platform}
    ${Platform_FileSystem}
    ${Platform_GUISystem}
    ${Platform_ImageHandling}
    ${Platform_Logger}
    ${Platform_OSAbstraction}
    ${Vulkan_Adjunct}
    ${Vulkan_Adjunct_Renderables}
    ${Vulkan_Adjunct_VertexTypes}
    ${Vulkan_Assist}
    ${Vulkan_Assist_ResourceTracker}
    ${Vulkan_Objects}
    ${Vulkan_Setup}
)

add_executable(${PROJECT_NAME} ${ALL_FILES})
set(ROOT_NAMESPACE MyProject)

# Find packages
find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

find_path(GLM_INCLUDE_DIR glm/glm.hpp
    HINTS "/opt/homebrew/include" "/usr/include" "/usr/local/include" "$ENV{VULKAN_SDK}/include"
)

# Platform includes (macOS shown; see TestHarness for other platforms)
set(PLATFORM_INCLUDES)
if(APPLE)
    list(APPEND PLATFORM_INCLUDES "/opt/homebrew/include" "/opt/homebrew/include/SDL2")
else()
    list(APPEND PLATFORM_INCLUDES "/usr/include" "/usr/include/SDL2" "/usr/local/include" "/usr/local/include/SDL2")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
    ${PLATFORM_INCLUDES}
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${VULKAN_MODULE}/Setup"
    "${VULKAN_MODULE}/Assist"
    "${VULKAN_MODULE}/Assist/ResourceTracker"
    "${VULKAN_MODULE}/Adjunct"
    "${VULKAN_MODULE}/Adjunct/Renderables"
    "${VULKAN_MODULE}/Adjunct/Shadowing"
    "${VULKAN_MODULE}/Adjunct/VertexTypes"
    "${VULKAN_MODULE}/Objects"
    "${VULKAN_MODULE}/Platform"
    "${VULKAN_MODULE}/Platform/OSAbstraction"
    "${VULKAN_MODULE}/Platform/ImageHandling"
    "${VULKAN_MODULE}/Platform/Logger"
    "${VULKAN_MODULE}/Platform/FileSystem"
    "${VULKAN_MODULE}/Platform/GUISystem"
    "${VULKAN_MODULE}/Platform/GUISystem/stubs"
    "${VULKAN_MODULE}/Platform/GUISystem/replacements"
    "${VULKAN_MODULE}/Platform/ControlScheme"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    "SDL_MAIN_HANDLED"
    "IMGUI_DISABLE"          # Required unless you have External/imgui-src/
)

if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "VK_USE_PLATFORM_METAL_EXT")
elseif(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "_MBCS" "WIN32_LEAN_AND_MEAN" "NOMINMAX" "VK_USE_PLATFORM_WIN32_KHR")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE "VK_USE_PLATFORM_XCB_KHR")
endif()

# Post-build: copy compiled shaders
if(NOT WIN32)
    file(GLOB SHADER_SPV_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.spv")
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/build/compiledShaders"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_SPV_FILES} "${CMAKE_CURRENT_BINARY_DIR}/build/compiledShaders/"
        COMMENT "Copying compiled shaders"
    )
endif()

# Libraries
set(ADDITIONAL_LIBRARY_DEPENDENCIES ${Vulkan_LIBRARIES} ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
if(APPLE)
    list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES "m" "stdc++")
else()
    list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES "m" "stdc++" "pthread" "dl")
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${ADDITIONAL_LIBRARY_DEPENDENCIES})

if(APPLE)
    target_link_directories(${PROJECT_NAME} PRIVATE "/opt/homebrew/lib")
else()
    target_link_directories(${PROJECT_NAME} PRIVATE "/usr/lib" "/usr/local/lib")
endif()

# Compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/Od /Zi> $<$<CONFIG:Release>:/O2 /Oi /Gy> /std:c++20 /W3 /EHsc)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-g -O0> $<$<CONFIG:Release>:-O2 -DNDEBUG> -std=c++20 -Wall -Wno-vla -Wno-reorder-init-list)
    if(NOT APPLE)
        target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
    endif()
endif()

if(NOT MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-s>)
    if(NOT APPLE)
        target_link_options(${PROJECT_NAME} PRIVATE -fPIE)
    endif()
endif()
```

---

## Build and Run

```bash
cd MyProject
mkdir -p build && cd build
cmake ..
cmake --build . -j$(sysctl -n hw.ncpu)
./build/MyProject    # executable is in build/build/ due to EXECUTABLE_OUTPUT_PATH
```

---

## Renderable Lifecycle Rules

1. **`renderables.Add()` copies via `newConcretion()`.** The engine draws the copy, not your original. Retrieve the actual pointer from `getNormalRenderables()` if you need to call `updateVertexData()`.

2. **Do NOT call `Add()` during the render loop.** Pre-initialize all renderable slots in `Init()`. Dead/unused objects should have zero-area vertices (all positions at origin).

3. **Dead objects must zero their vertices AND call `updateVertexData()`.** Otherwise the last frame's geometry persists as a static ghost.

4. **Call `PostInitPrepBuffers()` once after all `Add()` calls**, before the render loop starts.

5. **Do not name your update method `update()`** if you inherit from `iControlScheme` — it conflicts with a virtual method in that base class. Use `updateScene()` or similar.

---

## Debugging Checklist (when nothing renders)

1. Is the clear color visible? If yes, pipeline works. If black, check VulkanSetup construction.
2. Try static geometry: hard-code vertex positions at known world-space locations directly in front of the camera (e.g., `vec3(0, 1, 5)` for camera at `(0, 1, -3)` looking toward +Z). If static works, the issue is in your dynamic vertex computation.
3. Are you using `glm::lookAtLH` and `glm::perspectiveLH_ZO`? The standard versions produce invisible geometry.
4. Is `proj[1][1] *= -1.0f` applied? Without it, the scene is Y-flipped and may be clipped.
5. Are the shader SPV files in `build/build/compiledShaders/`? Check file sizes — a stale SPV won't crash but renders wrong.
6. For billboards: use `cross(direction, normalize(toCamera))` for the side vector, NOT `toCamera - dot(toCamera, direction) * direction`. The latter displaces in depth (invisible).
