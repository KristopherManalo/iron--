cmake_minimum_required(VERSION 3.25)

project(iron--)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_executable(iron--
    src/main.cpp
    
    src/Application/Application.h
    src/Application/Application.cpp

    src/Vulkan/VkContext.h
    src/Vulkan/VkContext.cpp

    src/Renderer/Renderer.h
    src/Renderer/Renderer.cpp
    
    
 "src/Renderer/RenderPassManager.h" "src/Renderer/RenderPassManager.cpp")

add_subdirectory(extern/glfw)
add_subdirectory(extern/glm)

find_package(Vulkan REQUIRED)

target_link_libraries(iron-- PRIVATE
    Vulkan::Vulkan
    glm
    glfw
)

target_include_directories(iron-- PRIVATE
    src
    extern
)

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
            -stage "${STAGE}"
            -target spirv
            -o "${OUTPUT_FILE}"
        DEPENDS "${SOURCE_FILE}"
        VERBATIM
    )

    set(SHADER_OUTPUTS ${SHADER_OUTPUTS} "${OUTPUT_FILE}" PARENT_SCOPE)
endfunction()

compile_slang_shader(triangle.slang vertexMain triangle vertex)
compile_slang_shader(fragment.slang fragmentMain fragment fragment)

add_custom_target(Shaders DEPENDS ${SHADER_OUTPUTS})
add_dependencies(${PROJECT_NAME} Shaders)

add_custom_command(TARGET iron-- POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/sprites
        $<TARGET_FILE_DIR:iron-->/sprites
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:iron-->/assets
)
