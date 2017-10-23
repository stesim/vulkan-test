#ifndef PIPELINE_H
#define PIPELINE_H

#include "common.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class Renderer;
class Shader;

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
	Pipeline( Renderer& renderer, const std::vector<Shader*>& shaders );
	~Pipeline();

	void destroy();

	VkPipeline getNativeHandle()
	{
		return m_vkPipeline;
	}
	bool       isValid()
	{
		return ( m_vkPipeline != VK_NULL_HANDLE );
	}

	VkRenderPass getRenderPass()
	{
		return m_vkRenderPass;
	}

private:
	static std::vector<char> readFile( const std::string& filename );

	static VkPipelineShaderStageCreateInfo createShaderStageCreateInfo( Shader& shader );

	VkShaderModule createShaderModule( const std::vector<char>& code );

	void populateFixedFunctionSetup( FixedFunctionSetup& ffs );

	bool createRenderPass();
	bool createLayout();
	bool createPipeline( const std::vector<Shader*>& shaders );

private:
	VkPipeline       m_vkPipeline;
	VkPipelineLayout m_vkLayout;
	VkRenderPass     m_vkRenderPass;

	Renderer* m_pRenderer;
};

#endif // PIPELINE_H
