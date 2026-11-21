#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#include <memory>
#include <string>
#include <fstream>

#include "Vulkan/VkContext.h"
#include "Renderer/RenderPassManager.h"

// Start Sub-renderers
#include "Renderer/TriangleRenderer.h"
//	 End Sub-renderers

class Renderer {
public:
	/// <summary>
	/// Initializes any necesarry
	/// sub-renderers
	/// </summary>
	Renderer();

	bool framebufferResized = false;

	/// <summary>
	/// Initializes renderer
	/// </summary>
	/// <param name="progName">Name of window</param>
	void initialize(const char progName[]);
	void cleanup();
	
	void drawFrame();

	void recreateSwapChain();

	GLFWwindow* getWindow() const;
private:
	const int WIDTH = 800;
	const int HEIGHT = 600;

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


	void initWindow(const char[]);
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

	TriangleRenderer triangleRenderer;
};

