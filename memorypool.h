#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include "common.h"

#include <vulkan/vulkan.h>
#include <list>
#include <unordered_map>

class Buffer;
class Renderer;

class MemoryPool
{
public:
	static constexpr uint32_t INVALID_TYPE = ~(uint32_t)0;

	struct Chunk
	{
		uint64_t offset;
		uint64_t size;
		Buffer*  owner;
	};

public:
	MemoryPool();
	MemoryPool( Renderer& renderer,
	            uint64_t size,
	            uint32_t typeFilter,
	            VkMemoryPropertyFlags properties );
	~MemoryPool();

	void           destroy();

	VkDeviceMemory getNativeHandle()
	{
		return m_vkMemory;
	}
	bool           isValid()
	{
		return ( m_vkMemory != VK_NULL_HANDLE );
	}

	Renderer&      getRenderer()
	{
		return *m_pRenderer;
	}

	bool           allocateBufferMemory( Buffer& buffer );
	void           freeBufferMemory( Buffer& buffer );

	void*          map( Buffer& buffer, uint64_t offset, uint64_t length );
	void           unmap();

private:
	bool determineCompatibleMemoryType( uint32_t filter,
	                                    VkMemoryPropertyFlags properties );

	bool allocateMemory( uint64_t size );

private:
	VkDeviceMemory m_vkMemory;
	uint32_t       m_vkType;

	Renderer*                           m_pRenderer;
	uint64_t                            m_uSize;
	std::list<Chunk>                    m_Chunks;
	std::unordered_map<Buffer*, Chunk*> m_BufferChunkMap;
};

#endif // MEMORYPOOL_H
