#ifndef COMMANDPOOL_H
#define COMMANDPOOL_H

#include "common.h"
#include "vulkanobjectwrapper.h"

#include <vulkan/vulkan.h>

class Renderer;

class CommandPool : public VulkanObjectWrapper<VkCommandPool, vkDestroyCommandPool>
{
public:
	CommandPool() = default;
	CommandPool( Renderer& renderer, uint32_t queueFamily );

	Renderer&      getRenderer()
	{
		return *m_pRenderer;
	}

private:
	Renderer*     m_pRenderer;
};

#endif // COMMANDPOOL_H
