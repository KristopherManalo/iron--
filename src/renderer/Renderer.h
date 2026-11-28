#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <memory>
#include <string>
#include <fstream>

#include "Vulkan/VkContext.h"
#include "renderer/RenderPassManager.h"
#include "scene/Camera.h"

#include "types/CameraUBO.h"

#include <glm/glm.hpp>

#define DEFAULT_CLEAR_COLOR glm::vec4(0.05f, 0.05f, 0.08f, 1.0f)

#pragma region sub-renderer includes
#include "renderer/TriangleRenderer.h"
#include "renderer/SpriteRenderer.h"
#pragma endregion


/// @brief Renderer class that polls
/// sub-renderers to fill a commandbuffer
class Renderer {
public:
	Camera camera;
	/// @briefInitializes any necesarry
	/// sub-renderers
	Renderer();

	bool framebufferResized = false;

	/// @brief Initializes renderer.
	/// @param progName Name of window
	void initialize(const char progName[]);

	/// @brief Performs cleanup operations.
	void cleanup();
	
	/// @brief Tells renderer to draw a frame.
	/// @param clearColor Background color
	void drawFrame(
		glm::vec4 clearColor = DEFAULT_CLEAR_COLOR
	);

	void recreateSwapChain();

	GLFWwindow* getWindow() const;
private:
	const int WIDTH = 1280;
	const int HEIGHT = 720;

	static constexpr size_t MAX_FRAMES_IN_FLIGHT =
		VkContext::MAX_FRAMES_IN_FLIGHT;
	
	size_t currentFrame = 0;

	std::vector<VkFence> imagesInFlight;

	VkContext ctxt;
	RenderPassManager renderPassManager;

	TriangleRenderer triangleRenderer;
	SpriteRenderer spriteRenderer;

	std::vector<VkFramebuffer> swapChainFramebuffers;


	std::array<std::unique_ptr<Buffer>, MAX_FRAMES_IN_FLIGHT> uniformBuffers;

	void createFramebuffers();
	void cleanupFramebuffers();

	void recordCommandBuffer(
		VkCommandBuffer,
		uint32_t,
		glm::vec4
	);

	static void framebufferResizeCallback(
		GLFWwindow*,
		int width,
		int height
	);


	void createUniformBuffers();
	void updateUniformBuffer(uint32_t currentFrame);
	void cleanupUniformBuffers();

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


};

