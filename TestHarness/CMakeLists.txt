cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CPP_COMPILER /usr/bin/clang-cpp)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_LINKER /usr/bin/ld.lld)

project(VulkanModule CXX)


if(NOT CMAKE_PLATFORM_NAME)				# Set target architecture type if empty.
    set(CMAKE_PLATFORM_NAME "x64")
endif()
message("${CMAKE_PLATFORM_NAME} architecture in use")

if(NOT ("${CMAKE_PLATFORM_NAME}" STREQUAL "x64"
     OR "${CMAKE_PLATFORM_NAME}" STREQUAL "x86"))
    message(FATAL_ERROR "${CMAKE_PLATFORM_NAME} arch is not supported!")
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


if("${CMAKE_PLATFORM_NAME}" STREQUAL "x64")		# Include directories
    target_include_directories(${PROJECT_NAME} PRIVATE
		"/opt/homebrew/Cellar/vulkan-headers/1.4.326/include;"
		"/opt/homebrew/Cellar/vulkan-headers/1.4.326/include/vulkan;"
		"/opt/homebrew/Cellar/sdl2/2.32.10/include;"
		"/opt/homebrew/Cellar/sdl2/2.32.10/include/SDL2;"
		"/opt/homebrew/Cellar/sdl2_image/2.8.8/include;"
		"/opt/homebrew/Cellar/sdl2_image/2.8.8/include/SDL2;"
		"/opt/homebrew/Cellar/glm/1.0.1/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Setup;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Assist;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Adjunct;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Adjunct/Renderables;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Adjunct/VertexTypes;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Objects;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Platform;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/OSAbstraction;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/ImageHandling;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/Logger;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/FileSystem;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/GUISystem;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/GUISystem/stubs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Platform/ControlScheme;"
    )
endif()


if("${CMAKE_PLATFORM_NAME}" STREQUAL "x64")		# Compile definitions
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "SDL_MAIN_HANDLED;"
        "_MBCS"
    )
elseif("${CMAKE_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_MBCS"
    )
endif()


if("${CMAKE_PLATFORM_NAME}" STREQUAL "x64")		# Post build events
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND if [ ! -d "build" ]\; then mkdir build\; fi
        COMMAND if [ ! -d "build/compiledShaders" ]\; then mkdir "build/compiledShaders"\; fi
        COMMAND cp src/shaders/*.spv build/compiledShaders/ 2>/dev/null || true
        COMMAND if [ ! -L "build/textures" ]\; then cd build\; ln -s "../../graphics/assets/textures" \; fi
        COMMENT "Post-build events complete."
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


if("${CMAKE_PLATFORM_NAME}" STREQUAL "x64")		# Dependencies
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "vulkan;"
        "SDL2;"
        "SDL2_image;"
        "m;"
        "stdc++"
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PRIVATE
		"/opt/homebrew/Cellar/molten-vk/1.4.0/lib;"
		"/opt/homebrew/Cellar/vulkan-loader/1.4.326/lib;"
		"/opt/homebrew/Cellar/sdl2/2.32.10/lib;"
		"/opt/homebrew/Cellar/sdl2_image/2.8.8/lib;"
    )
endif()

#add_link_options("-mcmodel=medium")

target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od
            >
            $<$<CONFIG:Release>:
                /O2;
                /Oi;
                /Gy
            >
            #/std:c++latest;
            #/sdl;
            #/W3;
            #-fPIC;
            -ggdb3
            -std=c++20
            #^because^ otherwise:  -Wno-c++17-extensions
            #          and for designated initializers
            -m64;
            #-mcmodel=medium;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            -Wno-vla;
            -Wno-reorder-init-list
            # ^also^ avert designated intializer warnings
            #-Wno-c99-designator;
            #-Wno-deprecated-volatile;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )

target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF;
                #-mcmodel=medium
            >
            #/DEBUG:FULL;
            -m64;
            -fPIE;
            #-mcmodel=medium
        )

# END OF FILE

