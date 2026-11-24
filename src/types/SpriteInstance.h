#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <cstddef>

struct SpriteInstance {
	glm::vec3 position;
	glm::vec2 scale;
	float rotation;
	glm::vec4 tint;

	static constexpr uint32_t ATTRIBUTE_LOCATION_BASE
		= Vertex::ATTRIBUTE_LOCATION_BASE
		+ Vertex::ATTRIBUTE_COUNT;
	static constexpr uint32_t ATTRIBUTE_COUNT = 4;

	static VkVertexInputBindingDescription
		getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};

		bindingDescription.binding = 1;
		bindingDescription.stride = sizeof(SpriteInstance);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4>
		getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4>
			attributeDescriptions{};

		attributeDescriptions[0].binding = 1;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(SpriteInstance, position);

		attributeDescriptions[1].binding = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(SpriteInstance, scale);

		attributeDescriptions[2].binding = 1;
		attributeDescriptions[2].format = VK_FORMAT_R32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(SpriteInstance, rotation);

		attributeDescriptions[3].binding = 1;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(SpriteInstance, tint);

		for (size_t i = 0; i < ATTRIBUTE_COUNT; ++i) {
			attributeDescriptions[i].location
				= ATTRIBUTE_LOCATION_BASE + i;
		}

		return attributeDescriptions;
	}
};
