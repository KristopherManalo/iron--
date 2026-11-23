#pragma once
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSENATIVE_WIN32
#include <glfw/glfw3native.h>

#include "Vulkan/VkContext.h"
#include "RenderPassManager.h"

#include "resources/Buffer.h"
#include "resources/Texture.h"

#include <memory>

class SpriteRenderer {
public:
	SpriteRenderer(
		VkContext&,
		RenderPassManager&
	);

	void initialize();
	void cleanup();

	void draw(VkCommandBuffer);
private:
	VkContext& ctxt;
	RenderPassManager& rpm;

	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	std::unique_ptr<Buffer> vertexBuffer;
	std::unique_ptr<Buffer> indexBuffer;

	std::unique_ptr<Texture> texture;

	void createPipelineLayout();
	void createGraphicsPipeline();

	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();

	void copyBuffer(
		VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize size
	);
};