#ifndef DESCRIPTORSET_H
#define DESCRIPTORSET_H

#include "common.h"
#include "vulkanobjectwrapper.h"

#include <vulkan/vulkan.h>
#include <vector>

class DescriptorPool;
class DescriptorSetLayout;
class Buffer;

class DescriptorSet
{
public:
	class Writer
	{
	public:
		Writer( DescriptorSet& set );

		Writer& uniformBuffer( uint32_t binding, Buffer& buffer, uint64_t offset, uint64_t length );

		const std::vector<VkWriteDescriptorSet>& getWrites()
		{
			return m_Writes;
		}

		const std::vector<VkCopyDescriptorSet>&  getCopies()
		{
			return m_Copies;
		}

	private:
		static constexpr uint32_t INVALID_BINDING = ~(uint32_t)0;

		DescriptorSet* m_pSet;

		uint32_t       m_uCurrentBinding;

		std::vector<VkDescriptorBufferInfo> m_Buffers;
		std::vector<VkWriteDescriptorSet>   m_Writes;
		std::vector<VkCopyDescriptorSet>    m_Copies;
	};

public:
	DescriptorSet() = default;
	DescriptorSet( DescriptorPool& pool, const std::vector<DescriptorSetLayout*>& layouts );
	~DescriptorSet();

	VkDescriptorSet getNativeHandle()
	{
		return m_vkHandle;
	}
	bool            isValid()
	{
		return ( m_vkHandle != VK_NULL_HANDLE );
	}

	void            destroy();

	void            update( Writer& writer );

private:
	VkDevice        m_vkDevice;
	VkDescriptorSet m_vkHandle;

	DescriptorPool* m_pPool;
};

#endif // DESCRIPTORSET_H
