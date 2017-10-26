#ifndef DESCRIPTORSETLAYOUT_H
#define DESCRIPTORSETLAYOUT_H

#include "common.h"
#include "vulkanobjectwrapper.h"

#include <vulkan/vulkan.h>
#include <vector>

class Renderer;

class DescriptorSetLayout
        : public VulkanObjectWrapper<VkDescriptorSetLayout, vkDestroyDescriptorSetLayout>
{
public:
	class Composer
	{
	public:
		Composer& stage( VkShaderStageFlagBits stage );

		Composer& uniformBuffer();
		Composer& uniformBuffer( uint32_t numDescriptors );

		const std::vector<VkDescriptorSetLayoutBinding>& getBindings()
		{
			return m_vkBindings;
		}

	private:
		VkShaderStageFlagBits                     m_vkCurrentStage;
		std::vector<VkDescriptorSetLayoutBinding> m_vkBindings;
	};

public:
	DescriptorSetLayout() = default;
	DescriptorSetLayout( Renderer& renderer, Composer& composer );

	static Composer compose();
};

#endif // DESCRIPTORSETLAYOUT_H
