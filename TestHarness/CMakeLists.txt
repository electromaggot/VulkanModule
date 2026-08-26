cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

# Platform detection
if(WIN32)
    set(PLATFORM_NAME "Windows")
elseif(APPLE)
    set(PLATFORM_NAME "macOS")
elseif(UNIX AND NOT APPLE)
    if(EXISTS "/proc/device-tree/model")
        file(READ "/proc/device-tree/model" DEVICE_MODEL)
        if(DEVICE_MODEL MATCHES "Raspberry Pi")
            set(PLATFORM_NAME "RaspberryPi")
        else()
            set(PLATFORM_NAME "Linux")
        endif()
    else()
        set(PLATFORM_NAME "Linux")
    endif()
else()
    set(PLATFORM_NAME "Unknown")
endif()

message(STATUS "Building for platform: ${PLATFORM_NAME}")

# Platform-specific compiler settings
if(APPLE)
    set(CMAKE_C_COMPILER /usr/bin/clang)
    set(CMAKE_CPP_COMPILER /usr/bin/clang-cpp)
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
    set(CMAKE_LINKER /usr/bin/ld.lld)
elseif(WIN32)
    # Use default MSVC or specified compiler
else()
    # Use default GCC/Clang for Linux/RaspberryPi
endif()

project(VulkanModule CXX)

if(APPLE)					# Obj-C++ (.mm) support, needed by MacOSFullScreen.mm
    enable_language(OBJCXX)
endif()


# Set target architecture
if(NOT CMAKE_PLATFORM_NAME)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_PLATFORM_NAME "x64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(CMAKE_PLATFORM_NAME "x86")
    else()
        set(CMAKE_PLATFORM_NAME "unknown")
    endif()
endif()

# ARM64 detection for Raspberry Pi 5
if(PLATFORM_NAME STREQUAL "RaspberryPi" AND CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(CMAKE_PLATFORM_NAME "arm64")
endif()

message(STATUS "${CMAKE_PLATFORM_NAME} architecture in use")

# Validate supported architectures
if(NOT ("${CMAKE_PLATFORM_NAME}" STREQUAL "x64"
     OR "${CMAKE_PLATFORM_NAME}" STREQUAL "x86"
     OR "${CMAKE_PLATFORM_NAME}" STREQUAL "arm64"))
    message(FATAL_ERROR "${CMAKE_PLATFORM_NAME} architecture is not supported!")
endif()

set(CMAKE_CONFIGURATION_TYPES				# Global configuration types
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

#include(CMake/Utils.cmake)				# Common utils

include(CMake/GlobalSettingsInclude.cmake OPTIONAL)	# Additional Global Settings(add specific info there)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)		# Use solution folders feature

set(PROJECT_NAME VulkanTester)				# Sub-projects


#################### Source groups ######################

# TestHarness source files in new structure
AUX_SOURCE_DIRECTORY("src" VulkanTester_src)
AUX_SOURCE_DIRECTORY("include" VulkanTester_include)

AUX_SOURCE_DIRECTORY("../Platform/" Platform)
AUX_SOURCE_DIRECTORY("../Platform/FileSystem/" Platform_FileSystem)
AUX_SOURCE_DIRECTORY("../Platform/GUISystem/stubs/" Platform_GUISystem)
set(Platform_ImageHandling			# ImageSDLoptional.cpp is a mutually-exclusive
    "../Platform/ImageHandling/ImageSDL.cpp"	#  stand-in for ImageSDL.cpp (SDL without
)						#  SDL_image) -- never build both, they define
						#  the same class.  SDL_image is REQUIRED below,
						#  so the full implementation is the one to use.
AUX_SOURCE_DIRECTORY("../Platform/Logger/" Platform_Logger)

set(Platform_OSAbstraction
    "../Platform/OSAbstraction/PlatformSDL.cpp"
)
if(APPLE)					# PlatformSDL.cpp calls into these Obj-C++ routines
    list(APPEND Platform_OSAbstraction
        "../Platform/OSAbstraction/MacOSFullScreen.mm"
    )
    set_source_files_properties(		# see "Objective-C memory management (ARC)" in CLAUDE.md
        "../Platform/OSAbstraction/MacOSFullScreen.mm"
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
    )
endif()
source_group("Platform\\OSAbstraction" FILES ${Platform_OSAbstraction})

AUX_SOURCE_DIRECTORY("../Adjunct/" Vulkan_Adjunct)

AUX_SOURCE_DIRECTORY("../Adjunct/Renderables/" Vulkan_Adjunct_Renderables)
source_group("Vulkan\\Adjunct\\Renderables" FILES ${Vulkan_Adjunct_Renderables})

AUX_SOURCE_DIRECTORY("../Adjunct/VertexTypes/" Vulkan_Adjunct_VertexTypes)

AUX_SOURCE_DIRECTORY("../Assist/" Vulkan_Assist)		# note: not recursive, hence...
AUX_SOURCE_DIRECTORY("../Assist/ResourceTracker/" Vulkan_Assist_ResourceTracker)

AUX_SOURCE_DIRECTORY("../Objects/" Vulkan_Objects)

AUX_SOURCE_DIRECTORY("../Setup/" Vulkan_Setup)

set(ALL_FILES
    ${VulkanTester_src}
    ${VulkanTester_include}
    ${Platform}
    ${Platform_FileSystem}
    ${Platform_GUISystem}
    ${Platform_ImageHandling}
    ${Platform_Logger}
    ${Platform_OSAbstraction}
    ${Platform_ControlScheme}
    ${Vulkan_Adjunct}
    ${Vulkan_Adjunct_Renderables}
    ${Vulkan_Adjunct_VertexTypes}
    ${Vulkan_Assist}
    ${Vulkan_Assist_ResourceTracker}
    ${Vulkan_Objects}
    ${Vulkan_Setup}
)

#########################################################


add_executable(${PROJECT_NAME} ${ALL_FILES})		# Target

set(ROOT_NAMESPACE VulkanTester)


# Output directory: the binary lands directly in the build directory it was configured in, so
#	`cmake .. && cmake --build .` from TestHarness/build yields TestHarness/build/VulkanTester.
#	Compiled shaders deploy to shaders/compiled/ beside it, via $<TARGET_FILE_DIR:...> below --
#	which is exactly where FileSystem.cpp looks for them.
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
foreach(OUTPUT_CONFIG ${CMAKE_CONFIGURATION_TYPES})	# multi-config generators (Visual Studio,
    string(TOUPPER ${OUTPUT_CONFIG} OUTPUT_CONFIG_UC)	#  Xcode) otherwise append a per-config
    set_target_properties(${PROJECT_NAME} PROPERTIES	#  subdirectory; keep them uniform so the
        RUNTIME_OUTPUT_DIRECTORY_${OUTPUT_CONFIG_UC} "${CMAKE_CURRENT_BINARY_DIR}"
    )							#  shader deployment path stays predictable
endforeach()

if("${CMAKE_PLATFORM_NAME}" STREQUAL "x64" OR "${CMAKE_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()


# Find required packages
find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# Find GLM
find_path(GLM_INCLUDE_DIR glm/glm.hpp
    HINTS
        "/opt/homebrew/include"  # macOS Homebrew
        "/usr/include"           # Linux
        "/usr/local/include"     # Generic Unix
        "C:/vcpkg/installed/x64-windows/include"  # Windows vcpkg
        "$ENV{VULKAN_SDK}/include"
)

# Platform-specific include directories
set(PLATFORM_INCLUDES)
if(APPLE)
    # macOS Homebrew paths
    list(APPEND PLATFORM_INCLUDES
        "/opt/homebrew/include"
        "/opt/homebrew/include/SDL2"
    )
elseif(WIN32)
    # Windows paths
    list(APPEND PLATFORM_INCLUDES
        "$ENV{VULKAN_SDK}/include"
        "C:/vcpkg/installed/x64-windows/include"
        "C:/vcpkg/installed/x64-windows/include/SDL2"
    )
else()
    # Linux/Raspberry Pi paths
    list(APPEND PLATFORM_INCLUDES
        "/usr/include"
        "/usr/include/SDL2"
        "/usr/local/include"
        "/usr/local/include/SDL2"
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIR}
    ${PLATFORM_INCLUDES}
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Setup"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Assist"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Assist/ResourceTracker"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Adjunct"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Adjunct/Renderables"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Adjunct/VertexTypes"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Objects"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/OSAbstraction"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/ImageHandling"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/Logger"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/FileSystem"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/GUISystem"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/GUISystem/stubs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/ControlScheme"
)


# Platform-specific compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "SDL_MAIN_HANDLED"
    "IMGUI_DISABLE"				# TestHarness builds GUISystem/stubs (above), not
)						#  External/imgui-src, so exclude ImGui globally.

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_MBCS"
        "WIN32_LEAN_AND_MEAN"
        "NOMINMAX"
        "VK_USE_PLATFORM_WIN32_KHR"
    )
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "VK_USE_PLATFORM_METAL_EXT"
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "VK_USE_PLATFORM_XCB_KHR"
    )
endif()


# Cross-platform post-build events will be added below

#################### Shaders ############################
#
# Compile GLSL -> SPIR-V as part of the build, then deploy alongside the binary.
#	Previously this only COPIED prebuilt .spv files.  Since *.spv is gitignored, a fresh clone
#	has none, so the harness died at startup on "Failed to open file ... triangle-vert.spv".
#	find_package(Vulkan) (above) locates glslc for us as Vulkan_GLSLC_EXECUTABLE.
#
#	NOTE: the deployed destination must match SHADER_SUBDIRECTORY in
#	 Platform/FileSystem/FileSystem.cpp ("shaders/compiled/"), which is where the runtime
#	 actually looks.  Keep the two in sync.
#
#	Naming: <base>.<stage> compiles to <base>-<stage>.spv (triangle.vert -> triangle-vert.spv),
#	 matching how shaders are named in the DrawableSpecifier that requests them.

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders/compiled")

file(GLOB SHADER_SOURCES "${SHADER_SOURCE_DIR}/*.vert" "${SHADER_SOURCE_DIR}/*.frag"
                         "${SHADER_SOURCE_DIR}/*.comp" "${SHADER_SOURCE_DIR}/*.geom"
                         "${SHADER_SOURCE_DIR}/*.tesc" "${SHADER_SOURCE_DIR}/*.tese")

if(Vulkan_GLSLC_EXECUTABLE AND SHADER_SOURCES)

    set(SPIRV_BINARIES)
    foreach(SHADER_SOURCE ${SHADER_SOURCES})
        get_filename_component(SHADER_BASE "${SHADER_SOURCE}" NAME_WE)
        get_filename_component(SHADER_TYPE "${SHADER_SOURCE}" LAST_EXT)
        string(SUBSTRING "${SHADER_TYPE}" 1 -1 SHADER_STAGE)		# ".vert" -> "vert"
        set(SPIRV "${SHADER_BINARY_DIR}/${SHADER_BASE}-${SHADER_STAGE}.spv")

        add_custom_command(
            OUTPUT "${SPIRV}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}"
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${SHADER_SOURCE}" -o "${SPIRV}"
            DEPENDS "${SHADER_SOURCE}"
            COMMENT "Compiling shader ${SHADER_BASE}${SHADER_TYPE}"
            VERBATIM
        )
        list(APPEND SPIRV_BINARIES "${SPIRV}")
    endforeach()

    add_custom_target(CompileShaders DEPENDS ${SPIRV_BINARIES})
    add_dependencies(${PROJECT_NAME} CompileShaders)

    add_custom_command(					# deploy where the runtime looks for them
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/compiled"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SPIRV_BINARIES}
                                                     "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/compiled/"
        COMMENT "Deploying compiled shaders"
    )

else()		# No glslc (or no GLSL sources): fall back to whatever prebuilt .spv are lying around.

    message(WARNING "glslc not found -- falling back to prebuilt .spv in ${SHADER_SOURCE_DIR}. "
                    "Since *.spv is gitignored, a clean checkout may have none, in which case "
                    "the harness will fail at startup.  Install the Vulkan SDK to get glslc.")
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/compiled"
        COMMAND ${CMAKE_COMMAND} -E copy "${SHADER_SOURCE_DIR}/*.spv"
                                         "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/compiled/"
        COMMENT "Copying prebuilt shaders (glslc unavailable)"
    )

endif()

# (No texture setup: TestHarness draws an untextured tri-color triangle.  The former symlink
#  here pointed at a nonexistent graphics/assets/texture and was named "texture" besides,
#  whereas TEXTURE_SUBDIRECTORY in FileSystem.cpp is "assets/texture/".  Restore it in that
#  shape, relative to the target file dir, if this harness ever loads a texture.)


#if("${CMAKE_PLATFORM_NAME}" STREQUAL "x64")		# Custom build events
#    add_custom_command(
#        OUTPUT "compiledShaders"
#        COMMAND cd ../graphics/shaders
#        COMMAND compileLinux.sh
#        COMMENT "Shaders compiled."
#    )
#endif()


# Platform-specific library dependencies
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    ${Vulkan_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)		# for MacOSFullScreen.mm
    list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES
        "m"					# (in libSystem here, so a no-op -- kept for symmetry)
        ${COCOA_FRAMEWORK}
    )						# Note: no "stdc++".  Clang maps -lstdc++ to -lc++ on macOS,
							#  and clang++ as the link driver already links libc++ implicitly,
							#  so naming it here just asks for the same library a second time.
elseif(WIN32)
    # Windows-specific libraries
    list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES
        "user32"
        "gdi32"
        "shell32"
    )
else()
    # Linux/Raspberry Pi libraries
    list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES
        "m"
        "stdc++"
        "pthread"
        "dl"
    )
endif()

# pkg-config reports SDL2 as a dependency of SDL2_image, so SDL2 arrives twice: once from
#	SDL2_LIBRARIES and again inside SDL2_IMAGE_LIBRARIES.  CMake preserves repeated entries
#	(ordering can matter for static archives) by re-emitting everything that follows them, so
#	that one duplicate also dragged m/stdc++/Cocoa onto the link line twice -- which the linker
#	then warned about.  These are all shared libraries, whose order is irrelevant, so simply
#	drop the repeats.  (REMOVE_DUPLICATES keeps each item's FIRST occurrence.)
list(REMOVE_DUPLICATES ADDITIONAL_LIBRARY_DEPENDENCIES)

target_link_libraries(${PROJECT_NAME} PRIVATE ${ADDITIONAL_LIBRARY_DEPENDENCIES})

# Platform-specific library directories
if(APPLE)
    target_link_directories(${PROJECT_NAME} PRIVATE
        "/opt/homebrew/lib"
    )
elseif(WIN32)
    target_link_directories(${PROJECT_NAME} PRIVATE
        "$ENV{VULKAN_SDK}/lib"
        "C:/vcpkg/installed/x64-windows/lib"
    )
else()
    target_link_directories(${PROJECT_NAME} PRIVATE
        "/usr/lib"
        "/usr/local/lib"
        "/usr/lib/aarch64-linux-gnu"  # Raspberry Pi 5 ARM64
    )
endif()

#add_link_options("-mcmodel=medium")

# Platform-specific compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2 /Oi /Gy>
        /std:c++20
        /W3
        /EHsc
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2 -DNDEBUG>
        -std=c++20
        -Wall
        -Wno-vla
        -Wno-reorder-init-list
    )

    if(NOT APPLE)
        target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
    endif()

    if(PLATFORM_NAME STREQUAL "RaspberryPi")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -march=armv8-a
            -mtune=cortex-a76
        )
    elseif("${CMAKE_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE -m64)
    endif()
endif()

# Platform-specific linker options
if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/OPT:REF /OPT:ICF>
        $<$<CONFIG:Debug>:/DEBUG:FULL>
    )
else()
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-s>  # Strip symbols in release
    )

    if(NOT APPLE)
        target_link_options(${PROJECT_NAME} PRIVATE -fPIE)
    endif()

    if("${CMAKE_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE -m64)
    endif()
endif()

# END OF FILE

