#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <string>

#include "Vulkan/VkContext.h"

/// @brief Creates main Renderpass
class RenderPassManager {
public:
	void initialize(
		VkDevice device,
		VkFormat swapChainImageFormat
	);
	void cleanup();

	VkRenderPass getMainRenderPass() const {
		return mainRenderPass;
	}
private:
	VkDevice device = VK_NULL_HANDLE;
	VkFormat swapChainImageFormat{};
	VkRenderPass mainRenderPass = VK_NULL_HANDLE;
	void createMainRenderPass();
};
