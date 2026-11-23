#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSENATIVE_WIN32
#include <glfw/glfw3native.h>

#include "Vulkan/VkContext.h"

/// @brief Wrapper for Vulkan Buffer
class Buffer {
public:
	Buffer(
		VkContext &ctxt,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties
	);

	~Buffer();

	void cleanup();

	void map(
		const void* data,
		VkDeviceSize size
	);

	VkBuffer get() const;
private:
	VkContext &ctxt;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};