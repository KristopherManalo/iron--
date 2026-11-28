cmake_minimum_required(VERSION 3.25)

project(iron--)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(CTest)

# ------------------------------------------------------
# Dependencies
# ------------------------------------------------------

add_subdirectory(extern/glfw)
add_subdirectory(extern/glm)

find_package(Vulkan REQUIRED)


# ------------------------------------------------------
# Core library
#       Contains anything not-reliant with Vulkan 
# ------------------------------------------------------

add_library(imm_core STATIC
    src/scene/Scene.h
    src/scene/Scene.cpp
    src/scene/Camera.h
    src/scene/Camera.cpp

    src/misc/stb_image.cpp
)

target_include_directories(imm_core PUBLIC
    src
    extern
)

target_link_libraries(imm_core PUBLIC
    glm
)

# ------------------------------------------------------
# Graphics library 
#       Contains anything reliant with Vulkan
# ------------------------------------------------------

add_library(imm_graphics STATIC
    src/Vulkan/VkContext.h
    src/Vulkan/VkContext.cpp

    src/resources/Buffer.h
    src/resources/Buffer.cpp
    src/resources/Image.h
    src/resources/Image.cpp
    src/resources/Texture.h
    src/resources/Texture.cpp

    src/renderer/Renderer.h
    src/renderer/Renderer.cpp
    src/renderer/RenderPassManager.h
    src/renderer/RenderPassManager.cpp
    src/renderer/TriangleRenderer.h
    src/renderer/TriangleRenderer.cpp
    src/renderer/SpriteRenderer.h
    src/renderer/SpriteRenderer.cpp

    src/types/SpriteSheet.h
    src/types/SpriteSheet.cpp
    src/types/SpriteInstance.h
    src/types/Vertex.h
    src/types/CameraUBO.h

)

target_include_directories(imm_graphics PUBLIC
    src
    extern
)

target_link_libraries(imm_graphics PUBLIC
    Vulkan::Vulkan
    imm_core
    glfw
)

# ------------------------------------------------------
# Application
#       Entry point
# ------------------------------------------------------

add_executable(iron--
    assets/resources.rc
    src/main.cpp
    src/Application/Application.cpp

)

target_link_libraries(iron-- PRIVATE
    imm_graphics
)

# ------------------------------------------------------
# Shaders
# ------------------------------------------------------

find_program(SLANGC slangc REQUIRED)

set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_BINARY_DIR}/shaders")

file(MAKE_DIRECTORY "${SHADER_BINARY_DIR}")

function(compile_slang_shader SOURCE ENTRY_POINT OUTPUT_NAME STAGE)
    set(SOURCE_FILE "${SHADER_SOURCE_DIR}/${SOURCE}")
    set(OUTPUT_FILE "${SHADER_BINARY_DIR}/${OUTPUT_NAME}.spv")

    add_custom_command(
        OUTPUT "${OUTPUT_FILE}"
        COMMAND "${SLANGC}"
            "${SOURCE_FILE}"
            -entry "${ENTRY_POINT}"
            -profile spirv_1_3
            -stage "${STAGE}"
            -target spirv
            -o "${OUTPUT_FILE}"
        DEPENDS "${SOURCE_FILE}"
        VERBATIM
    )

    set(SHADER_OUTPUTS ${SHADER_OUTPUTS} "${OUTPUT_FILE}" PARENT_SCOPE)
endfunction()

compile_slang_shader(triVertex.slang vertexMain triVert vertex)
compile_slang_shader(triFragment.slang fragmentMain triFrag fragment)
compile_slang_shader(sprite.slang vertexMain sprite.vert vertex)
compile_slang_shader(sprite.slang fragmentMain sprite.frag fragment)

add_custom_target(Shaders DEPENDS ${SHADER_OUTPUTS})
add_dependencies(${PROJECT_NAME} Shaders)

# ------------------------------------------------------
# Assets
# ------------------------------------------------------

add_custom_target(CopyAssets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
)

add_dependencies(iron-- CopyAssets)

# ------------------------------------------------------
# Tests
# ------------------------------------------------------

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(tests)

