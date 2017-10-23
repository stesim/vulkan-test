#include "renderpass.h"
#include "renderer.h"

RenderPass::RenderPass( Renderer& renderer, VkFormat format )
    : wrapper_type( renderer.getNativeDeviceHandle() )
{
	m_pRenderer = &renderer;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.flags          = 0;
	colorAttachment.format         = format;
	colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.flags                   = 0;
	subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount    = 0;
	subpass.pInputAttachments       = nullptr;
	subpass.colorAttachmentCount    = 1;
	subpass.pColorAttachments       = &colorAttachmentRef;
	subpass.pResolveAttachments     = nullptr;
	subpass.pDepthStencilAttachment = nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments    = nullptr;

	VkSubpassDependency dependency{}; // TODO: not always required,
	                                  //       introduce a way to define dependencies
	dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass      = 0;
	dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask   = 0;
	dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
	                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = 0;

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.pNext           = nullptr;
	createInfo.flags           = 0;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments    = &colorAttachment;
	createInfo.subpassCount    = 1;
	createInfo.pSubpasses      = &subpass;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies   = &dependency;

	VkResult res = vkCreateRenderPass( m_vkDevice,
	                                   &createInfo,
	                                   nullptr,
	                                   &m_vkHandle );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create render pass." );
		destroy();
	}
}
