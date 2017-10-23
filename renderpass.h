#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "common.h"
#include "vulkanobjectwrapper.h"

#include <vulkan/vulkan.h>

class Renderer;

class RenderPass : public VulkanObjectWrapper<VkRenderPass, vkDestroyRenderPass>
{
public:
	RenderPass() = default;
	RenderPass( Renderer& renderer, VkFormat format );

	Renderer& getRenderer()
	{
		return *m_pRenderer;
	}

private:
	Renderer* m_pRenderer;
};

#endif // RENDERPASS_H
