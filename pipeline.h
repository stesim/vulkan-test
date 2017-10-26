#ifndef PIPELINE_H
#define PIPELINE_H

#include "common.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class Renderer;
class Shader;
class RenderPass;
class DescriptorSetLayout;

class Pipeline
{
private:
	struct FixedFunctionSetup
	{
		VkPipelineVertexInputStateCreateInfo             vertexInput;
		std::vector<VkVertexInputBindingDescription>     vertexInputBindings;
		std::vector<VkVertexInputAttributeDescription>   vertexInputAttributes;
		VkPipelineInputAssemblyStateCreateInfo           inputAssembly;
		VkPipelineViewportStateCreateInfo                viewport;
		std::vector<VkViewport>                          viewportViewports;
		std::vector<VkRect2D>                            viewportScissors;
		VkPipelineRasterizationStateCreateInfo           rasterization;
		VkPipelineMultisampleStateCreateInfo             multisampling;
		VkPipelineColorBlendStateCreateInfo              colorBlend;
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
	};

public:
	Pipeline();
	Pipeline( RenderPass& renderPass,
	          const std::vector<Shader*>& shaders,
	          const std::vector<DescriptorSetLayout*>& descriptorLayouts );
	~Pipeline();

	void         destroy();

	VkPipeline   getNativeHandle()
	{
		return m_vkPipeline;
	}
	bool         isValid()
	{
		return ( m_vkPipeline != VK_NULL_HANDLE );
	}

	RenderPass&  getRenderPass()
	{
		return *m_pRenderPass;
	}

	VkPipelineLayout getLayout()
	{
		return m_vkLayout;
	}

private:
	static VkPipelineShaderStageCreateInfo createShaderStageCreateInfo( Shader& shader );

	void populateFixedFunctionSetup( FixedFunctionSetup& ffs );

	bool createLayout( const std::vector<DescriptorSetLayout*>& descriptorLayouts );
	bool createPipeline( const std::vector<Shader*>& shaders );

private:
	VkPipeline       m_vkPipeline;
	VkPipelineLayout m_vkLayout;

	RenderPass* m_pRenderPass;
};

#endif // PIPELINE_H
