#include "Buffer.h"
#include <stdexcept>
#include <string>


Buffer::Buffer(
	VkContext &ctxt, 
	VkDeviceSize size, 
	VkBufferUsageFlags usage, 
	VkMemoryPropertyFlags properties
) : ctxt(ctxt) {
	VkDevice device = ctxt.getDevice();
	VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	if (vkCreateBuffer(
		device,
		&bufferInfo,
		nullptr,
		&buffer
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer");
	}

	VkMemoryRequirements memRequirements;

	vkGetBufferMemoryRequirements(
		device,
		buffer,
		&memRequirements
	);

	VkMemoryAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = ctxt.findMemoryType(
				memRequirements.memoryTypeBits,
				properties
		),
	};

	if (vkAllocateMemory(
		device,
		&allocInfo,
		nullptr,
		&memory	
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory");
	}

	if (vkBindBufferMemory(
		device,
		buffer, 
		memory, 
		0
	) != VK_SUCCESS) {
		vkFreeMemory(device, memory, nullptr);
		vkDestroyBuffer(device, buffer, nullptr);

		memory = VK_NULL_HANDLE;
		buffer = VK_NULL_HANDLE;

		throw std::runtime_error("Failed to bind buffer memory");
	}
}

Buffer::~Buffer() {
	cleanup();
}

void Buffer::cleanup() {
	VkDevice device = ctxt.getDevice();

	if (buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device, buffer, nullptr);
		buffer = VK_NULL_HANDLE;
	}
	if (memory != VK_NULL_HANDLE) {
		vkFreeMemory(device, memory, nullptr);
		memory = VK_NULL_HANDLE;
	}
}

void Buffer::map(
	const void* data,
	VkDeviceSize size
) {
	void* mapped = nullptr;

	if (vkMapMemory(
		ctxt.getDevice(),
		memory,
		0,
		size,
		0,
		&mapped
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to map buffer memory");
	}

	std::memcpy(mapped, data, static_cast<size_t>(size));

	vkUnmapMemory(ctxt.getDevice(), memory);
}

VkBuffer Buffer::get() const {
	return this->buffer;
}