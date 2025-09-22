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

set(EXECUTABLE_OUTPUT_PATH build)			# sub-directory destination for built binary


#################### Source groups ######################

# TestHarness source files in new structure
AUX_SOURCE_DIRECTORY("src" VulkanTester_src)
AUX_SOURCE_DIRECTORY("include" VulkanTester_include)

AUX_SOURCE_DIRECTORY("../Platform/" Platform)
AUX_SOURCE_DIRECTORY("../Platform/FileSystem/" Platform_FileSystem)
AUX_SOURCE_DIRECTORY("../Platform/GUISystem/stubs/" Platform_GUISystem)
AUX_SOURCE_DIRECTORY("../Platform/ImageHandling/" Platform_ImageHandling)
AUX_SOURCE_DIRECTORY("../Platform/Logger/" Platform_Logger)

set(Platform_OSAbstraction
    "../Platform/OSAbstraction/PlatformSDL.cpp"
)
source_group("Platform\\OSAbstraction" FILES ${Platform_OSAbstraction})

AUX_SOURCE_DIRECTORY("../Adjunct/" Vulkan_Adjunct)

set(Vulkan_Adjunct_Renderables
    "../Adjunct/Renderables/AddOns.cpp"
    "../Adjunct/Renderables/FixedRenderable.cpp"
)
source_group("Vulkan\\Adjunct\\Renderables" FILES ${Vulkan_Adjunct_Renderables})

AUX_SOURCE_DIRECTORY("../Adjunct/VertexTypes/" Vulkan_Adjunct_VertexTypes)

AUX_SOURCE_DIRECTORY("../Assist/" Vulkan_Assist)

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
    ${Vulkan_Objects}
    ${Vulkan_Setup}
)

#########################################################


add_executable(${PROJECT_NAME} ${ALL_FILES})		# Target

set(ROOT_NAMESPACE VulkanTester)


if("${CMAKE_PLATFORM_NAME}" STREQUAL "x64")		# Output directory
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/build/${CMAKE_PLATFORM_NAME}/$<CONFIG>/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build/${CMAKE_PLATFORM_NAME}/$<CONFIG>/"
    )
endif()
if("${CMAKE_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/build/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build/$<CONFIG>/"
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_PLATFORM_NAME}" STREQUAL "x86")
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
)

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

# Create build directories
if(WIN32)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/compiledShaders"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.spv" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/compiledShaders/" 2>NUL || echo "Shaders copied"
        COMMENT "Windows: Setting up shader directory"
    )
else()
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "build"
        COMMAND ${CMAKE_COMMAND} -E make_directory "build/compiledShaders"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.spv" "build/compiledShaders/" 2>/dev/null || true
        COMMENT "Unix: Setting up shader directory"
    )
endif()

# Platform-specific resource setup
if(NOT WIN32)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND test -L "build/textures" || (cd build && ln -s "../../graphics/assets/textures" 2>/dev/null || true)
        COMMENT "Creating texture symlink"
    )
endif()


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
    list(APPEND ADDITIONAL_LIBRARY_DEPENDENCIES
        "m"
        "stdc++"
    )
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

