#include "SpriteSheet.h"
#include <iostream>
#include <algorithm>
#include <cassert>

SpriteSheet::SpriteSheet(
	VkContext& ctxt,
	const std::string& texturePath,
	const std::vector<SpriteUV>& sprites,
	uint32_t maxAmount
) {
	if (sprites.empty()) {
		throw std::invalid_argument(
			"SpriteSheet requirest at least one sprite"
		);
	}
	texture = std::make_unique<Texture>(
		ctxt,
		texturePath
	);

	createAtlasBuffer(
		ctxt,
		sprites
	);

	spriteCount = sprites.size();
	maxInstances = maxAmount;
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

uint32_t SpriteSheet::getInstanceCount() const {
	return sprites.size();
}

uint32_t SpriteSheet::getMaxInstances() const {
	return maxInstances;
}

uint32_t SpriteSheet::addSprite(
	const SpriteInstance& instance
) {
	if (sprites.size() >= maxInstances) {
		throw std::length_error("SpriteSheet instance capacity exceeded");
	}
	if (instance.spriteIndex >= spriteCount) {
		throw std::out_of_range("Invalid sprite atlas index");
	}

	uint32_t id = getID();
	uint32_t location = static_cast<uint32_t>(sprites.size());
	sprites.push_back(instance);
	locations.emplace(id, location);

	return id;
}

bool SpriteSheet::deleteSprite(
	uint32_t id
) {
	std::optional<uint32_t> location = getSpriteLocation(id);

	// If id not found
	if (!location.has_value()) {
		return false;
	}

	uint32_t lastIndex = static_cast<uint32_t>(sprites.size() - 1);

	if (*location != lastIndex) {
		auto movedSprite = std::find_if(
			locations.begin(),
			locations.end(),
			[lastIndex](const auto& entry) {
				return entry.second == lastIndex;
			}
		);

		assert(movedSprite != locations.end());
		// The last sprite takes the deleted sprite's position.
		sprites[*location] = std::move(sprites[lastIndex]);

		// Update the moved sprite's location.
		movedSprite->second = *location;
	}

	sprites.pop_back();
	locations.erase(id);

	return true;
}

SpriteInstance& SpriteSheet::getSprite(
	uint32_t id
) {
	std::optional<uint32_t> location = getSpriteLocation(id);

	if (location.has_value()) {
		return sprites[location.value()];
	}
	else {
		throw std::out_of_range("Could not locate sprite ID");
	}
}

std::optional<uint32_t> SpriteSheet::getSpriteLocation(
	uint32_t id
) const {
	auto it = locations.find(id);
	if (it == locations.end()) {
		return std::nullopt;
	}
	return it->second;
}

uint32_t SpriteSheet::getID() {
	uint32_t id = nextSpriteID;

	nextSpriteID++;

	return id;
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
