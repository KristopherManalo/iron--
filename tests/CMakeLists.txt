add_executable(RendererTests
    "renderer/SpriteRendererTests.cpp"
)

target_link_libraries(RendererTests PRIVATE
    GTest::gtest_main
    imm_graphics
)

add_executable(TypesTests
    "types/SpriteSheetTests.cpp"
)

target_link_libraries(TypesTests PRIVATE
    GTest::gtest_main
    imm_graphics
)

# Ensure shaders are compiled before running/building tests
add_dependencies(RendererTests
	Shaders
    CopyAssets
)

add_dependencies(TypesTests
    CopyAssets
)

# Google Test discovery
include(GoogleTest)

gtest_discover_tests(RendererTests PROPERTIES
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

gtest_discover_tests(TypesTests PROPERTIES
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" 
)

# Visual Studio debugger working directory
set_target_properties(RendererTests PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)