#include "shader.h"
#include "renderer.h"

Shader::Shader( Renderer& renderer,
                const std::vector<char>& code,
                std::string entryFunc,
                VkShaderStageFlagBits stage )
    : VulkanObjectWrapper<VkShaderModule, vkDestroyShaderModule>( renderer.getNativeDeviceHandle() ),
      m_vkStage( stage ),
      m_strEntryFunc( entryFunc )
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.pNext    = nullptr;
	createInfo.flags    = 0;
	createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode    = reinterpret_cast<const uint32_t*>( code.data() );

	VkResult res = vkCreateShaderModule( m_vkDevice,
	                                     &createInfo,
	                                     nullptr,
	                                     &m_vkHandle );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create shader module." );
		destroy();
	}
}
