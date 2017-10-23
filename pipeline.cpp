#include "pipeline.h"
#include "renderer.h"
#include "swapchain.h"
#include "vertex.h"
#include "shader.h"
#include "renderpass.h"

#include <fstream>

Pipeline::Pipeline()
    : m_vkPipeline( VK_NULL_HANDLE ),
      m_vkLayout( VK_NULL_HANDLE ),
      m_pRenderPass( nullptr )
{
}

Pipeline::Pipeline( RenderPass& renderPass,
                    const std::vector<Shader*>& shaders )
    : Pipeline()
{
	m_pRenderPass = &renderPass;

	if( !createLayout() ||
	    !createPipeline( shaders ) )
	{
		destroy();
	}
}

Pipeline::~Pipeline()
{
	destroy();
}

void Pipeline::destroy()
{
	VkDevice device = m_pRenderPass->getRenderer().getNativeDeviceHandle();

	if( m_vkPipeline != VK_NULL_HANDLE )
	{
		vkDestroyPipeline( device, m_vkPipeline, nullptr );
		m_vkPipeline = VK_NULL_HANDLE;
	}

	if( m_vkLayout != VK_NULL_HANDLE )
	{
		vkDestroyPipelineLayout( device, m_vkLayout, nullptr );
		m_vkLayout = VK_NULL_HANDLE;
	}
}

VkPipelineShaderStageCreateInfo Pipeline::createShaderStageCreateInfo( Shader& shader )
{
	VkPipelineShaderStageCreateInfo createInfo{};
	createInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.pNext               = nullptr;
	createInfo.flags               = 0;
	createInfo.stage               = shader.getStage();
	createInfo.module              = shader.getNativeHandle();
	createInfo.pName               = shader.getEntryFunction().c_str();
	createInfo.pSpecializationInfo = nullptr;

	return createInfo;
}

void Pipeline::populateFixedFunctionSetup( FixedFunctionSetup& ffs )
{
	ffs.vertexInputBindings.resize( 1 );
	ffs.vertexInputBindings[ 0 ].binding   = 0;
	ffs.vertexInputBindings[ 0 ].stride    = sizeof( Vertex );
	ffs.vertexInputBindings[ 0 ].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	const auto& vertexAttributes = Vertex::getAttributeDescriptions();

	ffs.vertexInputAttributes.resize( vertexAttributes.size() );
	for( auto i = 0; i < vertexAttributes.size(); ++i )
	{
		ffs.vertexInputAttributes[ i ].binding  = 0;
		ffs.vertexInputAttributes[ i ].location = i;
		ffs.vertexInputAttributes[ i ].format   = vertexAttributes[ i ].format;
		ffs.vertexInputAttributes[ i ].offset   = vertexAttributes[ i ].offset;
	}

	ffs.vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	ffs.vertexInput.pNext                           = nullptr;
	ffs.vertexInput.flags                           = 0;
	ffs.vertexInput.vertexBindingDescriptionCount   = ffs.vertexInputBindings.size();
	ffs.vertexInput.pVertexBindingDescriptions      = ffs.vertexInputBindings.data();
	ffs.vertexInput.vertexAttributeDescriptionCount = ffs.vertexInputAttributes.size();
	ffs.vertexInput.pVertexAttributeDescriptions    = ffs.vertexInputAttributes.data();

	ffs.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ffs.inputAssembly.pNext                  = nullptr;
	ffs.inputAssembly.flags                  = 0;
	ffs.inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	ffs.inputAssembly.primitiveRestartEnable = VK_FALSE;

//	uint32_t viewportWidth, viewportHeight;
//	m_pRenderer->getSwapChain().getExtent( viewportWidth, viewportHeight );
//
//	ffs.viewportViewports.resize( 1 );
//	ffs.viewportViewports[ 0 ].x        = 0.0f;
//	ffs.viewportViewports[ 0 ].y        = 0.0f;
//	ffs.viewportViewports[ 0 ].width    = viewportWidth;
//	ffs.viewportViewports[ 0 ].height   = viewportHeight;
//	ffs.viewportViewports[ 0 ].minDepth = 0.0f;
//	ffs.viewportViewports[ 0 ].maxDepth = 1.0f;
//
//	ffs.viewportScissors.resize( 1 );
//	ffs.viewportScissors[ 0 ].offset  = { 0, 0 };
//	ffs.viewportScissors[ 0 ].extent  = { viewportWidth, viewportHeight };

	ffs.viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ffs.viewport.pNext         = nullptr;
	ffs.viewport.flags         = 0;
//	ffs.viewport.viewportCount = ffs.viewportViewports.size();
//	ffs.viewport.pViewports    = ffs.viewportViewports.data();
//	ffs.viewport.scissorCount  = ffs.viewportScissors.size();
//	ffs.viewport.pScissors     = ffs.viewportScissors.data();
	ffs.viewport.viewportCount = 1;
	ffs.viewport.pViewports    = nullptr;
	ffs.viewport.scissorCount  = 1;
	ffs.viewport.pScissors     = nullptr;

	ffs.rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	ffs.rasterization.pNext                   = nullptr;
	ffs.rasterization.flags                   = 0;
	ffs.rasterization.depthClampEnable        = VK_FALSE;
	ffs.rasterization.rasterizerDiscardEnable = VK_FALSE;
	ffs.rasterization.polygonMode             = VK_POLYGON_MODE_FILL;
	ffs.rasterization.lineWidth               = 1.0f;
	ffs.rasterization.cullMode                = VK_CULL_MODE_BACK_BIT;
	ffs.rasterization.frontFace               = VK_FRONT_FACE_CLOCKWISE;
	ffs.rasterization.depthBiasEnable         = VK_FALSE;
	ffs.rasterization.depthBiasConstantFactor = 0.0f;
	ffs.rasterization.depthBiasClamp          = 0.0f;
	ffs.rasterization.depthBiasSlopeFactor    = 0.0f;

	ffs.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ffs.multisampling.pNext                 = nullptr;
	ffs.multisampling.flags                 = 0;
	ffs.multisampling.sampleShadingEnable   = VK_FALSE;
	ffs.multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
	ffs.multisampling.minSampleShading      = 1.0f;
	ffs.multisampling.pSampleMask           = nullptr;
	ffs.multisampling.alphaToCoverageEnable = VK_FALSE;
	ffs.multisampling.alphaToOneEnable      = VK_FALSE;

	ffs.colorBlendAttachmentStates.resize( 1 );
	ffs.colorBlendAttachmentStates[ 0 ].colorWriteMask      = VK_COLOR_COMPONENT_R_BIT |
	                                                          VK_COLOR_COMPONENT_G_BIT |
	                                                          VK_COLOR_COMPONENT_B_BIT |
	                                                          VK_COLOR_COMPONENT_A_BIT;
	ffs.colorBlendAttachmentStates[ 0 ].blendEnable         = VK_FALSE;
	ffs.colorBlendAttachmentStates[ 0 ].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	ffs.colorBlendAttachmentStates[ 0 ].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	ffs.colorBlendAttachmentStates[ 0 ].colorBlendOp        = VK_BLEND_OP_ADD;
	ffs.colorBlendAttachmentStates[ 0 ].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	ffs.colorBlendAttachmentStates[ 0 ].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	ffs.colorBlendAttachmentStates[ 0 ].alphaBlendOp        = VK_BLEND_OP_ADD;

	ffs.colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ffs.colorBlend.pNext               = nullptr;
	ffs.colorBlend.flags               = 0;
	ffs.colorBlend.logicOpEnable       = VK_FALSE;
	ffs.colorBlend.logicOp             = VK_LOGIC_OP_COPY;
	ffs.colorBlend.attachmentCount     = ffs.colorBlendAttachmentStates.size();
	ffs.colorBlend.pAttachments        = ffs.colorBlendAttachmentStates.data();
	ffs.colorBlend.blendConstants[ 0 ] = 0.0f;
	ffs.colorBlend.blendConstants[ 1 ] = 0.0f;
	ffs.colorBlend.blendConstants[ 2 ] = 0.0f;
	ffs.colorBlend.blendConstants[ 3 ] = 0.0f;
}

bool Pipeline::createLayout()
{
	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pNext                  = nullptr;
	createInfo.flags                  = 0;
	createInfo.setLayoutCount         = 0;
	createInfo.pSetLayouts            = nullptr;
	createInfo.pushConstantRangeCount = 0;
	createInfo.pPushConstantRanges    = 0;

	VkResult res = vkCreatePipelineLayout( m_pRenderPass->getRenderer().getNativeDeviceHandle(),
	                                       &createInfo,
	                                       nullptr,
	                                       &m_vkLayout );


	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create pipeline layout." );
		return false;
	}
	return true;
}

bool Pipeline::createPipeline( const std::vector<Shader*>& shaders )
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.resize( shaders.size() );

	for( auto i = 0; i < shaderStages.size(); ++i )
	{
		shaderStages[ i ] = createShaderStageCreateInfo( *shaders[ i ] );
	}

	FixedFunctionSetup fixedFunction;
	populateFixedFunctionSetup( fixedFunction );

	std::vector<VkDynamicState> dynamicStates = {
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext             = nullptr;
	dynamicState.flags             = 0;
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates    = dynamicStates.data();

	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.pNext               = nullptr;
	createInfo.flags               = 0;
	createInfo.stageCount          = shaderStages.size();
	createInfo.pStages             = shaderStages.data();
	createInfo.pVertexInputState   = &fixedFunction.vertexInput;
	createInfo.pInputAssemblyState = &fixedFunction.inputAssembly;
	createInfo.pTessellationState  = nullptr;
	createInfo.pViewportState      = &fixedFunction.viewport;
	createInfo.pRasterizationState = &fixedFunction.rasterization;
	createInfo.pMultisampleState   = &fixedFunction.multisampling;
	createInfo.pDepthStencilState  = nullptr;
	createInfo.pColorBlendState    = &fixedFunction.colorBlend;
	createInfo.pDynamicState       = &dynamicState;
	createInfo.layout              = m_vkLayout;
	createInfo.renderPass          = m_pRenderPass->getNativeHandle();
	createInfo.subpass             = 0;
	createInfo.basePipelineHandle  = VK_NULL_HANDLE;
	createInfo.basePipelineIndex   = -1;

	VkResult res = vkCreateGraphicsPipelines( m_pRenderPass->getRenderer().getNativeDeviceHandle(),
	                                          VK_NULL_HANDLE,
	                                          1,
	                                          &createInfo,
	                                          nullptr,
	                                          &m_vkPipeline );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create pipeline." );
		return false;
	}
	return true;
}
