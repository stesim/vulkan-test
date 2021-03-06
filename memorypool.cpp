#include "memorypool.h"
#include "buffer.h"
#include "renderer.h"

MemoryPool::MemoryPool( Renderer& renderer,
                        uint64_t size,
                        uint32_t typeFilter,
                        VkMemoryPropertyFlags properties )
    : wrapper_type( renderer.getNativeDeviceHandle() ),
      m_pRenderer( &renderer ),
      m_uSize( 0 ),
      m_Chunks(),
      m_BufferChunkMap()
{
	if( !determineCompatibleMemoryType( typeFilter, properties ) ||
	    !allocateMemory( size ) )
	{
		destroy();
	}
}

bool MemoryPool::allocateBufferMemory( Buffer& buffer )
{
	uint64_t alignment, size;
	uint32_t typeFilter;
	buffer.getMemoryRequirements( &alignment, &typeFilter, &size );

	uint64_t lastChunkEndAligned = 0;
	auto insertPos = m_Chunks.end();
	for( auto iter = m_Chunks.begin(); iter != m_Chunks.end(); ++iter )
	{
		if( iter->offset - lastChunkEndAligned >= size )
		{
			insertPos = iter;
			break;
		}

		lastChunkEndAligned = iter->offset + iter->size;

		// align last chunk end position to buffer alignment
		if( lastChunkEndAligned % alignment != 0 )
		{
			lastChunkEndAligned += ( alignment - lastChunkEndAligned % alignment );
		}
	}

	if( insertPos != m_Chunks.end() ||
	    m_uSize - lastChunkEndAligned >= size )
	{
		VkResult res = vkBindBufferMemory( m_vkDevice,
		                                   buffer.getNativeHandle(),
		                                   m_vkHandle,
		                                   lastChunkEndAligned );

		if( res == VK_SUCCESS )
		{
			auto chunk = m_Chunks.insert( insertPos, { lastChunkEndAligned, size, &buffer } );
			m_BufferChunkMap[ &buffer ] = &*chunk;
			return true;
		}
	}

	log_error( "Cannot allocate buffer memory in pool." );
	return false;
}

void MemoryPool::freeBufferMemory( Buffer& buffer )
{
	for( auto iter = m_Chunks.begin(); iter != m_Chunks.end(); ++iter )
	{
		if( iter->owner == &buffer )
		{
			m_Chunks.erase( iter );
			m_BufferChunkMap.erase( &buffer );
			return;
		}
	}
}

void* MemoryPool::map( Buffer& buffer, uint64_t offset, uint64_t length )
{
	const Chunk& chunk = *m_BufferChunkMap[ &buffer ];

#ifndef NDEBUG
	if( offset + length > buffer.getSize() )
	{
		log_warning( "Buffer memory map region exceeds buffer size." );
	}
#endif

	void* data;
	VkResult res = vkMapMemory( m_vkDevice,
	                            m_vkHandle,
	                            chunk.offset + offset,
	                            length,
	                            0,
	                            &data );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot map buffer memory." );
		return nullptr;
	}

	return data;
}

void MemoryPool::unmap()
{
	vkUnmapMemory( m_vkDevice, m_vkHandle );
}

bool MemoryPool::determineCompatibleMemoryType( uint32_t filter,
                                                VkMemoryPropertyFlags properties )
{
	VkPhysicalDeviceMemoryProperties deviceMemory;
	vkGetPhysicalDeviceMemoryProperties( m_pRenderer->getNativePhysicalDeviceHandle(),
	                                     &deviceMemory );

	for( auto i = 0; i < deviceMemory.memoryTypeCount; ++i )
	{
		if( ( filter & ( 1 << i ) ) != 0 &&
		    ( deviceMemory.memoryTypes[ i ].propertyFlags & properties ) == properties )
		{
			m_vkType = i;
			return true;
		}
	}

	log_error( "Cannot find suitable memory type." );
	return false;
}

bool MemoryPool::allocateMemory( uint64_t size )
{
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext           = nullptr;
	allocInfo.allocationSize  = size;
	allocInfo.memoryTypeIndex = m_vkType;

	VkResult res = vkAllocateMemory( m_vkDevice,
	                                 &allocInfo,
	                                 nullptr,
	                                 &m_vkHandle );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot allocate pool memory." );
		return false;
	}
	else
	{
		m_uSize = size;
		return true;
	}
}
