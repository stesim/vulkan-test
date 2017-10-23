#ifndef SHADER_H
#define SHADER_H

#include "common.h"
#include "vulkanobjectwrapper.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class Renderer;

class Shader : public VulkanObjectWrapper<VkShaderModule, vkDestroyShaderModule>
{
public:
	Shader() = default;
	Shader( Renderer& renderer,
	        const std::vector<char>& code,
	        std::string entryFunc,
	        VkShaderStageFlagBits stage );

	const std::string&    getEntryFunction()
	{
		return m_strEntryFunc;
	}

	VkShaderStageFlagBits getStage()
	{
		return m_vkStage;
	}

private:
	VkShaderStageFlagBits m_vkStage;

	std::string           m_strEntryFunc;
};

#endif // SHADER_H
