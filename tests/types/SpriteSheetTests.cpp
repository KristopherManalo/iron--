// SpriteSheetTests.cpp
#include <gtest/gtest.h>

#include <types/SpriteSheet.h>

#include <memory>

namespace TypeTests {

	class SpriteSheetTest : public ::testing::Test {
	protected:
		std::unique_ptr<SpriteSheet> sheet;

		GLFWwindow* window;
		VkContext ctxt;
		void SetUp() override {
			const std::vector<SpriteUV> atlasSprites = {
				{ { 0.025f, 0.025f },{ 0.475f, 0.475f } },
				{ { 0.525f, 0.025f },{ 0.975f, 0.475f } },
				{ { 0.025f, 0.525f },{ 0.475f, 0.975f } },
				{ { 0.525f, 0.525f },{ 0.975f, 0.975f } }
			};

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

			sheet = std::make_unique<SpriteSheet>(
				ctxt,
				"assets/sprites/test.png",
				atlasSprites
			);
		}

		void TearDown() override {
			sheet.reset();
		}

	};

	TEST_F(SpriteSheetTest, initialize) {
		EXPECT_NE(sheet, nullptr);
		EXPECT_NE(&sheet->getTexture(), nullptr);
		EXPECT_NE(&sheet->getAtlasBuffer(), nullptr);

		EXPECT_EQ(sheet->getSpriteCount(), 4);
		EXPECT_EQ(sheet->getMaxInstances(), 256);
		EXPECT_EQ(sheet->getAtlasBufferSize(), 64);
	}

	TEST_F(SpriteSheetTest, addsMultipleSprites) {
		constexpr uint32_t spriteCount = 100;

		for (uint32_t i = 0; i < spriteCount; ++i) {
			SpriteInstance instance{
				.position = glm::vec3(
					static_cast<float>(i),
					static_cast<float>(i + 1),
					0.0f
				),
				.scale = glm::vec2(1.0f, 1.0f),
				.rotation = static_cast<float>(i) * 0.1f,
				.tint = glm::vec4(1.0f),
				.spriteIndex = i % 4
			};

			uint32_t id = sheet->addSprite(instance);

			EXPECT_EQ(id, i);
			auto location = sheet->getSpriteLocation(id);
			ASSERT_TRUE(location.has_value());
			EXPECT_EQ(location.value(), i);
		}

		// Verify all sprites were added.
		EXPECT_EQ(sheet->getInstanceCount(), spriteCount);
	}

	TEST_F(SpriteSheetTest, removeSprite) {
		constexpr uint32_t spriteCount = 100;

		for (uint32_t i = 0; i < spriteCount; ++i) {
			SpriteInstance instance{
				.position = glm::vec3(
					static_cast<float>(i),
					static_cast<float>(i + 1),
					0.0f
				),
				.scale = glm::vec2(1.0f, 1.0f),
				.rotation = static_cast<float>(i) * 0.1f,
				.tint = glm::vec4(1.0f),
				.spriteIndex = i % 4
			};

			uint32_t id = sheet->addSprite(instance);

		}

		EXPECT_EQ(sheet->getInstanceCount(), 100);
		EXPECT_EQ(sheet->getSpriteLocation(99), 99);

		auto instance = sheet->getSprite(99);

		EXPECT_TRUE(sheet->deleteSprite(0));
		EXPECT_EQ(sheet->getInstanceCount(), 99);

		EXPECT_EQ(sheet->getSpriteLocation(0), std::nullopt);

		EXPECT_EQ(sheet->getSpriteLocation(99), 0);

		EXPECT_EQ(sheet->getSprite(99), instance);
	}

	TEST_F(SpriteSheetTest, removeMultipleSprites) {
		constexpr uint32_t spriteCount = 100;

		for (uint32_t i = 0; i < spriteCount; ++i) {
			SpriteInstance instance{
				.position = glm::vec3(
					static_cast<float>(i),
					static_cast<float>(i + 1),
					0.0f
				),
				.scale = glm::vec2(1.0f, 1.0f),
				.rotation = static_cast<float>(i) * 0.1f,
				.tint = glm::vec4(1.0f),
				.spriteIndex = i % 4
			};

			uint32_t id = sheet->addSprite(instance);
		}

		EXPECT_EQ(sheet->getInstanceCount(), spriteCount);

		EXPECT_TRUE(sheet->deleteSprite(99));

		EXPECT_TRUE(sheet->deleteSprite(67));

		EXPECT_TRUE(sheet->deleteSprite(25));

		EXPECT_EQ(sheet->getInstanceCount(), spriteCount - 3);

		EXPECT_EQ(sheet->getSpriteLocation(99), std::nullopt);
		EXPECT_EQ(sheet->getSpriteLocation(67), std::nullopt);
		EXPECT_EQ(sheet->getSpriteLocation(25), std::nullopt);
	}

	TEST_F(SpriteSheetTest, deleteInvalidID) {
		EXPECT_EQ(sheet->getInstanceCount(), 0);
		for (int i = 0; i < sheet->getMaxInstances(); ++i) {
			EXPECT_EQ(sheet->deleteSprite(i), false);
		}
		EXPECT_EQ(sheet->deleteSprite(UINT32_MAX), false);

		constexpr uint32_t spriteCount = 5;

		for (uint32_t i = 0; i < spriteCount; ++i) {
			SpriteInstance instance{
				.position = glm::vec3(static_cast<float>(i), 0.0f, 0.0f),
				.scale = glm::vec2(1.0f),
				.rotation = 0.0f,
				.tint = glm::vec4(1.0f),
				.spriteIndex = i % 4
			};

			sheet->addSprite(instance);
		}

		EXPECT_EQ(sheet->deleteSprite(UINT32_MAX), false);
	}

	TEST_F(SpriteSheetTest, findInvalidID) {
		EXPECT_EQ(sheet->getInstanceCount(), 0);

		for (int i = 0; i < sheet->getMaxInstances(); ++i) {
			EXPECT_EQ(sheet->getSpriteLocation(i), std::nullopt);
		}
		EXPECT_EQ(sheet->getSpriteLocation(UINT32_MAX), std::nullopt);
	}

	TEST_F(SpriteSheetTest, removeMiddleSprite) {
		constexpr uint32_t spriteCount = 5;

		for (uint32_t i = 0; i < spriteCount; ++i) {
			SpriteInstance instance{
				.position = glm::vec3(static_cast<float>(i), 0.0f, 0.0f),
				.scale = glm::vec2(1.0f),
				.rotation = 0.0f,
				.tint = glm::vec4(1.0f),
				.spriteIndex = i % 4
			};

			sheet->addSprite(instance);
		}

		EXPECT_TRUE(sheet->deleteSprite(1));

		EXPECT_EQ(sheet->getInstanceCount(), 4);

		EXPECT_EQ(sheet->getSpriteLocation(0), 0);
		EXPECT_EQ(sheet->getSpriteLocation(1), std::nullopt);
		EXPECT_EQ(sheet->getSpriteLocation(2), 2);
		EXPECT_EQ(sheet->getSpriteLocation(3), 3);
		EXPECT_EQ(sheet->getSpriteLocation(4), 1);
	}

	TEST_F(SpriteSheetTest, removeOnlySprite) {
		SpriteInstance instance{
			.position = { 0.0f, 0.0f , 0.0f},
			.scale = { 1.0f, 1.0f },
			.rotation = 0.0f,
			.tint = glm::vec4(1.0f),
			.spriteIndex = 0
		};
		sheet->addSprite(
			instance
		);

		EXPECT_TRUE(sheet->deleteSprite(0));

		EXPECT_EQ(sheet->getInstanceCount(), 0);
		EXPECT_EQ(sheet->getSpriteLocation(0), std::nullopt);
	}

	TEST_F(SpriteSheetTest, addsSpriteAfterDeletion) {
		// Test creates ids:
		// 0, 1, 2 -> locations: 0, 1, 2
		// Remove id 2. => location 2 is now open
		// new added sprite should have location 2
		constexpr uint32_t spriteCount = 3;

		for (uint32_t i = 0; i < spriteCount; ++i) {
			SpriteInstance instance{
				.position = glm::vec3(static_cast<float>(i), 0.0f, 0.0f),
				.scale = glm::vec2(1.0f),
				.rotation = 0.0f,
				.tint = glm::vec4(1.0f),
				.spriteIndex = i % 4
			};

			sheet->addSprite(instance);
		}

		EXPECT_TRUE(sheet->deleteSprite(1));
		EXPECT_EQ(sheet->getInstanceCount(), 2);

		SpriteInstance instance{
			.position = glm::vec3(10.0f, 10.0f, 0.0f),
			.scale = glm::vec2(1.0f),
			.rotation = 0.0f,
			.tint = glm::vec4(1.0f),
			.spriteIndex = 0
		};

		uint32_t id = sheet->addSprite(instance);

		EXPECT_EQ(id, 3);
		EXPECT_EQ(sheet->getInstanceCount(), 3);
		EXPECT_EQ(sheet->getSpriteLocation(id), 2);
	}
}