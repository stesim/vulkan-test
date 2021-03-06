#include "buffer.h"
#include "memorypool.h"
#include "renderer.h"

Buffer::Buffer( Renderer& renderer, uint64_t size, VkBufferUsageFlags usage )
    : Buffer( renderer, size, usage, {} )
{
}

Buffer::Buffer( Renderer& renderer,
                uint64_t size,
                VkBufferUsageFlags usage,
                std::vector<uint32_t> queues )
    : wrapper_type( renderer.getNativeDeviceHandle() ),
      m_pRenderer( &renderer ),
      m_pMemoryPool( nullptr ),
      m_uSize( 0 )
{
	if( !createBuffer( size, usage, queues ) )
	{
		destroy();
	}
}

Buffer::Buffer( MemoryPool& pool,
                uint64_t size,
                VkBufferUsageFlags usage )
    : Buffer( pool, size, usage, {} )
{
}

Buffer::Buffer( MemoryPool& pool,
                uint64_t size,
                VkBufferUsageFlags usage,
                std::vector<uint32_t> queues )
    : Buffer( pool.getRenderer(), size, usage, queues )
{
	if( !allocateMemoryFromPool( pool ) )
	{
		destroy();
	}
}

Buffer::~Buffer()
{
	freeMemory();
}

void Buffer::getMemoryRequirements( uint64_t* alignment, uint32_t* typeFilter, uint64_t* size )
{
	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements( m_pRenderer->getNativeDeviceHandle(),
	                               m_vkHandle,
	                               &requirements );

	if( alignment  != nullptr ) *alignment  = requirements.alignment;
	if( typeFilter != nullptr ) *typeFilter = requirements.memoryTypeBits;
	if( size       != nullptr ) *size       = requirements.size;
}

bool Buffer::allocateMemoryFromPool( MemoryPool& pool )
{
#ifndef NDEBUG
	if( m_pMemoryPool != nullptr )
	{
		log_error( "Buffer already has allocated memory." );
	}
	if( &pool.getRenderer() != m_pRenderer )
	{
		log_warning( "Renderers associated with buffer and memory pool do not match." );
	}
#endif

	if( pool.allocateBufferMemory( *this ) )
	{
		m_pMemoryPool = &pool;
		return true;
	}
	else
	{
		return false;
	}
}

void Buffer::freeMemory()
{
#ifndef NDEBUG
	if( m_pMemoryPool == nullptr )
	{
		log_error( "Attempting to free unallocated buffer memory." );
	}
#endif

	m_pMemoryPool->freeBufferMemory( *this );
	m_pMemoryPool = nullptr;
}

void* Buffer::map()
{
	return map( 0, m_uSize );
}

void* Buffer::map( uint64_t offset, uint64_t length )
{
	return m_pMemoryPool->map( *this, offset, length );
}

void Buffer::unmap()
{
	m_pMemoryPool->unmap();
}

bool Buffer::createBuffer( uint64_t size,
                           VkBufferUsageFlags usage,
                           const std::vector<uint32_t>& queues )
{
	VkBufferCreateInfo createInfo{};
	createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext                 = nullptr;
	createInfo.flags                 = 0;
	createInfo.size                  = size;
	createInfo.usage                 = usage;
	createInfo.sharingMode           = ( queues.empty() ? VK_SHARING_MODE_EXCLUSIVE
	                                                    : VK_SHARING_MODE_CONCURRENT );
	createInfo.queueFamilyIndexCount = queues.size();
	createInfo.pQueueFamilyIndices   = ( !queues.empty() ? queues.data() : nullptr );

	VkResult res = vkCreateBuffer( m_pRenderer->getNativeDeviceHandle(),
	                               &createInfo,
	                               nullptr,
	                               &m_vkHandle );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create buffer." );
		return false;
	}
	else
	{
		m_uSize = size;
		return true;
	}
}
