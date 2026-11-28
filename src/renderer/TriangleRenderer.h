#pragma once
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSENATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Vulkan/VkContext.h"
#include "RenderPassManager.h"

/// @brief Sample sub-renderer
/// Displays simple triangle on 
/// view window
class TriangleRenderer {
public:
	/// @brief Constructs triangle rendering
	/// @param ctxt Vulkan context.
	/// @param rpm Render pass manager.
	TriangleRenderer(
		VkContext& ctxt,
		RenderPassManager& rpm
	);

	/// @brief Initializesany necesarry
	/// class variables
	void initialize();

	/// @brief Cleans any class variables
	void cleanup();

	/// @brief Puts commands to draw
	/// into command buffer.
	/// @param  
	void draw(VkCommandBuffer);
private:
	VkContext& ctxt;
	RenderPassManager& rpm;

	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	void createPipelineLayout();
	void createGraphicsPipeline();
};