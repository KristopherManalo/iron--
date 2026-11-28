#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSENATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <optional>
#include <vector>
#include <string>
//#include <Renderer/Sprite.hpp>

struct FrameData {
	VkSemaphore imageAvailableSemaphore;
	VkFence inFlightFence;
};

/// @brief Contains Vulkan environment variables
/// required for rendering
class VkContext {
public:
	static constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;

	/// @brief Initialize context.
	///
	/// Initializes all vulkan context variables
	/// required to render in target window
	/// 
	/// @param window Target window
	void initialize(GLFWwindow* window);

	/// Cleans up generated environment variables
	void cleanup();

	void cleanupSwapChain();
	void recreateSwapChain();

#pragma region getters
	VkDevice getDevice() const;
	VkPhysicalDevice getPhysicalDevice() const;

	VkQueue getPresentQueue() const;
	VkQueue getGraphicsQueue() const;

	VkSwapchainKHR getSwapChain() const;
	const std::vector<VkImageView>& getSwapChainImageViews() const;
	VkFormat getSwapChainImageFormat() const;
	VkExtent2D getSwapChainExtent() const;

	VkCommandPool getCommandPool() const;
	VkCommandBuffer getCommandBuffer(size_t index) const;

	const FrameData& getFrameData(size_t index) const;

	VkSemaphore getRenderFinishedSemaphore(size_t imageIndex) const;
#pragma endregion getters

	/// @brief Finds a Vulkan memory type 
	/// supported by the given memory 
	/// requirements and properties.
	///
	/// @param typeFilter Bitmask indicating the memory types supported by the resource.
	/// @param properties Required Vulkan memory property flags for the allocation.
	/// @return The index of a matching memory type, or an implementation-defined invalid index if no match is found.
	uint32_t findMemoryType(
		uint32_t typeFilter, 
		VkMemoryPropertyFlags properties
	) const;
private:

	//bool enableValidationLayers = false;
#ifdef NDEBUG 
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	GLFWwindow* window = nullptr;

	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger{};
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;

	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;

	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	VkExtent2D swapChainExtent{};

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat{};

	VkCommandPool commandPool = VK_NULL_HANDLE;

	std::vector<FrameData> frameData;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkCommandBuffer> commandBuffers;



	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool isComplete() const {
			return graphicsFamily.has_value()
				&& presentFamily.has_value();
		}
	};
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

#pragma region initializers
	void createInstance();

	void setupDebugMessenger();
	
	void createSurface();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createSwapChain();

	void createImageViews();

	void createCommandPool();

	void createCommandBuffers();

	void createFrameData();

	void createRenderFinishedSemaphores();
#pragma endregion initializers

#pragma region cleanup
	void cleanupCommandBuffers();

	void cleanupSwapChainSemaphores();
#pragma endregion cleanup

#pragma region helpers
	std::vector<const char*> getRequiredExtensions() const;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>&
	);

	VkPresentModeKHR chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>&
	);

	VkExtent2D chooseSwapExtent(
		const VkSurfaceCapabilitiesKHR&
	);

	VkImageView createImageView(
		VkImage,
		VkFormat,
		VkImageAspectFlags
	) const;

	SwapChainSupportDetails querySwapChainSupport(
		VkPhysicalDevice
	) const;

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice) const;

	bool checkDeviceExtensionSupport(VkPhysicalDevice);

	bool isDeviceSuitable(VkPhysicalDevice);

	bool checkValidationLayerSupport();

	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	);

	static void DestroyDebugUtilsMessengerEXT(
		VkInstance,
		VkDebugUtilsMessengerEXT,
		const VkAllocationCallbacks*
	);

	void populateDebugMessengerCreateInfo(
		VkDebugUtilsMessengerCreateInfoEXT&	
	);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT,
		VkDebugUtilsMessageTypeFlagsEXT,
		const VkDebugUtilsMessengerCallbackDataEXT*,
		void* pUserData
	);
#pragma endregion helpers
};