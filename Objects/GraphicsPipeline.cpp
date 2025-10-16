//
// GraphicsPipeline.cpp
//	Vulkan Setup
//
// See matched header file for definitive main comment.
//
// 1/31/19 Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "GraphicsPipeline.h"


GraphicsPipeline::GraphicsPipeline(ShaderModules& shaders, RenderPass& renderPass,
								   Swapchain& swapchain, GraphicsDevice& graphics,
								   VertexAbstract* pVertex, Descriptors* pDescriptors,
								   Customizer customize)
	:	device(graphics.getLogical())
{
	pVertex->vetIsValid();

	create(shaders, pVertex, swapchain.getExtent(), renderPass, pDescriptors, customize);
}

// Overloaded constructor for custom VkRenderPass (e.g., shadow mapping)
GraphicsPipeline::GraphicsPipeline(ShaderModules& shaders, VkRenderPass vkRenderPass,
								   Swapchain& swapchain, GraphicsDevice& graphics,
								   VertexAbstract* pVertex, Descriptors* pDescriptors,
								   Customizer customize, VkExtent2D customExtent)
	:	device(graphics.getLogical())
{
	pVertex->vetIsValid();

	// Use custom extent if provided (width != 0), otherwise use swapchain extent
	VkExtent2D extent = (customExtent.width != 0) ? customExtent : swapchain.getExtent();
	create(shaders, pVertex, extent, vkRenderPass, true, false, pDescriptors, customize);
}

GraphicsPipeline::~GraphicsPipeline()
{
	destroy();
}
void GraphicsPipeline::destroy()
{
	vkDestroyPipeline(device, graphicsPipeline, nullALLOC);
	vkDestroyPipelineLayout(device, pipelineLayout, nullALLOC);
}


void GraphicsPipeline::create(ShaderModules& shaderModules, VertexAbstract* pVertex,
							  VkExtent2D swapchainExtent, RenderPass& renderPass,
							  Descriptors* pDescriptors, Customizer customize)
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.vertexBindingDescriptionCount	 = pVertex ? pVertex->nBindingDescriptions()   : 0,
		.pVertexBindingDescriptions		 = pVertex ? pVertex->pBindingDescriptions()   : nullptr,
		.vertexAttributeDescriptionCount = pVertex ? pVertex->nAttributeDescriptions() : 0,
		.pVertexAttributeDescriptions	 = pVertex ? pVertex->pAttributeDescriptions() : nullptr
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType	  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext	  = nullptr,
		.flags	  = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport = {
		.x	= 0.0f,
		.y	= 0.0f,
		.width	= (float) swapchainExtent.width,
		.height	= (float) swapchainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = swapchainExtent
	};
	VkPipelineViewportStateCreateInfo viewportState = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.viewportCount	= 1,
		.pViewports		= &viewport,
		.scissorCount	= 1,
		.pScissors		= &scissor
	};

	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.depthClampEnable		 = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode			 = customize & WIREFRAME ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL,
		.cullMode	  = (VkFlags) (customize & SHOW_BACKFACES ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT),
		.frontFace				 = customize & FRONT_CLOCKWISE ? VK_FRONT_FACE_CLOCKWISE
														: VK_FRONT_FACE_COUNTER_CLOCKWISE,	// (*) note below
		.depthBiasEnable		 = VK_FALSE,
		/*.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp			 = 0.0f,
		.depthBiasSlopeFactor	 = 0.0f,*/
		.lineWidth				 = 1.0f
	};

	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable	= VK_FALSE,
		/*.minSampleShading		= 1.0f,					// FYI: all these commented-out fields, above
		.pSampleMask			= nullptr,				//	and below, are "possible but not required"
		.alphaToCoverageEnable	= VK_FALSE,				//	for the specific needs of this demo program.
		.alphaToOneEnable		= VK_FALSE*/			//	If needed, reactivate them, or even add
	};													//	passed-in parameter(s) to set them to.

	VkPipelineDepthStencilStateCreateInfo depthStencil = {
		.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext					= nullptr,
		.flags					= 0,
		.depthTestEnable		= VK_TRUE,
		.depthWriteEnable		= static_cast<VkBool32>(customize & ALPHA_BLENDING ? VK_FALSE : VK_TRUE),  // Disable depth writes for transparent objects
		.depthCompareOp			= VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable	= VK_FALSE,
		.stencilTestEnable		= VK_FALSE,
		/*.front				= { },
		.back					= { },
		.minDepthBounds			= 0.0f,
		.maxDepthBounds			= 1.0f*/
	};

	// Alpha blending: disabled by default, enabled via ALPHA_BLENDING customizer flag
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable		 = static_cast<VkBool32>(customize & ALPHA_BLENDING ? VK_TRUE : VK_FALSE),
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp		 = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp		 = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
						| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};
	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.logicOpEnable		= VK_FALSE,
		.logicOp			= VK_LOGIC_OP_COPY,
		.attachmentCount	= 1,
		.pAttachments		= &colorBlendAttachment,
		.blendConstants		= { 0.0f, 0.0f, 0.0f, 0.0f }
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.setLayoutCount	= pDescriptors ? (uint32_t) 1 : 0,
		.pSetLayouts	= pDescriptors ? pDescriptors->getpLayout() : nullptr,
		/*.pushConstantRangeCount = 0,		// can pass values into shader
		.pPushConstantRanges	= nullptr*/
	};

	call = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullALLOC, &pipelineLayout);

	if (call != VK_SUCCESS)
		Fatal("Create Pipeline Layout FAILURE" + ErrStr(call));

	VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.stageCount			 = shaderModules.NumShaderStages(),
		.pStages			 = shaderModules.ShaderStages(),
		.pVertexInputState	 = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pTessellationState	 = nullptr,
		.pViewportState		 = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState	 = &multisampling,
		.pDepthStencilState	 = renderPass.isDepthBufferUsed ? &depthStencil : nullptr,
		.pColorBlendState	 = &colorBlending,
		.pDynamicState		 = nullptr,
		.layout				 = pipelineLayout,
		.renderPass			 = renderPass.getVkRenderPass(),
		.subpass			 = 0,
		.basePipelineHandle	 = VK_NULL_HANDLE,
		.basePipelineIndex	 = -1
	};

	call = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, // number of pipelines
									 &pipelineInfo, nullALLOC, &graphicsPipeline);
	if (call != VK_SUCCESS)
		Fatal("FAIL on Create Graphics Pipeline" + ErrStr(call));
}

void GraphicsPipeline::Recreate(ShaderModules& shaders, RenderPass& renderPass, Swapchain& swapchain,
								VertexAbstract* pVertex, Descriptors* pDescriptors, Customizer customize)
{
	destroy();
	create(shaders, pVertex, swapchain.getExtent(), renderPass, pDescriptors, customize);
}


/* DEV NOTE
	(*) - We're standardizing on COUNTER_CLOCKWISE vertex ordering to define the
	front face (outward-pointing normal) and also using backface culling.
  e.g.:
	Wavefront OBJ: "Vertices are stored in a counter-clockwise order by default"
	https://en.wikipedia.org/wiki/Wavefront_.obj_file
	OpenGL: "By default, counterclockwise polygons are taken to be front-facing."
	https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glFrontFace.xml
	Vulkan: "VK_FRONT_FACE_COUNTER_CLOCKWISE specifies that a triangle with positive area is considered front-facing."
	https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkFrontFace.html
	Maya: "If you place vertices in a counter-clockwise direction, the face normal points upward."
	https://knowledge.autodesk.com/support/maya/learn-explore/caas/CloudHelp/cloudhelp/2019/ENU/Maya-Modeling/files/GUID-9C257D44-924D-4B3F-ADEF-C71FAA98EAB1-htm.html

	Therefore, any manually defined geometry used herein is counter-clockwise.

	Supposedly Unity3D uses clockwise and left-hand-coordinate system, so watch out for that.
*/

// Overloaded create for custom VkRenderPass (shadow mapping uses depth-only pass)
void GraphicsPipeline::create(ShaderModules& shaderModules, VertexAbstract* pVertex,
							  VkExtent2D swapchainExtent, VkRenderPass vkRenderPass, bool useDepthBuffer,
							  bool hasColorAttachment, Descriptors* pDescriptors, Customizer customize)
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.vertexBindingDescriptionCount	 = pVertex ? pVertex->nBindingDescriptions()   : 0,
		.pVertexBindingDescriptions		 = pVertex ? pVertex->pBindingDescriptions()   : nullptr,
		.vertexAttributeDescriptionCount = pVertex ? pVertex->nAttributeDescriptions() : 0,
		.pVertexAttributeDescriptions	 = pVertex ? pVertex->pAttributeDescriptions() : nullptr
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType	  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext	  = nullptr,
		.flags	  = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport = {
		.x	= 0.0f,
		.y	= 0.0f,
		.width	= (float) swapchainExtent.width,
		.height	= (float) swapchainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = swapchainExtent
	};
	VkPipelineViewportStateCreateInfo viewportState = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.viewportCount	= 1,
		.pViewports		= &viewport,
		.scissorCount	= 1,
		.pScissors		= &scissor
	};

	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.depthClampEnable		 = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode			 = customize & WIREFRAME ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL,
		.cullMode	  = (VkFlags) (customize & SHOW_BACKFACES ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT),
		.frontFace				 = customize & FRONT_CLOCKWISE ? VK_FRONT_FACE_CLOCKWISE
														: VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable		 = VK_FALSE,
		.lineWidth				 = 1.0f
	};

	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable	= VK_FALSE
	};

	VkPipelineDepthStencilStateCreateInfo depthStencil = {
		.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext					= nullptr,
		.flags					= 0,
		.depthTestEnable		= VK_TRUE,
		.depthWriteEnable		= static_cast<VkBool32>(customize & ALPHA_BLENDING ? VK_FALSE : VK_TRUE),  // Disable depth writes for transparent objects
		.depthCompareOp			= VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable	= VK_FALSE,
		.stencilTestEnable		= VK_FALSE
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable		 = static_cast<VkBool32>(customize & ALPHA_BLENDING ? VK_TRUE : VK_FALSE),
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp		 = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp		 = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
						| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};
	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.logicOpEnable		= VK_FALSE,
		.logicOp			= VK_LOGIC_OP_COPY,
		.attachmentCount	= static_cast<uint32_t>(hasColorAttachment ? 1 : 0),
		.pAttachments		= hasColorAttachment ? &colorBlendAttachment : nullptr,
		.blendConstants		= { 0.0f, 0.0f, 0.0f, 0.0f }
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		.sType	= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.setLayoutCount	= pDescriptors ? (uint32_t) 1 : 0,
		.pSetLayouts	= pDescriptors ? pDescriptors->getpLayout() : nullptr
	};

	call = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullALLOC, &pipelineLayout);

	if (call != VK_SUCCESS)
		Fatal("Create Pipeline Layout FAILURE" + ErrStr(call));

	VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType	= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext	= nullptr,
		.flags	= 0,
		.stageCount			 = shaderModules.NumShaderStages(),
		.pStages			 = shaderModules.ShaderStages(),
		.pVertexInputState	 = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pTessellationState	 = nullptr,
		.pViewportState		 = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState	 = &multisampling,
		.pDepthStencilState	 = useDepthBuffer ? &depthStencil : nullptr,
		.pColorBlendState	 = &colorBlending,
		.pDynamicState		 = nullptr,
		.layout				 = pipelineLayout,
		.renderPass			 = vkRenderPass,
		.subpass			 = 0,
		.basePipelineHandle	 = VK_NULL_HANDLE,
		.basePipelineIndex	 = -1
	};

	call = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
									 &pipelineInfo, nullALLOC, &graphicsPipeline);
	if (call != VK_SUCCESS)
		Fatal("FAIL on Create Graphics Pipeline" + ErrStr(call));
}
