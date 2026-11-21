#pragma once
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSENATIVE_WIN32
#include <glfw/glfw3native.h>

#include "Vulkan/VkContext.h"
#include "RenderPassManager.h"

class TriangleRenderer {
public:
	TriangleRenderer(
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

	void createPipelineLayout();
	void createGraphicsPipeline();
};