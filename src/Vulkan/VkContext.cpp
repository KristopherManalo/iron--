#include "Vulkan/VkContext.h"
#include <stdexcept>
#include <iostream>
#include <set>
#include <algorithm>

#define PROGRAM_NAME "Iron--"
#define ENGINE_NAME "Iron--"

void VkContext::initialize(GLFWwindow* window) {
	this->window = window;

	createInstance();
	setupDebugMessenger();

	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();

	createSwapChain();
	createImageViews();

	createCommandPool();
	createCommandBuffers();
	createFrameData();
	createRenderFinishedSemaphores();
}

void VkContext::cleanup() {
	cleanupSwapChain();

	cleanupCommandBuffers();

	for (auto& frame : frameData) {
		vkDestroySemaphore(
			device, frame.imageAvailableSemaphore, nullptr
		);
		vkDestroyFence(
			device, frame.inFlightFence, nullptr
		);
	}
	frameData.clear();

	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(
			instance,
			debugMessenger,
			nullptr
		);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void VkContext::cleanupSwapChain() {
	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	cleanupSwapChainSemaphores();

	vkDestroySwapchainKHR(device, swapChain, nullptr);

	swapChainImageViews.clear();
	swapChain = VK_NULL_HANDLE;
}

void VkContext::recreateSwapChain() {
	createSwapChain();
	createImageViews();
	createRenderFinishedSemaphores();
	
	
}

VkDevice VkContext::getDevice() const {
	return this->device;
}

VkPhysicalDevice VkContext::getPhysicalDevice() const {
	return this->physicalDevice;
}

VkQueue VkContext::getPresentQueue() const {
	return this->presentQueue;
}

VkQueue VkContext::getGraphicsQueue() const {
	return this->graphicsQueue;
}

VkSwapchainKHR VkContext::getSwapChain() const {
	return this->swapChain;
}

const std::vector<VkImageView>& VkContext::getSwapChainImageViews() const {
	return this->swapChainImageViews;
}

VkFormat VkContext::getSwapChainImageFormat() const {
	return this->swapChainImageFormat;
}

VkExtent2D VkContext::getSwapChainExtent() const {
	return this->swapChainExtent;
}

VkCommandPool VkContext::getCommandPool() const {
	return this->commandPool;
}

VkCommandBuffer VkContext::getCommandBuffer(size_t index) const {
	return this->commandBuffers.at(index);
}

const FrameData& VkContext::getFrameData(size_t index) const {
	return frameData.at(index);
}

VkSemaphore VkContext::getRenderFinishedSemaphore(
	size_t imageIndex
) const {
	return renderFinishedSemaphores.at(imageIndex);
}

// -------------------
// BEGIN INITIALIZERS
// -------------------

void VkContext::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error(
			"Validation layers requested, but not available"
		);
	}

	constexpr VkApplicationInfo appInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = PROGRAM_NAME,
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = ENGINE_NAME,
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};

	auto extensions = getRequiredExtensions();
	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
	};

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount =
			static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames =
			validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext =
			(VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance)
		!= VK_SUCCESS) {
		throw std::runtime_error(
			"Error: Failed to create instance");
	}
}

void VkContext::setupDebugMessenger() {
	if (!enableValidationLayers) return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(
		instance, &createInfo, nullptr, &debugMessenger)
		!= VK_SUCCESS) {
		throw std::runtime_error(
			"Error: failed to setup debug messenger");
	}
}

void VkContext::createSurface() {
	if (glfwCreateWindowSurface(
		instance, window, nullptr, &surface)
		!= VK_SUCCESS) {
		throw std::runtime_error(
			"Error: Failed to create window surface"
		);
	}
}

void VkContext::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error(
			"Error: Failed to find GPUs with Vulkan support");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(
		instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	if (enableValidationLayers) {
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		std::cout << "Selected GPU: " 
			<< properties.deviceName << std::endl;
	}
}

void VkContext::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value(),
	};
	float queuePriority = 1.0f;
	//queueCreateInfo.pQueuePriorities = &queuePriority;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{
		.samplerAnisotropy = VK_TRUE,
	};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("Error: failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

	if (graphicsQueue == VK_NULL_HANDLE ||
		presentQueue == VK_NULL_HANDLE) {
		throw std::runtime_error("Failed to retrieve device queues");
	}
}

void VkContext::createSwapChain() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0
		&& imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain)
		!= VK_SUCCESS) {
		throw std::runtime_error("Error: Failed to create swapchain");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void VkContext::createImageViews() {
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); ++i) {
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void VkContext::createCommandPool() {
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
	};

	if (vkCreateCommandPool(
		device, 
		&poolInfo, 
		nullptr, 
		&commandPool
	) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool");
	}
}

void VkContext::createCommandBuffers() {
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	
	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
	};

	if (vkAllocateCommandBuffers(
		device, 
		&allocInfo, 
		commandBuffers.data()
	) != VK_SUCCESS) {
		throw std::runtime_error("Error: Failed to allocate command buffers");
	}
}

void VkContext::cleanupCommandBuffers() {
	if (!commandBuffers.empty()) {
		vkFreeCommandBuffers(
			device,
			commandPool,
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data()
		);
		commandBuffers.clear();
	}
}

void VkContext::cleanupSwapChainSemaphores() {
	for (auto semaphore : renderFinishedSemaphores) {
		vkDestroySemaphore(device, semaphore, nullptr);
	}

	renderFinishedSemaphores.clear();
}

void VkContext::createFrameData() {
	frameData.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO	
	};

	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (vkCreateSemaphore(
				device,
				&semaphoreInfo,
				nullptr,
				&frameData[i].imageAvailableSemaphore
		) != VK_SUCCESS
			|| vkCreateFence(
				device,
				&fenceInfo,
				nullptr,
				&frameData[i].inFlightFence
		) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create sync objects");
		}
	}
}

void VkContext::createRenderFinishedSemaphores() {
	renderFinishedSemaphores.resize(swapChainImages.size());
	VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO	
	};
	for (size_t i = 0; i < swapChainImages.size(); ++i) {
		if (vkCreateSemaphore(
			device,
			&semaphoreInfo,
			nullptr,
			&renderFinishedSemaphores[i]
		) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create sync objects");
		}
	}
}

// -------------------
// END INITIALIZERS
// -------------------


std::vector<const char*> VkContext::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(
		&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions,
		glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

VkSurfaceFormatKHR VkContext::chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& availableFormats
) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VkContext::chooseSwapPresentMode(
	const std::vector<VkPresentModeKHR>& availablePresentModes
) {
	for (const auto& availablePresentMode
		: availablePresentModes) {
		if (availablePresentMode
			== VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VkContext::chooseSwapExtent(
	const VkSurfaceCapabilitiesKHR& capabilities
) {
	if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actualExtent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);

		return actualExtent;
	}
}

VkImageView VkContext::createImageView(
	VkImage image,
	VkFormat format,
	VkImageAspectFlags aspectFlags
) const {
	VkImageViewCreateInfo viewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = {
			.aspectMask = aspectFlags,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view");
	}
	return imageView;
}

VkContext::SwapChainSupportDetails VkContext::querySwapChainSupport(
	VkPhysicalDevice device
) const {
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		device,
		surface,
		&details.capabilities
	);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		device,
		surface,
		&formatCount,
		nullptr
	);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device,
			surface,
			&formatCount,
			details.formats.data()
		);
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device,
		surface,
		&presentModeCount,
		nullptr
	);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device,
			surface,
			&presentModeCount,
			details.presentModes.data()
		);
	}
	return details;
}

VkContext::QueueFamilyIndices VkContext::findQueueFamilies(
	VkPhysicalDevice device
) const {
	QueueFamilyIndices indices;
	VkBool32 presentSupport = false;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(
		device,
		&queueFamilyCount,
		nullptr
	);

	std::vector<VkQueueFamilyProperties> queueFamilies(
		queueFamilyCount
	);
	vkGetPhysicalDeviceQueueFamilyProperties(
		device, 
		&queueFamilyCount, 
		queueFamilies.data()
	);

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		vkGetPhysicalDeviceSurfaceSupportKHR(
			device, 
			i, 
			surface, 
			&presentSupport
		);
		if (presentSupport) {
			indices.presentFamily = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		if (indices.isComplete()) {
			break;
		}
		i++;
	}

	return indices;
}

bool VkContext::checkDeviceExtensionSupport(
	VkPhysicalDevice device
) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(
		device, nullptr, &extensionCount, nullptr
	);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool VkContext::isDeviceSuitable(
	VkPhysicalDevice device
) {
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport
			= querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty()
			&& !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete()
		&& extensionsSupported
		&& swapChainAdequate
		&& supportedFeatures.samplerAnisotropy;
}

bool VkContext::checkValidationLayerSupport() {
	uint32_t layerCount;

	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount,
		availableLayers.data());
	for (const char* layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

VkResult VkContext::CreateDebugUtilsMessengerEXT(
	VkInstance instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
	const VkAllocationCallbacks* pAllocator, 
	VkDebugUtilsMessengerEXT* pDebugMessenger
) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VkContext::DestroyDebugUtilsMessengerEXT(
	VkInstance instance, 
	VkDebugUtilsMessengerEXT debugMessenger, 
	const VkAllocationCallbacks* pAllocator
) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT"
	);
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void VkContext::populateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT& createInfo
) {
	createInfo = {
	.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
	.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
	.pfnUserCallback = debugCallback,
	};
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkContext::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
) {
	std::cerr << "Validation layer: "
		<< pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

