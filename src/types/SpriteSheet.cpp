#include "types/SpriteSheet.h"

SpriteSheet::SpriteSheet(
	VkContext& ctxt,
	const std::string& texturePath,
	const std::vector<SpriteUV>& sprites,
	uint32_t max_amount
) {
	texture = std::make_unique<Texture>(
		ctxt,
		texturePath
	);

	createAtlasBuffer(
		ctxt,
		sprites
	);

	spriteCount = sprites.size();
	maxInstances = max_amount;
}

SpriteSheet::~SpriteSheet() {
	this->cleanup();
}

void SpriteSheet::cleanup() {
	texture.reset();
	atlasBuffer.reset();
}

Texture& SpriteSheet::getTexture() {
	return *texture;
}

Buffer& SpriteSheet::getAtlasBuffer() {
	return *atlasBuffer;
}

VkDeviceSize SpriteSheet::getAtlasBufferSize() const {
	return atlusBufferSize;
}

uint32_t SpriteSheet::getSpriteCount() const {
	return spriteCount;
}

uint32_t SpriteSheet::getMaxInstances() const {
	return maxInstances;
}

void SpriteSheet::createAtlasBuffer(
	VkContext& ctxt,
	const std::vector<SpriteUV>& atlasSprites
) {
	atlusBufferSize =
		atlasSprites.size() * sizeof(SpriteUV);

	atlasBuffer = std::make_unique<Buffer>(
		ctxt,
		atlusBufferSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	atlasBuffer->write(
		atlasSprites.data(),
		atlusBufferSize,
		0
	);
}