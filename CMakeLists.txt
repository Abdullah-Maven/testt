cmake_minimum_required(VERSION 3.20)
project(System0Engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(S0_BUILD_COOKER "Build the Asset Cooker CLI" ON)
option(S0_BUILD_GAME "Build the Game Executable" ON)
option(S0_USE_VULKAN "Use Vulkan instead of OpenGL (Experimental)" OFF)

# Platform Detection
if(APPLE)
    set(S0_RENDERER "metal")
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
else()
    set(S0_RENDERER "gl")
    find_package(OpenGL REQUIRED)
endif()

# Third Party (Assuming submodules or system install)
find_package(SDL2 REQUIRED)
find_package(bullet REQUIRED)
find_package(miniaudio REQUIRED) # Assuming custom Find module or header-only

# ------------------------------------------------------------------
# Core Library (Shared between Engine and Cooker)
# ------------------------------------------------------------------
add_library(s0_core STATIC
    src/core/Logger.cpp
    src/core/Memory.cpp
    src/math/MathUtils.cpp
)
target_include_directories(s0_core PUBLIC src/)
target_link_libraries(s0_core PRIVATE SDL2::SDL2)

# ------------------------------------------------------------------
# RHI Library (Render Hardware Interface)
# ------------------------------------------------------------------
set(RHI_SOURCES
    src/rhi/GraphicsDevice.cpp
    src/rhi/CommandBuffer.cpp
    src/rhi/Pipeline.cpp
    src/rhi/Texture.cpp
    src/rhi/Buffer.cpp
)

if(APPLE)
    list(APPEND RHI_SOURCES
        src/rhi/metal/MetalDevice.mm
        src/rhi/metal/MetalShaderLibrary.mm
    )
else()
    list(APPEND RHI_SOURCES
        src/rhi/gl/GLDevice.cpp
        src/rhi/gl/GLShaderLoader.cpp
    )
endif()

add_library(s0_rhi STATIC ${RHI_SOURCES})
target_include_directories(s0_rhi PUBLIC src/)
target_link_libraries(s0_rhi PUBLIC s0_core)

if(APPLE)
    target_link_libraries(s0_rhi PRIVATE ${COCOA_FRAMEWORK} ${METAL_FRAMEWORK} ${METALKIT_FRAMEWORK})
else()
    target_link_libraries(s0_rhi PRIVATE OpenGL::GL)
endif()

# ------------------------------------------------------------------
# Engine Library (Renderer, Physics Wrapper, Audio, Loader)
# ------------------------------------------------------------------
add_library(s0_engine STATIC
    src/engine/Application.cpp
    src/engine/World.cpp
    src/renderer/ForwardPlusContext.cpp
    src/renderer/LightGrid.cpp
    src/renderer/ShadowAtlas.cpp
    src/physics/PhysicsWorld.cpp
    src/audio/AudioEngine.cpp
    src/loader/S0DataLoader.cpp
    src/loader/MapParser.cpp
)
target_include_directories(s0_engine PUBLIC src/)
target_link_libraries(s0_engine PUBLIC s0_rhi s0_core bullet miniaudio SDL2::SDL2)

# ------------------------------------------------------------------
# Asset Cooker (CLI Tool)
# ------------------------------------------------------------------
if(S0_BUILD_COOKER)
    add_executable(s0_cooker
        cooker/main.cpp
        cooker/MapParser.cpp
        cooker/CSG.cpp
        cooker/BinaryWriter.cpp
    )
    target_include_directories(s0_cooker PRIVATE src/)
    target_link_libraries(s0_cooker PRIVATE s0_core)
endif()

# ------------------------------------------------------------------
# Game Executable
# ------------------------------------------------------------------
if(S0_BUILD_GAME)
    add_executable(s0_game
        game/main.cpp
        game/PlayerController.cpp
        game/ScriptedSequences.cpp
    )
    target_include_directories(s0_game PRIVATE src/)
    target_link_libraries(s0_game PRIVATE s0_engine)
endif()
