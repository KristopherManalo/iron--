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
#include "types/SpriteUV.h"

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
private:
	const std::vector<SpriteUV> atlasSprites = {
	{ { 0.025f, 0.025f },{ 0.475f, 0.475f } },
	{ { 0.525f, 0.025f },{ 0.975f, 0.475f } },
	{ { 0.025f, 0.525f },{ 0.475f, 0.975f } },
	{ { 0.525f, 0.525f },{ 0.975f, 0.975f } }
	};

	VkContext& ctxt;
	RenderPassManager& rpm;


	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	//VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	std::array<VkDescriptorSet, VkContext::MAX_FRAMES_IN_FLIGHT> descriptorSets;

	std::unique_ptr<Buffer> vertexBuffer;
	std::unique_ptr<Buffer> indexBuffer;
	uint32_t indexCount = 0;
	std::unique_ptr<Buffer> instanceBuffer;
	uint32_t instanceCount = 0;
	std::unique_ptr<Buffer> atlasBuffer;

	std::unique_ptr<Texture> texture;

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

	void createInstanceBuffer(
		const std::vector<SpriteInstance>&
	);

	void createAtlasBuffer(
		//const std::vector<SpriteUV>&
	);
};