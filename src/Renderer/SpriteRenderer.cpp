#include <iostream>
#include <vector>

#include "SpriteRenderer.h"
#include "renderer/FileHelper.h"

#include "types/Vertex.h"
#include "types/SpriteInstance.h"
#include "types/CameraUBO.h"

SpriteRenderer::SpriteRenderer(
	VkContext& ctxt,
	RenderPassManager& rpm
) : ctxt(ctxt), rpm(rpm) {
}

void SpriteRenderer::initialize(
	const std::array<std::unique_ptr<Buffer>, 
		VkContext::MAX_FRAMES_IN_FLIGHT>& 
		uniformBuffers
) {
	createVertexBuffer();
	createIndexBuffer();

	createInstanceBuffers();

	createDescriptorSetLayout();
	createDescriptorPool();
	createDescriptorSets(
		uniformBuffers
	);

	createPipelineLayout();
	createGraphicsPipeline();
}

void SpriteRenderer::cleanup() {
	VkDevice device = ctxt.getDevice();

	// Graphics pipeline resources
	if (graphicsPipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		graphicsPipeline = VK_NULL_HANDLE;
	}

	if (pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		pipelineLayout = VK_NULL_HANDLE;
	}

	// Descriptor resources
	if (descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		descriptorPool = VK_NULL_HANDLE;
	}

	descriptorSets.clear();

	if (descriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			nullptr
		);
		descriptorSetLayout = VK_NULL_HANDLE;
	}

	// Per-sheet instance buffers
	instanceBuffers.clear();

	// CPU-side instance batches
	instanceBatches.clear();

	// Sprite sheets (textures and atlas buffers)
	spriteSheets.clear();

	// Shared geometry buffers
	vertexBuffer.reset();
	indexBuffer.reset();
}

void SpriteRenderer::draw(
	VkCommandBuffer commandBuffer,
	uint32_t currentFrame
) {
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		graphicsPipeline
	);

	VkBuffer vertexBuffers[] = {
		vertexBuffer->get(),
		nullptr
	};

	VkDeviceSize offsets[] = { 0, 0 };

	for (size_t sheetIndex = 0;
		sheetIndex < spriteSheets.size();
		++sheetIndex) {
		
		auto& batch = instanceBatches[sheetIndex];

		// if sheet has no sprites
		if (batch.empty()) {
			continue;
		}

		// Bind the sheet's descriptor set for current frame
		VkDescriptorSet descriptorSet =
			descriptorSets[sheetIndex][currentFrame];

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&descriptorSet,
			0,
			nullptr
		);

		// Bind this sheet's instance buffer.
		vertexBuffers[1] =
			instanceBuffers[sheetIndex][currentFrame]->get();

		vkCmdBindVertexBuffers(
			commandBuffer,
			0,
			2,
			vertexBuffers,
			offsets
		);

		vkCmdBindIndexBuffer(
			commandBuffer,
			indexBuffer->get(),
			0,
			VK_INDEX_TYPE_UINT16
		);

		vkCmdDrawIndexed(
			commandBuffer,
			6,
			static_cast<uint32_t>(batch.size()),
			0,
			0,
			0
		);
	}
}

uint32_t SpriteRenderer::loadSpriteSheet(
	const std::string& texturePath,
	const std::vector<SpriteUV>& sprites
) {
	spriteSheets.push_back(
		std::make_unique<SpriteSheet>(
			ctxt,
			texturePath,
			sprites
		)
	);
	instanceBatches.emplace_back();
	return static_cast<uint32_t>(spriteSheets.size() - 1);
}

SpriteID SpriteRenderer::addSprite(
	uint32_t sheetIndex,
	const SpriteInstance& instance
) {
	instanceBatches[sheetIndex].push_back(instance);
	return SpriteID{ 
		sheetIndex,
		static_cast<uint32_t>(instanceBatches[sheetIndex].size() - 1)
	};
}

void SpriteRenderer::createPipelineLayout() {
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
	.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	.setLayoutCount = 1,
	.pSetLayouts = &descriptorSetLayout,

	};
	if (vkCreatePipelineLayout(
		ctxt.getDevice(),
		&pipelineLayoutInfo,
		nullptr,
		&pipelineLayout
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout");
	}
}

void SpriteRenderer::createGraphicsPipeline() {
	auto vertShaderCode = readFile("shaders/sprite.vert.spv");
	auto fragShaderCode = readFile("shaders/sprite.frag.spv");

	VkShaderModule vertShaderModule
		= createShaderModule(ctxt, vertShaderCode);
	VkShaderModule fragShaderModule
		= createShaderModule(ctxt, fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShaderModule,
		.pName = "main"
	};
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShaderModule,
		.pName = "main"
	};
	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};


	std::array<VkVertexInputBindingDescription, 2>
		bindingDescriptions = {
		Vertex::getBindingDescription(),
		SpriteInstance::getBindingDescription()
	};

#pragma region Create attribute descriptions
	std::array<VkVertexInputAttributeDescription, 
		Vertex::ATTRIBUTE_COUNT + SpriteInstance::ATTRIBUTE_COUNT>
		attributeDescriptions;

	auto vertexAttributes = Vertex::getAttributeDescriptions();
	auto instanceAttributes = SpriteInstance::getAttributeDescriptions();

	std::copy(
		vertexAttributes.begin(),
		vertexAttributes.end(),
		attributeDescriptions.begin()
	);

	std::copy(
		instanceAttributes.begin(),
		instanceAttributes.end(),
		attributeDescriptions.begin() + vertexAttributes.size()
	);
#pragma endregion Create attribute descriptions

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 
			static_cast<uint32_t>(bindingDescriptions.size()),
		.pVertexBindingDescriptions = bindingDescriptions.data(),
		.vertexAttributeDescriptionCount =
			static_cast<uint32_t>(attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data(),
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkPipelineViewportStateCreateInfo viewportState{
	.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	.viewportCount = 1,
	.scissorCount = 1,
	};

	VkPipelineRasterizationStateCreateInfo rasterizer{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
#if NDEBUG
		.cullMode = VK_CULL_MODE_BACK_BIT,
#else
		.cullMode = VK_CULL_MODE_NONE,
#endif // NDEBUG
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisampling{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		//.minSampleShading = 1.0f,
		//.pSampleMask = nullptr,
		//.alphaToCoverageEnable = VK_FALSE,
		//.alphaToOneEnable = VK_FALSE,
	};

	VkPipelineDepthStencilStateCreateInfo depthStencil{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
#if NDEBUG
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
#else
		.depthTestEnable = VK_FALSE,
		.depthWriteEnable = VK_FALSE,
#endif
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.front = {},
		.back = {},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
			| VK_COLOR_COMPONENT_G_BIT
			| VK_COLOR_COMPONENT_B_BIT
			| VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo colorBlending{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f },
	};

	VkPipelineDynamicStateCreateInfo dynamicState{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};

	VkGraphicsPipelineCreateInfo pipelineInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depthStencil,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = pipelineLayout,
		.renderPass = rpm.getMainRenderPass(),
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};

	if (vkCreateGraphicsPipelines(
		ctxt.getDevice(),
		VK_NULL_HANDLE,
		1,
		&pipelineInfo,
		nullptr,
		&graphicsPipeline
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline");
	}

	vkDestroyShaderModule(ctxt.getDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(ctxt.getDevice(), vertShaderModule, nullptr);
}

void SpriteRenderer::createDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding spriteTexture{
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr,
	};

	VkDescriptorSetLayoutBinding cameraUBO{
		.binding = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
	};

	VkDescriptorSetLayoutBinding atlasBinding{
		.binding = 2,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr
	};

	std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
		spriteTexture,
		cameraUBO,
		atlasBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data(),
	};

	if (vkCreateDescriptorSetLayout(
		ctxt.getDevice(), 
		&layoutInfo, 
		nullptr, 
		&descriptorSetLayout
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout");
	}
}

void SpriteRenderer::createDescriptorPool() {
	std::array<VkDescriptorPoolSize, 3> poolSizes{
		// Sprite texture descriptors
		VkDescriptorPoolSize{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = static_cast<uint32_t>(ctxt.MAX_FRAMES_IN_FLIGHT),
		},
		// Camera UBO descriptors
		VkDescriptorPoolSize{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = static_cast<uint32_t>(ctxt.MAX_FRAMES_IN_FLIGHT),
		},
		VkDescriptorPoolSize{
			.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = static_cast<uint32_t>(ctxt.MAX_FRAMES_IN_FLIGHT),
		}
	};

	VkDescriptorPoolCreateInfo poolInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = static_cast<uint32_t>(ctxt.MAX_FRAMES_IN_FLIGHT),
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data(),
	};

	if (vkCreateDescriptorPool(
		ctxt.getDevice(),
		&poolInfo, 
		nullptr, 
		&descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool");
	}
}

void SpriteRenderer::createDescriptorSets(
	const std::array<std::unique_ptr<Buffer>, 
		VkContext::MAX_FRAMES_IN_FLIGHT>&
		uniformBuffers
) {
	const size_t sheetCount = spriteSheets.size();

	descriptorSets.resize(sheetCount);

	//layouts.fill(descriptorSetLayout);
	std::vector<VkDescriptorSetLayout> layouts(
		sheetCount * VkContext::MAX_FRAMES_IN_FLIGHT,
		descriptorSetLayout
	);

	std::vector<VkDescriptorSet> allocatedSets(layouts.size());

	VkDescriptorSetAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data()
	};

	if (vkAllocateDescriptorSets(
		ctxt.getDevice(),
		&allocInfo,
		allocatedSets.data()
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate desscriptor sets");
	}

	// Distributes allocated sets into [sheet][frame]
	for (size_t sheetIndex = 0;
		sheetIndex < sheetCount; ++sheetIndex) {
		for (size_t frame = 0;
			frame < VkContext::MAX_FRAMES_IN_FLIGHT;
			++frame) {
			descriptorSets[sheetIndex][frame] =
				allocatedSets[
					sheetIndex * VkContext::MAX_FRAMES_IN_FLIGHT + frame
				];
		}
	}

	for (size_t sheetIndex = 0; 
		sheetIndex < sheetCount; 
		++sheetIndex) {
		SpriteSheet& sheet = *spriteSheets[sheetIndex];

		VkDescriptorImageInfo imageInfo{
			.sampler = sheet.getTexture().getSampler(),
			.imageView = sheet.getTexture().getImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};

		VkDescriptorBufferInfo atlasInfo{
			.buffer = sheet.getAtlasBuffer().get(),
			.offset = 0,
			.range = sheet.getAtlasBufferSize()
		};

		for (size_t frame = 0;
			frame < VkContext::MAX_FRAMES_IN_FLIGHT;
			++frame) {
			VkDescriptorBufferInfo cameraInfo{
				.buffer = uniformBuffers[frame]->get(),
				.offset = 0,
				.range = sizeof(CameraUBO)
			};

			VkDescriptorSet descriptorSet
				= descriptorSets[sheetIndex][frame];

			std::array<VkWriteDescriptorSet, 3> writes{};

			// Sprite texture
			writes[0] = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = descriptorSet,
				.dstBinding = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &imageInfo
			};

			// Camera UBO
			writes[1] = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = descriptorSet,
				.dstBinding = 1,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pBufferInfo = &cameraInfo,
			};

			// Sprite atlas metadata
			writes[2] = {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = descriptorSet,
				.dstBinding = 2,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.pBufferInfo = &atlasInfo
			};

			vkUpdateDescriptorSets(
				ctxt.getDevice(),
				static_cast<uint32_t>(writes.size()),
				writes.data(),
				0,
				nullptr
			);
		}
	}
}

void SpriteRenderer::copyBuffer(
	VkBuffer srcBuffer,
	VkBuffer dstBuffer,
	VkDeviceSize size
) {
	VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = ctxt.getCommandPool(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer commandBuffer;

	if (vkAllocateCommandBuffers(
		ctxt.getDevice(),
		&allocInfo,
		&commandBuffer
	) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate copy command buffer");
	}

	VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		vkFreeCommandBuffers(
			ctxt.getDevice(), ctxt.getCommandPool(), 1, &commandBuffer
		);
		throw std::runtime_error("Failed to begin copy command buffer");
	}

	VkBufferCopy copyRegion{
		.srcOffset = 0,
		.dstOffset = 0,
		.size = size,
	};

	vkCmdCopyBuffer(
		commandBuffer,
		srcBuffer,
		dstBuffer,
		1,
		&copyRegion
	);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		vkFreeCommandBuffers(
			ctxt.getDevice(), ctxt.getCommandPool(), 1, &commandBuffer
		);
		throw std::runtime_error("Failed to end copy command buffer");
	}

	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
	};

	VkResult result = vkQueueSubmit(
		ctxt.getGraphicsQueue(),
		1,
		&submitInfo,
		VK_NULL_HANDLE
	);

	if (result == VK_SUCCESS) {
		result = vkQueueWaitIdle(ctxt.getGraphicsQueue());
	}

	vkFreeCommandBuffers(
		ctxt.getDevice(),
		ctxt.getCommandPool(),
		1,
		&commandBuffer
	);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to copy buffer");
	}
}

void SpriteRenderer::createVertexBuffer(
	const std::vector<Vertex>& vertices
) {
	// Vertex buffer
	VkDeviceSize vertexBufferSize =
		sizeof(vertices[0]) * vertices.size();

	Buffer stagingVertexBuffer(
		ctxt,
		vertexBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	stagingVertexBuffer.write(
		vertices.data(), 
		vertexBufferSize,
		0
	);

	vertexBuffer = std::make_unique<Buffer>(
		ctxt,
		vertexBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	copyBuffer(
		stagingVertexBuffer.get(),
		vertexBuffer->get(),
		vertexBufferSize
	);
}

void SpriteRenderer::createIndexBuffer(
	const std::vector<uint16_t>& indices
) {
	// Index buffer
	VkDeviceSize indexBufferSize =
		sizeof(indices[0]) * indices.size();

	Buffer stagingIndexBuffer(
		ctxt,
		indexBufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	stagingIndexBuffer.write(
		indices.data(), 
		indexBufferSize,
		0
	);

	indexBuffer = std::make_unique<Buffer>(
		ctxt,
		indexBufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	copyBuffer(
		stagingIndexBuffer.get(),
		indexBuffer->get(),
		indexBufferSize
	);
}

void SpriteRenderer::createVertexBuffer() {
	constexpr VkDeviceSize bufferSize = sizeof(vertices);

	vertexBuffer = std::make_unique<Buffer>(
		ctxt,
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	vertexBuffer->write(
		vertices.data(),
		bufferSize,
		0
	);
}

void SpriteRenderer::createIndexBuffer() {
	constexpr VkDeviceSize bufferSize = sizeof(indices);

	indexBuffer = std::make_unique<Buffer>(
		ctxt,
		bufferSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	indexBuffer->write(
		indices.data(),
		bufferSize,
		0
	);
}

void SpriteRenderer::createInstanceBuffer(
	const std::vector<SpriteInstance>& instances
) {
	VkDeviceSize bufferSize =
		sizeof(SpriteInstance) * instances.size();

	instanceBuffer = std::make_unique<Buffer>(
		ctxt,
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	instanceBuffer->write(
		instances.data(),
		bufferSize,
		0
	);

	instanceCount = static_cast<uint32_t>(instances.size());
	//std::cout << instanceCount << std::endl;
}

void SpriteRenderer::createInstanceBuffers() {
	instanceBuffers.resize(spriteSheets.size());

	for (size_t sheet = 0; sheet < spriteSheets.size(); ++sheet) {
		for (size_t frame = 0;
			frame < VkContext::MAX_FRAMES_IN_FLIGHT;
			++frame) {

			instanceBuffers[sheet][frame] =
				std::make_unique<Buffer>(
					ctxt,
					spriteSheets[sheet]->getMaxInstances() 
						* sizeof(SpriteInstance),
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				);
		}
	}
}

void SpriteRenderer::updateInstanceBuffers(
	uint32_t currentFrame
) {
	for (size_t i = 0; i < instanceBatches.size(); ++i) {
		auto& batch = instanceBatches[i];

		if (batch.empty()) {
			continue;
		}

		instanceBuffers[i][currentFrame]->write(
			batch.data(),
			batch.size() * sizeof(SpriteInstance),
			0
		);
	}
}

