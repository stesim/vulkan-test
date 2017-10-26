#include "descriptorpool.h"
#include "renderer.h"

DescriptorPool::DescriptorPool( Renderer& renderer,
                                std::vector<VkDescriptorPoolSize> poolSizes,
                                uint32_t maxSets )
    : wrapper_type( renderer.getNativeDeviceHandle() ),
      m_pRenderer( &renderer ),
      m_PoolSizes()
{
	for( const auto& size : poolSizes )
	{
		m_PoolSizes[ size.type ] = size.descriptorCount;
	}

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.pNext         = nullptr;
	createInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // TODO: make configurable
	createInfo.poolSizeCount = poolSizes.size();
	createInfo.pPoolSizes    = poolSizes.data();
	createInfo.maxSets       = maxSets;

	VkResult res = vkCreateDescriptorPool( m_vkDevice,
	                                       &createInfo,
	                                       nullptr,
	                                       &m_vkHandle );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create descriptor pool." );
		destroy();
	}
}
