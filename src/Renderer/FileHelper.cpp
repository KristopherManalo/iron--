#pragma once
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSENATIVE_WIN32
#include <glfw/glfw3native.h>

#include <fstream>
#include <vector>
#include <Vulkan/VkContext.h>

static std::vector<char> readFile(
	const std::string& filename
) {
	std::ifstream file(
		filename, 
		(std::ios::ate) | (std::ios::binary)
	);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

static VkShaderModule createShaderModule(
	VkContext &ctxt,
	const std::vector<char>& code
) {
	VkShaderModuleCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(
		ctxt.getDevice(),
		&createInfo,
		nullptr,
		&shaderModule
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module");
	}

	return shaderModule;
}
