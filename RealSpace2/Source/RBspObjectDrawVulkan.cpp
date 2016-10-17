#include "stdafx.h"
#include "RBspObjectDrawVulkan.h"
#include "RBspObject.h"
#include "RS2.h"
#include "BasicRenderVS.h"
#include "BasicRenderFS.h"

void RBspObjectDrawVulkan::CreateBuffers()
{
	auto ret = GetRS2Vulkan().VulkanDevice->createBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&VertexBuffer,
		bsp.OcVertices.size() * sizeof(bsp.OcVertices[0]),
		bsp.OcVertices.data());

	if (ret != VK_SUCCESS)
	{
		MLog("RBspObjectDrawVulkan::Init - Failed to create vertex buffer\n");
	}

	ret = GetRS2Vulkan().VulkanDevice->createBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&IndexBuffer,
		bsp.OcIndices.size() * sizeof(bsp.OcIndices[0]),
		bsp.OcIndices.data());

	if (ret != VK_SUCCESS)
	{
		MLog("RBspObjectDrawVulkan::Init - Failed to create index buffer\n");
	}
}

void RBspObjectDrawVulkan::RenderNode(VkCommandBuffer CmdBuffer, RSBspNode& Node, int Material)
{
	// Leaf node
	if (Node.nPolygon)
	{
		auto TriangleCount = Node.pDrawInfo[Material].nTriangleCount;
		if (TriangleCount)
		{
#ifdef _DEBUG
			g_nCall++;
			g_nPoly += TriangleCount;
#endif

			vkCmdDrawIndexed(CmdBuffer,
				TriangleCount * 3, 1,
				Node.pDrawInfo[Material].nIndicesOffset, 0, 0);
		}
		return;
	}

	// Branch node
	auto DrawNode = [&](auto Branch) {
		if (Node.*Branch)
			RenderNode(CmdBuffer, *(Node.*Branch), Material);
	};

	DrawNode(&RSBspNode::m_pPositive);
	DrawNode(&RSBspNode::m_pNegative);
}

void RBspObjectDrawVulkan::Render(VkCommandBuffer CmdBuffer)
{
	for (int i = 1; i < bsp.m_nMaterial; ++i)
	{
		auto& Material = bsp.Materials[i];

		if (Material.dwFlags & (RM_FLAG_ADDITIVE | RM_FLAG_USEOPACITY))
			continue;

		if (Material.VkMaterial.DescriptorSet == VK_NULL_HANDLE)
			continue;

		std::array<VkDescriptorSet, 2> descriptorSets;
		// Set 0: Scene descriptor set containing global matrices
		descriptorSets[0] = DescriptorSet;
		// Set 1: Per-Material descriptor set containing bound images
		descriptorSets[1] = Material.VkMaterial.DescriptorSet;

		vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, NULL);
		
		RenderNode(CmdBuffer, bsp.OcRoot[0], i);
	}
}

void RBspObjectDrawVulkan::CreateCommandBuffers()
{
	auto Width = RGetScreenWidth();
	auto Height = RGetScreenHeight();

	VkCommandBufferBeginInfo cmdBufInfo = vkTools::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0, 0, 0.5, 1 } };
	clearValues[1].depthStencil = { 1, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vkTools::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = GetRS2Vulkan().RenderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = Width;
	renderPassBeginInfo.renderArea.extent.height = Height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (size_t i = 0; i < GetRS2Vulkan().DrawCmdBuffers.size(); ++i)
	{
		auto CmdBuffer = GetRS2Vulkan().DrawCmdBuffers[i];

		// Set target frame buffer
		renderPassBeginInfo.framebuffer = GetRS2Vulkan().FrameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(CmdBuffer, &cmdBufInfo));

		vkCmdBeginRenderPass(CmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vkTools::initializers::viewport((float)Width, (float)Height, 0.0f, 1.0f);
		vkCmdSetViewport(CmdBuffer, 0, 1, &viewport);

		VkRect2D scissor = vkTools::initializers::rect2D(Width, Height, 0, 0);
		vkCmdSetScissor(CmdBuffer, 0, 1, &scissor);

		vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout,
			0, 1, &DescriptorSet, 0, NULL);
		vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

		constexpr u32 VERTEX_BUFFER_BIND_ID{};
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(CmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &VertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(CmdBuffer, IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

		Render(CmdBuffer);

		vkCmdEndRenderPass(CmdBuffer);

		VK_CHECK_RESULT(vkEndCommandBuffer(CmdBuffer));
	}
}

void RBspObjectDrawVulkan::SetupVertexDescriptions()
{
	constexpr u32 VERTEX_BUFFER_BIND_ID{};
	// Binding description
	vertices.bindingDescriptions.resize(1);
	vertices.bindingDescriptions[0] =
		vkTools::initializers::vertexInputBindingDescription(
			VERTEX_BUFFER_BIND_ID,
			sizeof(BSPVERTEX),
			VK_VERTEX_INPUT_RATE_VERTEX);

	// Attribute descriptions
	// Describes memory layout and shader positions
	vertices.attributeDescriptions.resize(3);
	// Location 0: Position
	vertices.attributeDescriptions[0] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(BSPVERTEX, x));
	// Location 1: Diffuse texture coordinates
	vertices.attributeDescriptions[1] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			1,
			VK_FORMAT_R32G32_SFLOAT,
			offsetof(BSPVERTEX, tu1));
	// Location 2: Lightmap texture coordinates
	vertices.attributeDescriptions[2] =
		vkTools::initializers::vertexInputAttributeDescription(
			VERTEX_BUFFER_BIND_ID,
			2,
			VK_FORMAT_R32G32_SFLOAT,
			offsetof(BSPVERTEX, tu2));

	vertices.inputState = vkTools::initializers::pipelineVertexInputStateCreateInfo();
	vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
	vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
	vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
	vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

void RBspObjectDrawVulkan::CreatePipelines()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		vkTools::initializers::pipelineInputAssemblyStateCreateInfo(
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			0,
			VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		vkTools::initializers::pipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			0);

	VkPipelineColorBlendAttachmentState blendAttachmentState =
		vkTools::initializers::pipelineColorBlendAttachmentState(
			0xf,
			VK_FALSE);

	VkPipelineColorBlendStateCreateInfo colorBlendState =
		vkTools::initializers::pipelineColorBlendStateCreateInfo(
			1,
			&blendAttachmentState);

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		vkTools::initializers::pipelineDepthStencilStateCreateInfo(
			VK_TRUE,
			VK_TRUE,
			VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipelineViewportStateCreateInfo viewportState =
		vkTools::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

	VkPipelineMultisampleStateCreateInfo multisampleState =
		vkTools::initializers::pipelineMultisampleStateCreateInfo(
			VK_SAMPLE_COUNT_1_BIT,
			0);

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState =
		vkTools::initializers::pipelineDynamicStateCreateInfo(
			dynamicStateEnables.data(),
			static_cast<uint32_t>(dynamicStateEnables.size()),
			0);

	// Load shaders
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	shaderStages[0] = GetRS2Vulkan().loadShader(BasicRenderVS, ArraySize(BasicRenderVS), VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = GetRS2Vulkan().loadShader(BasicRenderFS, ArraySize(BasicRenderFS), VK_SHADER_STAGE_FRAGMENT_BIT);

	auto pipelineCreateInfo = vkTools::initializers::pipelineCreateInfo(
			PipelineLayout,
			GetRS2Vulkan().RenderPass,
			0);

	pipelineCreateInfo.pVertexInputState = &vertices.inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(GetRS2Vulkan().Device, PipelineCache, 1, &pipelineCreateInfo,
		nullptr, &Pipeline));

	// Wireframe rendering pipeline
	blendAttachmentState.blendEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	rasterizationState.lineWidth = 1.0f;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(GetRS2Vulkan().Device, PipelineCache, 1, &pipelineCreateInfo,
		nullptr, &WireframePipeline));
}

void RBspObjectDrawVulkan::PrepareUniformBuffers()
{
	VK_CHECK_RESULT(GetRS2Vulkan().VulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&uniformBufferVS,
		sizeof(uboVS),
		&uboVS));

	UpdateUniformBuffers();
}

void RBspObjectDrawVulkan::UpdateUniformBuffers()
{
	glm::vec3 cameraPos{ EXPAND_VECTOR(RCameraPosition) };

	uboVS.projection = glm::perspective(glm::radians(60.0f),
		(float)RGetScreenWidth() / (float)RGetScreenHeight(), 5.f, 10000.0f);

	uboVS.model = glm::lookAt(
		cameraPos,
		cameraPos + glm::vec3{ EXPAND_VECTOR(RCameraDirection) },
		glm::vec3{ EXPAND_VECTOR(RCameraUp) }),

	uboVS.viewPos = glm::vec4(cameraPos, 0.0f);

	VK_CHECK_RESULT(uniformBufferVS.map());
	memcpy(uniformBufferVS.mapped, &uboVS, sizeof(uboVS));
	uniformBufferVS.unmap();
}

void RBspObjectDrawVulkan::SetupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		// Only need one uniform buffer
		vkTools::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		// Need a texture descriptor for each material in the map
		vkTools::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, bsp.m_nMaterial)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vkTools::initializers::descriptorPoolCreateInfo(
			static_cast<uint32_t>(poolSizes.size()),
			poolSizes.data(),
			bsp.m_nMaterial + 1);

	VK_CHECK_RESULT(vkCreateDescriptorPool(GetRS2Vulkan().Device, &descriptorPoolInfo, nullptr, &DescriptorPool));
}

void RBspObjectDrawVulkan::SetupDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{ 1 };

	// Binding 0: Vertex shader uniform buffer
	setLayoutBindings[0] = vkTools::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_VERTEX_BIT,
		0);

	auto descriptorLayout = vkTools::initializers::descriptorSetLayoutCreateInfo(
		setLayoutBindings.data(),
		static_cast<uint32_t>(setLayoutBindings.size()));

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(GetRS2Vulkan().Device, &descriptorLayout, nullptr,
		&DescriptorSetLayouts.Scene));

	setLayoutBindings[0] = vkTools::initializers::descriptorSetLayoutBinding(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0);

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(GetRS2Vulkan().Device, &descriptorLayout, nullptr,
		&DescriptorSetLayouts.Material));

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vkTools::initializers::pipelineLayoutCreateInfo(
			DescriptorSetLayouts.Layouts,
			2);

	VK_CHECK_RESULT(vkCreatePipelineLayout(GetRS2Vulkan().Device, &pPipelineLayoutCreateInfo, nullptr, &PipelineLayout));
}

void RBspObjectDrawVulkan::SetupDescriptorSet()
{
	auto allocInfo = vkTools::initializers::descriptorSetAllocateInfo(
			DescriptorPool,
			&DescriptorSetLayouts.Scene,
			1);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(GetRS2Vulkan().Device, &allocInfo, &DescriptorSet));

	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		// Binding 0: Vertex shader uniform buffer
		vkTools::initializers::writeDescriptorSet(
			DescriptorSet,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			0,
			&uniformBufferVS.descriptor)
	};

	vkUpdateDescriptorSets(GetRS2Vulkan().Device,
		static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(),
		0, nullptr);

	for (int i = 1; i < bsp.m_nMaterial; ++i)
	{
		auto& Material = bsp.Materials[i];

		if (Material.VkMaterial.Texture.view == VK_NULL_HANDLE)
			continue;

		allocInfo = vkTools::initializers::descriptorSetAllocateInfo(
				DescriptorPool,
				&DescriptorSetLayouts.Material,
				1);

		VK_CHECK_RESULT(vkAllocateDescriptorSets(GetRS2Vulkan().Device, &allocInfo, &Material.VkMaterial.DescriptorSet));

		std::vector<VkWriteDescriptorSet> writeDescriptorSets =
		{
			// Binding 0: Pixel shader texture
			vkTools::initializers::writeDescriptorSet(
				Material.VkMaterial.DescriptorSet,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				0,
				&Material.VkMaterial.Texture.descriptor)
		};

		vkUpdateDescriptorSets(GetRS2Vulkan().Device, 
			static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(),
			0, nullptr);
	}
}

void RBspObjectDrawVulkan::Init()
{
	CreateBuffers();
	SetupVertexDescriptions();
	PrepareUniformBuffers();
	SetupDescriptorSetLayout();
	CreatePipelines();
	SetupDescriptorPool();
	SetupDescriptorSet();
	CreateCommandBuffers();
}

void RBspObjectDrawVulkan::Draw()
{
}