cmake_minimum_required(VERSION 3.25)

project(iron--)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_executable(iron--
    src/main.cpp
    
    src/Application/Application.h
    src/Application/Application.cpp

    src/Renderer/Renderer.h
    src/Renderer/Renderer.cpp
#    src/Game/camera.h

#    src/Renderer/Constants.h
#    src/Renderer/TileRenderer.hpp
#    src/Renderer/TileRenderer.cpp
#    src/Renderer/spr_manager.h
#    src/Renderer/spr_manager.cpp
#    src/Renderer/Sprite.hpp
#    src/Renderer/Vertex.hpp

    
    src/Vulkan/VkContext.h
    src/Vulkan/VkContext.cpp
    
# "src/Renderer/Sprite.cpp" "src/Renderer/TileRenderer.cpp" "src/Renderer/Constants.h")
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

#[[
add_custom_command(TARGET iron-- POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:iron-->/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/sprites
        $<TARGET_FILE_DIR:iron-->/sprites
)
]]
