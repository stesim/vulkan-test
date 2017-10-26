#ifndef BUFFER_H
#define BUFFER_H

#include "common.h"
#include "vulkanobjectwrapper.h"

#include <vulkan/vulkan.h>
#include <vector>

class Renderer;
class MemoryPool;

class Buffer : public VulkanObjectWrapper<VkBuffer, vkDestroyBuffer>
{
public:
	Buffer() = default;
	Buffer( Renderer& renderer, uint64_t size, VkBufferUsageFlags usage );
	Buffer( Renderer& renderer,
	        uint64_t size,
	        VkBufferUsageFlags usage,
	        std::vector<uint32_t> queues );
	Buffer( MemoryPool& pool,
	        uint64_t size,
	        VkBufferUsageFlags usage );
	Buffer( MemoryPool& pool,
	        uint64_t size,
	        VkBufferUsageFlags usage,
	        std::vector<uint32_t> queues );

	virtual ~Buffer();

	uint64_t getSize()
	{
		return m_uSize;
	}

	void     getMemoryRequirements( uint64_t* alignment, uint32_t* typeFilter, uint64_t* size );

	bool     allocateMemoryFromPool( MemoryPool& pool );
	void     freeMemory();

	void*    map();
	void*    map( uint64_t offset, uint64_t length );
	void     unmap();

	void     invalidate( Buffer&, uint64_t offset, uint64_t length ); // TODO: implement
	void     flush( Buffer&, uint64_t offset, uint64_t length );      // TODO: implement

private:
	bool     createBuffer( uint64_t size,
	                       VkBufferUsageFlags usage,
	                       const std::vector<uint32_t>& queues );

private:
	Renderer*   m_pRenderer;
	MemoryPool* m_pMemoryPool;
	uint64_t    m_uSize;
};

#endif // BUFFER_H
