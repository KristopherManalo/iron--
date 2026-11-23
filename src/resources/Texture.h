#pragma once

#include <memory>
#include <string>

#include "Vulkan/VkContext.h"
#include "resources/Image.h"
#include "resources/Buffer.h"

/// @brief Wrapper to create Texture
class Texture {
public:
	Texture(
		VkContext& ctxt,
		const std::string& path
	);
	~Texture();

	void cleanup();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	VkImageView getImageView() const {
		return image->getView();
	}

	VkSampler getSampler() const {
		return sampler;
	}
private:
	VkContext& ctxt;

	std::unique_ptr<Image> image;
	VkSampler sampler = VK_NULL_HANDLE;

	void load(const std::string& path);
	void createSampler();
	void transitionImageLayout(
		VkImageLayout oldLayout,
		VkImageLayout newLayout
	);
	void copyBufferToImage(
		VkBuffer buffer,
		uint32_t width,
		uint32_t height
	);
};