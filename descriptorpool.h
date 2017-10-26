#ifndef DESCRIPTORPOOL_H
#define DESCRIPTORPOOL_H

#include "common.h"
#include "vulkanobjectwrapper.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

class Renderer;

class DescriptorPool : public VulkanObjectWrapper<VkDescriptorPool, vkDestroyDescriptorPool>
{
public:
	DescriptorPool() = default;
	DescriptorPool( Renderer& renderer,
	                std::vector<VkDescriptorPoolSize> poolSizes,
	                uint32_t maxSets );

	Renderer& getRenderer()
	{
		return *m_pRenderer;
	}

private:
	Renderer* m_pRenderer;

	std::unordered_map<VkDescriptorType, uint32_t> m_PoolSizes;
};

#endif // DESCRIPTORPOOL_H
