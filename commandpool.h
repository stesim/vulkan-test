#ifndef COMMANDPOOL_H
#define COMMANDPOOL_H

#include "common.h"

#include <vulkan/vulkan.h>

class Renderer;

class CommandPool
{
public:
	CommandPool();
	CommandPool( Renderer& renderer, uint32_t queueFamily );
	~CommandPool();

	void destroy();

	VkCommandPool  getNativeHandle()
	{
		return m_vkCommandPool;
	}
	bool           isValid()
	{
		return ( m_vkCommandPool != VK_NULL_HANDLE );
	}

	Renderer&      getRenderer()
	{
		return *m_pRenderer;
	}

private:
	bool createCommandPool( uint32_t queueFamily );

private:
	VkCommandPool m_vkCommandPool;

	Renderer*     m_pRenderer;
};

#endif // COMMANDPOOL_H
