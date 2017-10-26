#include "descriptorset.h"
#include "descriptorpool.h"
#include "descriptorsetlayout.h"
#include "renderer.h"
#include "buffer.h"

DescriptorSet::Writer::Writer( DescriptorSet& set )
    : m_pSet( &set ),
      m_uCurrentBinding( INVALID_BINDING ),
      m_Buffers(),
      m_Writes()
{
}

DescriptorSet::Writer& DescriptorSet::Writer::uniformBuffer( uint32_t binding,
                                                             Buffer&  buffer,
                                                             uint64_t offset,
                                                             uint64_t length )
{
	m_uCurrentBinding = binding;

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer.getNativeHandle();
	bufferInfo.offset = offset;
	bufferInfo.range  = length;

	m_Buffers.emplace_back( bufferInfo );

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext            = nullptr;
	descriptorWrite.dstSet           = m_pSet->getNativeHandle();
	descriptorWrite.dstBinding       = m_uCurrentBinding;
	descriptorWrite.dstArrayElement  = 0;
	descriptorWrite.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount  = 1;
	descriptorWrite.pBufferInfo      = &m_Buffers.back();
	descriptorWrite.pImageInfo       = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;

	m_Writes.emplace_back( descriptorWrite );

	return *this;
}


DescriptorSet::DescriptorSet( DescriptorPool& pool,
                              const std::vector<DescriptorSetLayout*>& layouts )
    : m_vkDevice( pool.getRenderer().getNativeDeviceHandle() ),
      m_vkHandle( VK_NULL_HANDLE ),
      m_pPool( &pool )
{
	std::vector<VkDescriptorSetLayout> layoutHandles( layouts.size() );
	for( auto i = 0; i < layouts.size(); ++i )
	{
		layoutHandles[ i ] = layouts[ i ]->getNativeHandle();
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext              = nullptr;
	allocInfo.descriptorPool     = pool.getNativeHandle();
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts        = layoutHandles.data();

	VkResult res = vkAllocateDescriptorSets( m_vkDevice, &allocInfo, &m_vkHandle );
	if( res != VK_SUCCESS )
	{
		log_error( "Cannot allocate descriptor sets." );
		destroy();
	}
}

DescriptorSet::~DescriptorSet()
{
	destroy();
}

void DescriptorSet::destroy()
{
	if( m_vkHandle != VK_NULL_HANDLE )
	{
		vkFreeDescriptorSets( m_vkDevice, m_pPool->getNativeHandle(), 1, &m_vkHandle );
		m_vkHandle = VK_NULL_HANDLE;
	}
}

void DescriptorSet::update( Writer& writer )
{
	vkUpdateDescriptorSets( m_vkDevice,
	                        writer.getWrites().size(),
	                        ( writer.getWrites().empty() ? nullptr : writer.getWrites().data() ),
	                        writer.getCopies().size(),
	                        ( writer.getCopies().empty() ? nullptr : writer.getCopies().data() ) );
}
