#pragma once

#include "Vulkan/VkContext.h"

/// @brief Wrapper to create image
class Image {
public:
	Image(
		VkContext& ctxt,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImageAspectFlags aspectFlags
	);

	~Image();

	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;

	void cleanup();

	VkImage get() const { return image; }
	VkImageView getView() const { return imageView; }
private:
	VkContext& ctxt;

	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView imageView = VK_NULL_HANDLE;

	void createImage(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties
	);

	void createImageView(
		VkFormat format,
		VkImageAspectFlags aspectFlags
	);
};