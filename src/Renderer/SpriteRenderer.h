#pragma once
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSENATIVE_WIN32
#include <glfw/glfw3native.h>

#include "Vulkan/VkContext.h"
#include "RenderPassManager.h"

#include "resources/Buffer.h"
#include "resources/Texture.h"
#include "types/Vertex.h"
#include "types/SpriteInstance.h"
#include "types/SpriteSheet.h"

#include <memory>

/// @brief Subrenderer that
/// allows 2d images to be rendered
class SpriteRenderer {
public:
	/// @brief Construsts sprite rendering.
	/// @param ctxt Vulkan context.
	/// @param rpm Render pass manager.
	SpriteRenderer(
		VkContext& ctxt,
		RenderPassManager& rpm
	);

	/// @brief Initializes any necesarry 
	/// class variables
	void initialize(
		const std::array<std::unique_ptr<Buffer>, 
			VkContext::MAX_FRAMES_IN_FLIGHT>&
			uniformBuffers
	);

	/// @brief Cleans any class variables
	void cleanup();

	/// @brief Puts commands to draw
	/// into command buffer.
	/// @param commandBuffer Buffer to put commands
	void draw(
		VkCommandBuffer commandBuffer,
		uint32_t currentFrame
	);

	/// @brief Updates the instance buffers for the current frame.
	/// @param currentFrame The index of the current frame.
	void updateInstanceBuffers(
		uint32_t currentFrame
	);

	/// @brief Loads a sprite sheet into sprite renderer
	/// and returns what index to write instances into.
	/// @param texturePath File path to sprite sheet.
	/// @param sprites UV definitions of sprites.
	/// @return Sprite sheet index.
	uint32_t loadSpriteSheet(
		const std::string& texturePath,
		const std::vector<SpriteUV>& sprites
	);

	/// @brief Adds sprite to sprite sheet's instance.
	/// @param sheetIndex Index of sprite sheet.
	/// @param instance Instance info of new sprite.
	/// @return {sheetIndex, spriteLocation}
	SpriteID addSprite(
		uint32_t sheetIndex,
		const SpriteInstance& instance
	);

private:
	
	VkContext& ctxt;
	RenderPassManager& rpm;

	static constexpr std::array<Vertex, 4> vertices{ {
		{ { -0.5f, -0.5f },{ 0.0f, 0.0f } },
		{ { 0.5f, -0.5f },{ 1.0f, 0.0f } },
		{ { 0.5f,  0.5f },{ 1.0f, 1.0f } },
		{ { -0.5f,  0.5f },{ 0.0f, 1.0f } }
	} };

	static constexpr std::array<uint16_t, 6> indices = {
		0, 1, 2,
		2, 3, 0
	};
	static constexpr VkDeviceSize bufferSize = sizeof(indices);



	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::vector<
		std::array<
		VkDescriptorSet,
		VkContext::MAX_FRAMES_IN_FLIGHT
		>
	> descriptorSets;

	std::unique_ptr<Buffer> vertexBuffer;
	std::unique_ptr<Buffer> indexBuffer;
	uint32_t indexCount = 0;
	std::unique_ptr<Buffer> instanceBuffer;
	std::vector<std::array<
		std::unique_ptr<Buffer>,
		VkContext::MAX_FRAMES_IN_FLIGHT
	>> instanceBuffers;
	uint32_t instanceCount = 0;

	std::unique_ptr<Texture> texture;

	std::vector<std::unique_ptr<SpriteSheet>> spriteSheets;

	std::vector<std::vector<SpriteInstance>> instanceBatches;


	void createPipelineLayout();
	void createGraphicsPipeline();

	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets(
		const std::array<std::unique_ptr<Buffer>, 
			VkContext::MAX_FRAMES_IN_FLIGHT>&
	);

	void copyBuffer(
		VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize size
	);

	void createVertexBuffer(
		const std::vector<Vertex>&
	);
	void createIndexBuffer(
		const std::vector<uint16_t>&
	);
	void createVertexBuffer();
	void createIndexBuffer();

	void createInstanceBuffer(
		const std::vector<SpriteInstance>&
	);

	void createInstanceBuffers();
};

