#include "Image.h"

#include <stdexcept>

Image::Image(
	VkContext& ctxt,
	uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkImageAspectFlags aspectFlags
) : ctxt(ctxt) {

	createImage(
		width,
		height,format,
		tiling,
		usage,
		properties
	);

	createImageView(format, aspectFlags);
}

Image::~Image() {
	cleanup();
}

void Image::cleanup() {
	VkDevice device = ctxt.getDevice();

	if (imageView != VK_NULL_HANDLE) {
		vkDestroyImageView(device, imageView, nullptr);
		imageView = VK_NULL_HANDLE;
	}

	if (image != VK_NULL_HANDLE) {
		vkDestroyImage(device, image, nullptr);
		image = VK_NULL_HANDLE;
	}

	if (memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, memory, nullptr);
		memory = VK_NULL_HANDLE;
	}
}

void Image::createImage(
	uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties
) {
	VkImageCreateInfo imageInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = {
			width,
			height,
			1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = tiling,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	if (vkCreateImage(
		ctxt.getDevice(),
		&imageInfo,
		nullptr,
		&image
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image");
	}

	VkMemoryRequirements memRequirements;

	vkGetImageMemoryRequirements(
		ctxt.getDevice(),
		image,
		&memRequirements
	);

	VkMemoryAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = ctxt.findMemoryType(
			memRequirements.memoryTypeBits,
			properties
		)
	};

	if (vkAllocateMemory(
		ctxt.getDevice(),
		&allocInfo,
		nullptr,
		&memory
	) != VK_SUCCESS) {
		vkDestroyImage(ctxt.getDevice(), image, nullptr);
		image = VK_NULL_HANDLE;
		throw std::runtime_error("Failed to allocate image memory");
	}

	if (vkBindImageMemory(
		ctxt.getDevice(),
		image,
		memory,
		0
	) != VK_SUCCESS) {
		vkFreeMemory(ctxt.getDevice(), memory, nullptr);
		vkDestroyImage(ctxt.getDevice(), image, nullptr);
		memory = VK_NULL_HANDLE;
		image = VK_NULL_HANDLE;
		throw std::runtime_error("Failed to bind image memory");
	}
}

void Image::createImageView(
	VkFormat format,
	VkImageAspectFlags aspectFlags
) {
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
			.layerCount = 1
		}
	};

	if (vkCreateImageView(
		ctxt.getDevice(),
		&viewInfo,
		nullptr,
		&imageView
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image view");
	}
}