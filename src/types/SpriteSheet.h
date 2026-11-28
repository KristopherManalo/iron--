// SpriteSheet.h
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "vulkan/VkContext.h"
#include "resources/Texture.h"
#include "resources/Buffer.h"
#include "types/SpriteInstance.h"

struct SpriteUV {
	glm::vec2 min;
	glm::vec2 max;
};

class SpriteSheet {
public:
	/// @brief Constructs sprite sheet.
	/// @param ctxt Vulkan Context.
	/// @param texturePath Path to sprit sheet atlas.
	/// @param sprites UV definition of spritesheet.
	/// @param maxAmount Maximum amount of instances
	SpriteSheet(
		VkContext& ctxt,
		const std::string& texturePath,
		const std::vector<SpriteUV>& sprites,
		uint32_t maxAmount = 256
	);

	~SpriteSheet();

	/// @brief Cleans up any created resources for spritesheet
	void cleanup();

	/// @brief Returns pointer of texture buffer.
	/// @return Pointer to buffer of texture.
	Texture& getTexture();

	/// @brief Returns pointer of atlas buffer.
	/// @return Pointer to buffer of atlas.
	Buffer& getAtlasBuffer();

	/// @brief Returns atlus buffer size
	/// @return VkDeviceSize of atlus size.
	VkDeviceSize getAtlasBufferSize() const;

	/// @brief Returns amount of sprites in atlas.
	/// @return Number of sprites in atlas.
	uint32_t getSpriteCount() const;

	/// @brief Returns number of instances.
	/// @return Number of instances of this sprite sheet.
	uint32_t getInstanceCount() const;

	/// @brief Returns maximum allowed instance of sprite.
	/// @return Maximum allowed instances.
	uint32_t getMaxInstances() const;

	/// @brief Adds instance of sprite sheet.
	/// @param instance Data about instance.
	/// @return Id of sprite
	uint32_t addSprite(
		const SpriteInstance& instance
	);

	/// @brief Deletes instance of sprite sheet.
	/// @param id Id of sprite
	/// @return True if deletion successful, else false.
	bool deleteSprite(
		uint32_t id 
	);

	SpriteInstance& getSprite(
		uint32_t id
	);
	const SpriteInstance& getSprite(
		uint32_t id
	) const;

	/// @brief Gets index of id in instance vector.
	/// @param id ID of sprite to find
	/// @return Index of location
	std::optional<uint32_t> getSpriteLocation(
		uint32_t id
	) const;

private:
	std::vector<SpriteInstance> sprites;

	// <K, V>
	// K: SpriteID
	// V: spritesInstances array true location
	std::unordered_map<uint32_t, uint32_t> locations;

	std::unique_ptr<Texture> texture;
	std::unique_ptr<Buffer> atlasBuffer;

	uint32_t spriteCount = 0;
	VkDeviceSize atlusBufferSize = 0;

	uint32_t maxInstances = 0;

	uint32_t nextSpriteID = 0;

	void createAtlasBuffer(
		VkContext& ctxt,
		const std::vector<SpriteUV>& 
	);

	/// @brief Gets id. Should guarentee
	/// no id reuse
	/// @return id to use.
	uint32_t getID();
};
