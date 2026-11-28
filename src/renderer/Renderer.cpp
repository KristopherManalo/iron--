#include "Renderer.h"
#include <stdexcept>
#include <iostream>
#include <chrono>
#include "scene/Camera.h"

Renderer::Renderer() :
	camera(16.0f, 9.0f),
	triangleRenderer(ctxt, renderPassManager),
	spriteRenderer(ctxt, renderPassManager) {
}

void Renderer::initialize(const char progName[]) {
	initWindow(progName);
	ctxt.initialize(window.get());

	renderPassManager.initialize(
		ctxt.getDevice(),
		ctxt.getSwapChainImageFormat()
	);

	renderPassManager.getMainRenderPass();

	createFramebuffers();

	imagesInFlight.assign(
		ctxt.getSwapChainImageViews().size(),
		VK_NULL_HANDLE
	);

	createUniformBuffers();

	triangleRenderer.initialize();

	const std::vector<SpriteUV> atlasSprites = {
		{ { 0.025f, 0.025f },{ 0.475f, 0.475f } },
		{ { 0.525f, 0.025f },{ 0.975f, 0.475f } },
		{ { 0.025f, 0.525f },{ 0.475f, 0.975f } },
		{ { 0.525f, 0.525f },{ 0.975f, 0.975f } }
	};

	uint32_t temp_sheet
		= spriteRenderer.loadSpriteSheet(
		"assets/sprites/test.png",
		atlasSprites
	);
	spriteRenderer.initialize(
		uniformBuffers
	);
	std::vector<SpriteInstance> instances;
	constexpr uint32_t columns = 10;
	constexpr uint32_t rows = 10;
	constexpr float spacing = 0.5f;

	const std::vector<glm::vec4> tints = {
		{ 1.0f, 0.5f, 0.5f, 1.0f }, // Pink
		{ 0.5f, 1.0f, 0.5f, 1.0f }, // Green
		{ 0.5f, 0.5f, 1.0f, 1.0f }, // Blue
		{ 1.0f, 1.0f, 0.5f, 1.0f }, // Yellow
		{ 1.0f, 0.5f, 1.0f, 1.0f }, // Purple
		{ 0.5f, 1.0f, 1.0f, 1.0f }, // Cyan
	};

	//instances.reserve(columns * rows);

	for (uint32_t y = 0; y < rows; ++y) {
		for (uint32_t x = 0; x < columns; ++x) {
			uint32_t index = y * columns + x;

			glm::vec4 tint = index < 4
				? glm::vec4(1.0f)
				: tints[(static_cast<unsigned long long>(index) - 4) % tints.size()];

			spriteRenderer.addSprite(
				temp_sheet,
				{
					{ 4.0f + x * spacing, 1.0f + y * spacing, 0.5f },
					{ 0.5f, 0.5f },
					0.0f,
					tint,
					index % 4
				}
			);
		}
	}
}

void Renderer::cleanup() {
	if (ctxt.getDevice() != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(ctxt.getDevice());
	}
	spriteRenderer.cleanup();
	triangleRenderer.cleanup();
	cleanupUniformBuffers();
	cleanupFramebuffers();
	renderPassManager.cleanup();
	ctxt.cleanup();
	cleanupWindow();
}

void Renderer::recreateSwapChain() {
	int width = 0;
	int height = 0;

	glfwGetFramebufferSize(window.get(), &width, &height);

	while (width == 0 || height == 0) {
		if (glfwWindowShouldClose(window.get())) {
			return;
		}

		glfwWaitEvents();
		glfwGetFramebufferSize(window.get(), &width, &height);
	}

	if (vkDeviceWaitIdle(ctxt.getDevice()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to wait for device idle");
	}

	cleanupFramebuffers();

	ctxt.cleanupSwapChain();
	ctxt.recreateSwapChain();

	createFramebuffers();

	imagesInFlight.assign(
		ctxt.getSwapChainImageViews().size(),
		VK_NULL_HANDLE
	);

	framebufferResized = false;
}

GLFWwindow* Renderer::getWindow() const {
	return window.get();
}


void Renderer::framebufferResizeCallback(
	GLFWwindow* window,
	int width,
	int height
) {
	auto* renderer = static_cast<Renderer*>(
		glfwGetWindowUserPointer(window)
	);

	renderer->framebufferResized = true;
}

void Renderer::createFramebuffers() {
	const auto& imageViews = ctxt.getSwapChainImageViews();
	VkExtent2D extent = ctxt.getSwapChainExtent();

	swapChainFramebuffers.resize(imageViews.size());

	for (size_t i = 0; i < imageViews.size(); i++) {
		VkImageView attachments[] = {
			imageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPassManager.getMainRenderPass(),
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = extent.width,
			.height = extent.height,
			.layers = 1
		};

		if (vkCreateFramebuffer(
			ctxt.getDevice(),
			&framebufferInfo,
			nullptr,
			&swapChainFramebuffers[i]
		) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void Renderer::cleanupFramebuffers() {
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(
			ctxt.getDevice(),
			framebuffer,
			nullptr
		);
	}

	swapChainFramebuffers.clear();
}

void Renderer::drawFrame(
	glm::vec4 clearColor
) {
	VkDevice device = ctxt.getDevice();

	const FrameData& frame =
		ctxt.getFrameData(currentFrame);

	// Wait for frame's previous gpu work
	if (vkWaitForFences(
		device,
		1,
		&frame.inFlightFence,
		VK_TRUE,
		UINT64_MAX
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed waiting for frame fence");
	}

	// Acquire a swapchain image
	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(
		device,
		ctxt.getSwapChain(),
		UINT64_MAX,
		frame.imageAvailableSemaphore,
		VK_NULL_HANDLE,
		&imageIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swapchain image");
	}
	bool suboptimal = result == VK_SUBOPTIMAL_KHR;

	// Wait if image is still being used by another frame
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		if (vkWaitForFences(
			device,
			1,
			&imagesInFlight[imageIndex],
			VK_TRUE,
			UINT64_MAX
		) != VK_SUCCESS) {
			throw std::runtime_error(
				"Failed waiting for image fence!"
			);
		}
	}
	imagesInFlight[imageIndex] = frame.inFlightFence;

	// Reset command buffer
	VkCommandBuffer commandBuffer =
		ctxt.getCommandBuffer(currentFrame);

	if (vkResetCommandBuffer(commandBuffer, 0) != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to reset command buffer!"
		);
	}

	updateUniformBuffer(currentFrame);
	
	spriteRenderer.updateInstanceBuffers(currentFrame);

	// Record rendering commands
	recordCommandBuffer(
		commandBuffer, 
		imageIndex,
		clearColor
	);

	// Reset fence 
	if (vkResetFences(device, 1, &frame.inFlightFence)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to reset frame fence!");
	}

	// Submit command buffer
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	VkSemaphore signalSemaphore =
		ctxt.getRenderFinishedSemaphore(imageIndex);

	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &frame.imageAvailableSemaphore,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &signalSemaphore
	};

	if (vkQueueSubmit(
		ctxt.getGraphicsQueue(),
		1,
		&submitInfo,
		frame.inFlightFence
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command!");
	}


	// Present image
	VkSwapchainKHR swapChain = ctxt.getSwapChain();

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &signalSemaphore,
		.swapchainCount = 1,
		.pSwapchains = &swapChain,
		.pImageIndices = &imageIndex
	};

	result = vkQueuePresentKHR(
		ctxt.getPresentQueue(),
		&presentInfo
	);

	if (result != VK_SUCCESS &&
		result != VK_SUBOPTIMAL_KHR &&
		result != VK_ERROR_OUT_OF_DATE_KHR) {
		throw std::runtime_error("Failed to present swapchain image!");
	}

	if (framebufferResized ||
		suboptimal ||
		result == VK_ERROR_OUT_OF_DATE_KHR ||
		result == VK_SUBOPTIMAL_KHR) {

		framebufferResized = false;
		recreateSwapChain();
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::recordCommandBuffer(
	VkCommandBuffer commandBuffer,
	uint32_t imageIndex,
	glm::vec4 clearColor
) {
	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
	};

	if (vkBeginCommandBuffer(
		commandBuffer,
		&beginInfo
	) != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to begin recording command buffer!"
		);
	}

	VkClearValue clearValue{
		.color = {{0.05f, 0.05f, 0.08f, 1.0f}}
	};

	VkRenderPassBeginInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = renderPassManager.getMainRenderPass(),
		.framebuffer = swapChainFramebuffers.at(imageIndex),
		.renderArea = {
			.offset = {0, 0},
			.extent = ctxt.getSwapChainExtent()
		},
		.clearValueCount = 1,
		.pClearValues = &clearValue
	};

	vkCmdBeginRenderPass(
		commandBuffer,
		&renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE
	);

	// Future:
	// meshRenderer.draw(commandBuffer);
	triangleRenderer.draw(commandBuffer);
	spriteRenderer.draw(commandBuffer, currentFrame);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error(
			"Failed to record command buffer!"
		);
	}
}

void Renderer::createUniformBuffers() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		uniformBuffers[i] = std::make_unique<Buffer>(
			ctxt,
			sizeof(CameraUBO),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
	};
}
void Renderer::updateUniformBuffer(
	uint32_t currentFrame
) {
	CameraUBO ubo{};
	
	ubo.view = camera.getViewMatrix();
	ubo.projection = camera.getProjectionMatrix();


	uniformBuffers[currentFrame]->write(
		&ubo,
		sizeof(ubo),
		0
	);
}
void Renderer::cleanupUniformBuffers() {
	for (auto& buffer : uniformBuffers) {
		buffer.reset();
	}
}


void Renderer::initWindow(const char progName[]) {
	if (glfwInit() != GLFW_TRUE) {
		throw std::runtime_error("Failed to initialize GLFW");
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* rawWindow = glfwCreateWindow(
		WIDTH, HEIGHT, progName, nullptr, nullptr
	);

	if (!rawWindow) {
		glfwTerminate();
		throw std::runtime_error("Failed to create window");
	}

	window.reset(rawWindow);

	glfwSetWindowUserPointer(window.get(), this);

	glfwSetFramebufferSizeCallback(
		window.get(),
		framebufferResizeCallback
	);
}

void Renderer::cleanupWindow() {
	window.reset();
	glfwTerminate();
}

