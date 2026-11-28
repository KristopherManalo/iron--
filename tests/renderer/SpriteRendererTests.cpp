// SpriteRendererTests.cpp
#include <gtest/gtest.h>

#include <memory>

#include "renderer/SpriteRenderer.h"
#include "renderer/RenderPassManager.h"
#include "Vulkan/VkContext.h"
#include "resources/Buffer.h"
#include "types/CameraUBO.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace RendererTests {

class SpriteRendererTest : public ::testing::Test {
protected:
    GLFWwindow* window = nullptr;

    VkContext ctxt;
    RenderPassManager renderPassManager;

    std::unique_ptr<SpriteRenderer> spriteRenderer;

    std::array<
        std::unique_ptr<Buffer>,
        VkContext::MAX_FRAMES_IN_FLIGHT
    > uniformBuffers;

    uint32_t tempSheet = -1;

    void SetUp() override {
        // GLFW initialization
        ASSERT_TRUE(glfwInit());

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(
            800,
            600,
            "SpriteRenderer Tests",
            nullptr,
            nullptr
        );

        ASSERT_NE(window, nullptr);

        // Vulkan initialization
        ctxt.initialize(window);

        // Render pass initialization
        renderPassManager.initialize(
            ctxt.getDevice(),
            ctxt.getSwapChainImageFormat()
        );

        renderPassManager.getMainRenderPass();

        // Uniform buffers
        createUniformBuffers();

        // Sprite renderer initialization
        spriteRenderer = std::make_unique<SpriteRenderer>(
            ctxt,
            renderPassManager
        );

        // Test sprite sheet
        const std::vector<SpriteUV> atlasSprites = {
            { { 0.025f, 0.025f },{ 0.475f, 0.475f } },
            { { 0.525f, 0.025f },{ 0.975f, 0.475f } },
            { { 0.025f, 0.525f },{ 0.475f, 0.975f } },
            { { 0.525f, 0.525f },{ 0.975f, 0.975f } }
        };

        tempSheet = spriteRenderer->loadSpriteSheet(
            "assets/sprites/test.png",
            atlasSprites
        );

        spriteRenderer->initialize(uniformBuffers);
    }

    void TearDown() override {
        if (spriteRenderer) {
            spriteRenderer->cleanup();
            spriteRenderer.reset();
        }

        // Release uniform buffers
        for (auto& buffer : uniformBuffers) {
            buffer.reset();
        }

        // Clean up remaining Vulkan resources
        renderPassManager.cleanup();
        ctxt.cleanup();

        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }

        glfwTerminate();
    }

    void createUniformBuffers() {
        for (size_t i = 0; i < VkContext::MAX_FRAMES_IN_FLIGHT; i++) {
            uniformBuffers[i] = std::make_unique<Buffer>(
                ctxt,
                sizeof(CameraUBO),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
        };
    }
};

TEST_F(SpriteRendererTest, initializesSuccessfully) {
    EXPECT_NE(spriteRenderer, nullptr);
}

TEST_F(SpriteRendererTest, addSprites) {
    constexpr uint32_t columns = 10;
    constexpr uint32_t rows = 10;
    constexpr float spacing = 0.5f;

    const std::vector<glm::vec4> tints = {
        { 1.0f, 0.5f, 0.5f, 1.0f }, // Pink
        { 0.5f, 1.0f, 0.5f, 1.0f }, // Green
        { 0.5f, 0.5f, 1.0f, 1.0f }, // Blue
        { 1.0f, 1.0f, 0.5f, 1.0f }, // Yellow
        { 1.0f, 0.5f, 1.0f, 1.0f }, // Purple
        { 0.5f, 1.0f, 1.0f, 1.0f }, // Cyan
    };

    //instances.reserve(columns * rows);
    std::vector<SpriteInstance> expectedInstances;
    for (uint32_t y = 0; y < rows; ++y) {
        for (uint32_t x = 0; x < columns; ++x) {
            uint32_t index = y * columns + x;

            glm::vec4 tint = index < 4
                ? glm::vec4(1.0f)
                : tints[(static_cast<unsigned long long>(index) - 4) % tints.size()];

            SpriteInstance instance = {
                    { 4.0f + x * spacing, 1.0f + y * spacing, 0.5f },
                    { 0.5f, 0.5f },
                    0.0f,
                    tint,
                    index % 4
            };

            expectedInstances.push_back(instance);

            auto sprite = spriteRenderer->addSprite(
                tempSheet,
                instance
            );
            EXPECT_EQ(sprite.sheetIndex, 0);
            EXPECT_EQ(sprite.spriteLocation, index);
        }
    }

    auto batchCount = spriteRenderer->getBatchAmount(
        0
    );
    EXPECT_EQ(expectedInstances.size(), batchCount);
    auto batch = spriteRenderer->getBatch(
        0
    );

    ASSERT_NE(batch, nullptr);

    for (size_t j = 0; j < batchCount; ++j) {
        EXPECT_EQ(expectedInstances[j], batch->at(j));
    }
}

}

