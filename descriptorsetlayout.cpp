#include "descriptorsetlayout.h"
#include "renderer.h"

DescriptorSetLayout::Composer& DescriptorSetLayout::Composer::stage( VkShaderStageFlagBits stage )
{
	m_vkCurrentStage = stage;
	return *this;
}

DescriptorSetLayout::Composer& DescriptorSetLayout::Composer::uniformBuffer()
{
	return uniformBuffer( 1 );
}

DescriptorSetLayout::Composer& DescriptorSetLayout::Composer::uniformBuffer(
        uint32_t numDescriptors )
{
	VkDescriptorSetLayoutBinding binding{};
	binding.binding            = m_vkBindings.size();
	binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	binding.descriptorCount    = numDescriptors;
	binding.stageFlags         = m_vkCurrentStage;
	binding.pImmutableSamplers = nullptr;

	m_vkBindings.emplace_back( binding );

	return *this;
}


DescriptorSetLayout::DescriptorSetLayout( Renderer& renderer, Composer& composer )
    : wrapper_type( renderer.getNativeDeviceHandle() )
{
	const auto& bindings = composer.getBindings();

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.pNext        = nullptr;
	createInfo.flags        = 0;
	createInfo.bindingCount = bindings.size();
	createInfo.pBindings    = bindings.data();

	VkResult res = vkCreateDescriptorSetLayout( m_vkDevice,
	                                            &createInfo,
	                                            nullptr,
	                                            &m_vkHandle );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create descriptor set layout." );
		destroy();
	}
}

DescriptorSetLayout::Composer DescriptorSetLayout::compose()
{
	return Composer();
}
