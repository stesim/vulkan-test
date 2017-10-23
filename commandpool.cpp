#include "commandpool.h"
#include "renderer.h"

CommandPool::CommandPool()
    : m_vkCommandPool( VK_NULL_HANDLE ),
      m_pRenderer( nullptr )
{
}

CommandPool::CommandPool( Renderer& renderer, uint32_t queueFamily )
    : CommandPool()
{
	m_pRenderer = &renderer;

	if( !createCommandPool( queueFamily ) )
	{
		destroy();
	}
}

CommandPool::~CommandPool()
{
	destroy();
}

void CommandPool::destroy()
{
	if( m_vkCommandPool != VK_NULL_HANDLE )
	{
		vkDestroyCommandPool( m_pRenderer->getNativeDeviceHandle(),
		                      m_vkCommandPool,
		                      nullptr );
		m_vkCommandPool = VK_NULL_HANDLE;
	}

	m_pRenderer = nullptr;
}

bool CommandPool::createCommandPool( uint32_t queueFamily )
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.pNext            = nullptr;
	createInfo.flags            = 0;
	createInfo.queueFamilyIndex = queueFamily;

	VkResult res = vkCreateCommandPool( m_pRenderer->getNativeDeviceHandle(),
	                                    &createInfo,
	                                    nullptr,
	                                    &m_vkCommandPool );
	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create command pool." );
		return false;
	}
	return true;
}
