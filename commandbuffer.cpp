#include "commandbuffer.h"
#include "commandpool.h"
#include "renderer.h"
#include "pipeline.h"
#include "buffer.h"

CommandBuffer::CommandBuffer()
    : m_vkCommandBuffer( VK_NULL_HANDLE ),
      m_pPool( nullptr )
{
}

CommandBuffer::CommandBuffer( CommandPool& pool )
    : CommandBuffer()
{
	m_pPool = &pool;

	if( !allocateBuffer() )
	{
		destroy();
	}
}

CommandBuffer::~CommandBuffer()
{
	destroy();
}

void CommandBuffer::destroy()
{
	if( m_vkCommandBuffer != VK_NULL_HANDLE )
	{
		vkFreeCommandBuffers( m_pPool->getRenderer().getNativeDeviceHandle(),
		                      m_pPool->getNativeHandle(),
		                      1,
		                      &m_vkCommandBuffer );
		m_vkCommandBuffer = VK_NULL_HANDLE;
	}

	m_pPool = nullptr;
}

bool CommandBuffer::begin( VkCommandBufferUsageFlags usage )
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext            = nullptr;
	beginInfo.flags            = usage;
	beginInfo.pInheritanceInfo = nullptr;

	VkResult res = vkBeginCommandBuffer( m_vkCommandBuffer, &beginInfo );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot begin command buffer recording." );
		return false;
	}
	return true;
}

bool CommandBuffer::end()
{
	VkResult res = vkEndCommandBuffer( m_vkCommandBuffer );
	return ( res == VK_SUCCESS );
}

void CommandBuffer::beginRenderPass( VkRenderPass renderPass,
                                     VkFramebuffer frambuffer,
                                     VkRect2D renderArea,
                                     std::vector<VkClearValue> clearValues )
{
	    VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.pNext             = nullptr;
		renderPassInfo.renderPass        = renderPass;
		renderPassInfo.framebuffer       = frambuffer;
		renderPassInfo.renderArea        = renderArea;
		renderPassInfo.clearValueCount   = clearValues.size();
		renderPassInfo.pClearValues      = clearValues.data();

		// TODO: subpass contents selection
		vkCmdBeginRenderPass( m_vkCommandBuffer,
		                      &renderPassInfo,
		                      VK_SUBPASS_CONTENTS_INLINE );
}

void CommandBuffer::endRenderPass()
{
	vkCmdEndRenderPass( m_vkCommandBuffer );
}

void CommandBuffer::bindPipeline( VkPipelineBindPoint bindPoint, Pipeline& pipeline )
{
	vkCmdBindPipeline( m_vkCommandBuffer, bindPoint, pipeline.getNativeHandle() );
}

void CommandBuffer::bindVertexBuffers( uint32_t firstIndex,
                                       std::vector<Buffer*> buffers,
                                       std::vector<uint64_t> offsets )
{
	std::vector<VkBuffer> bufferHandles( buffers.size() );
	for( auto i = 0; i < bufferHandles.size(); ++i )
	{
		bufferHandles[ i ] = buffers[ i ]->getNativeHandle();
	}

	vkCmdBindVertexBuffers( m_vkCommandBuffer,
	                        firstIndex,
	                        buffers.size(),
	                        bufferHandles.data(),
	                        offsets.data() );
}

void CommandBuffer::bindIndexBuffer( Buffer& buffer, uint64_t offset, VkIndexType indexType )
{
	vkCmdBindIndexBuffer( m_vkCommandBuffer, buffer.getNativeHandle(), offset, indexType );
}

void CommandBuffer::setViewports( uint32_t firstIndex, std::vector<VkViewport> viewports )
{
	vkCmdSetViewport( m_vkCommandBuffer, firstIndex, viewports.size(), viewports.data() );
}

void CommandBuffer::setScissors( uint32_t firstIndex, std::vector<VkRect2D> scissors )
{
	vkCmdSetScissor( m_vkCommandBuffer, firstIndex, scissors.size(), scissors.data() );
}

void CommandBuffer::draw( uint32_t firstVertex,
                          uint32_t numVertices,
                          uint32_t firstInstance,
                          uint32_t numInstances )
{
	vkCmdDraw( m_vkCommandBuffer,
	           numVertices,
	           numInstances,
	           firstVertex,
	           firstInstance );
}

void CommandBuffer::drawIndexed( uint32_t firstIndex,
                                 uint32_t numIndices,
                                 uint32_t firstVertex,
                                 uint32_t firstInstance,
                                 uint32_t numInstances )
{
	vkCmdDrawIndexed( m_vkCommandBuffer,
	                  numIndices,
	                  numInstances,
	                  firstIndex,
	                  firstVertex,
	                  firstInstance );
}

void CommandBuffer::copyBuffer( Buffer& dst, Buffer& src, std::vector<VkBufferCopy> regions )
{
	vkCmdCopyBuffer( m_vkCommandBuffer,
	                 src.getNativeHandle(),
	                 dst.getNativeHandle(),
	                 regions.size(),
	                 regions.data() );
}

bool CommandBuffer::allocateBuffer()
{
	// TODO: buffer level selection
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext              = nullptr;
	allocInfo.commandPool        = m_pPool->getNativeHandle();
	allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkResult res = vkAllocateCommandBuffers( m_pPool->getRenderer().getNativeDeviceHandle(),
	                                         &allocInfo,
	                                         &m_vkCommandBuffer );
	if( res != VK_SUCCESS )
	{
		log_error( "Cannot allocate command buffer." );
		return false;
	}
	return true;
}
