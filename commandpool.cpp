#include "commandpool.h"
#include "renderer.h"

CommandPool::CommandPool( Renderer& renderer, uint32_t queueFamily )
    : wrapper_type( renderer.getNativeDeviceHandle() ),
      m_pRenderer( &renderer )
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.pNext            = nullptr;
	createInfo.flags            = 0;
	createInfo.queueFamilyIndex = queueFamily;

	VkResult res = vkCreateCommandPool( m_vkDevice,
	                                    &createInfo,
	                                    nullptr,
	                                    &m_vkHandle );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create command pool." );
		destroy();
	}
}
