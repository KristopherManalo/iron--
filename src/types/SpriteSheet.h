#pragma once

#include <glm/glm.hpp>

#include "vulkan/VkContext.h"
#include "resources/Texture.h"
#include "resources/Buffer.h"

struct SpriteUV {
	glm::vec2 min;
	glm::vec2 max;
};

class SpriteSheet {
public:
	SpriteSheet(
		VkContext& ctxt,
		const std::string& texturePath,
		const std::vector<SpriteUV>& sprites,
		uint32_t max_amount = 256
	);

	~SpriteSheet();

	void cleanup();

	Texture& getTexture();
	Buffer& getAtlasBuffer();

	VkDeviceSize getAtlasBufferSize() const;

	uint32_t getSpriteCount() const;

	uint32_t getMaxInstances() const;
private:

	std::unique_ptr<Texture> texture;
	std::unique_ptr<Buffer> atlasBuffer;

	uint32_t spriteCount = 0;
	VkDeviceSize atlusBufferSize = 0;

	uint32_t maxInstances = 0;

	void createAtlasBuffer(
		VkContext& ctxt,
		const std::vector<SpriteUV>& 
	);
};