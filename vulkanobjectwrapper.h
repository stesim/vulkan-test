#ifndef VULKANOBJECTWRAPPER_H
#define VULKANOBJECTWRAPPER_H

#include "common.h"

#include <vulkan/vulkan.h>

template<typename handle_type,
         void ( *destroy_func )( VkDevice, handle_type, const VkAllocationCallbacks* )>
class VulkanObjectWrapper
{
public:
	typedef VulkanObjectWrapper<handle_type, destroy_func> wrapper_type;

public:
	VulkanObjectWrapper()
	    : VulkanObjectWrapper( VK_NULL_HANDLE, VK_NULL_HANDLE )
	{
	}

	VulkanObjectWrapper( VkDevice device )
	    : VulkanObjectWrapper( device, VK_NULL_HANDLE )
	{
	}

	VulkanObjectWrapper( VkDevice device, handle_type handle )
	    : m_vkHandle( handle ),
	      m_vkDevice( device )
	{
	}

	virtual ~VulkanObjectWrapper()
	{
		destroy();
	}

	void destroy()
	{
		if( m_vkHandle != VK_NULL_HANDLE )
		{
			destroy_func( m_vkDevice, m_vkHandle, nullptr );
			m_vkHandle = VK_NULL_HANDLE;
		}
	}

	handle_type getNativeHandle()
	{
		return m_vkHandle;
	}

	bool isValid()
	{
		return ( m_vkHandle != VK_NULL_HANDLE );
	}

protected:
	handle_type m_vkHandle;
	VkDevice    m_vkDevice;
};

#endif // VULKANOBJECTWRAPPER_H
