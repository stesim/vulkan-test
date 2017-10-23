#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include "common.h"

#include <vulkan/vulkan.h>
#include <vector>

class CommandPool;
class Pipeline;
class Buffer;

class CommandBuffer
{
public:
	CommandBuffer();
	CommandBuffer( CommandPool& pool );
	~CommandBuffer();

	void destroy();

	VkCommandBuffer getNativeHandle()
	{
		return m_vkCommandBuffer;
	}
	bool            isValid()
	{
		return ( m_vkCommandBuffer != VK_NULL_HANDLE );
	}

	bool begin( VkCommandBufferUsageFlags usage );
	bool end();

	// TODO: replace render pass and framebuffer with wrapper classes
	void beginRenderPass( VkRenderPass renderPass,
	                      VkFramebuffer frambuffer,
	                      VkRect2D renderArea,
	                      std::vector<VkClearValue> clearValues );
	void endRenderPass();

	void bindPipeline( VkPipelineBindPoint bindPoint, Pipeline& pipeline );
	void bindVertexBuffers( uint32_t firstIndex,
	                        std::vector<Buffer*> buffers,
	                        std::vector<uint64_t> offsets );
	void bindIndexBuffer( Buffer& buffer, uint64_t offset, VkIndexType indexType );

	void setViewports( uint32_t firstIndex, std::vector<VkViewport> viewports );
	void setScissors( uint32_t firstIndex, std::vector<VkRect2D> scissors );

	void draw( uint32_t firstVertex,
	           uint32_t numVertices,
	           uint32_t firstInstance,
	           uint32_t numInstances );
	void drawIndexed( uint32_t firstIndex,
	                  uint32_t numIndices,
	                  uint32_t firstVertex,
	                  uint32_t firstInstance,
	                  uint32_t numInstances );

	void copyBuffer( Buffer& dst, Buffer& src, std::vector<VkBufferCopy> regions );

private:
	bool allocateBuffer();

private:
	VkCommandBuffer m_vkCommandBuffer;

	CommandPool*    m_pPool;
};

#endif // COMMANDBUFFER_H
