#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#include <memory>
#include <string>

#include "Vulkan/VkContext.h"
#include "Renderer/RenderPassManager.h"

class Renderer {
public:
	bool framebufferResized = false;

	void initialize(const char[]);
	void cleanup();
	
	void drawFrame();

	void recreateSwapChain();

	GLFWwindow* getWindow() const;
private:
	const int WIDTH = 800;
	const int HEIGHT = 600;
	std::string programName;

	static constexpr size_t MAX_FRAMES_IN_FLIGHT =
		VkContext::MAX_FRAMES_IN_FLIGHT;
	
	size_t currentFrame = 0;

	std::vector<VkFence> imagesInFlight;

	RenderPassManager renderPassManager;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	void createFramebuffers();
	void cleanupFramebuffers();

	void recordCommandBuffer(
		VkCommandBuffer,
		uint32_t
	);

	static void framebufferResizeCallback(
		GLFWwindow*,
		int width,
		int height
	);


	void initWindow();
	void cleanupWindow();
	struct WindowCleanup {
		void operator()(GLFWwindow* window) const {
			if (window) {
				glfwDestroyWindow(window);
			}
		}
	};
	std::unique_ptr<GLFWwindow, WindowCleanup> window{ nullptr };

	VkContext ctxt;
};


