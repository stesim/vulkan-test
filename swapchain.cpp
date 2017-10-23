#include "swapchain.h"
#include "window.h"
#include "windowsurface.h"
#include "renderer.h"

#include <limits>
#include <algorithm>

SwapChain::SwapChain()
    : m_vkSwapchain( VK_NULL_HANDLE ),
      m_vkFormat(),
      m_vkExtent(),
      m_vkImages(),
      m_vkImageViews(),
      m_pRenderer( nullptr )
{
}

SwapChain::SwapChain( Renderer& renderer )
    : SwapChain()
{
	m_pRenderer = &renderer;

	if( !createSwapchain( nullptr ) ||
	    !createImageViews() )
	{
		destroy();
	}
}

SwapChain::SwapChain( SwapChain& oldSwapchain )
    : SwapChain()
{
	m_pRenderer = oldSwapchain.m_pRenderer;

	if( !createSwapchain( &oldSwapchain ) ||
	    !createImageViews() )
	{
		destroy();
	}
}

SwapChain::~SwapChain()
{
	destroy();
}

//SwapChain& SwapChain::operator=( SwapChain&& rhs )
//{
//	if( rhs.m_vkSwapchain == VK_NULL_HANDLE )
//	{
//		destroy();
//	}
//	else
//	{
//		m_vkSwapchain = rhs.m_vkSwapchain;
//		rhs.m_vkSwapchain = VK_NULL_HANDLE;
//
//		m_vkFormat = rhs.m_vkFormat;
//		m_vkExtent = rhs.m_vkExtent;
//
//		m_vkImages = std::move( rhs.m_vkImages );
//
//		m_pRenderer = rhs.m_pRenderer;
//	}
//	return *this;
//}

void SwapChain::destroy()
{
	VkDevice device = m_pRenderer->getNativeDeviceHandle();

	for( auto i = 0; i < m_vkImageViews.size(); ++i )
	{
		if( m_vkImageViews[ i ] != VK_NULL_HANDLE )
		{
			vkDestroyImageView( device, m_vkImageViews[ i ], nullptr );
		}
	}

	if( m_vkSwapchain != VK_NULL_HANDLE )
	{
		vkDestroySwapchainKHR( device, m_vkSwapchain, nullptr );
		m_vkSwapchain = VK_NULL_HANDLE;
	}
}

SwapChain::Capabilities SwapChain::queryDeviceCapabilities( VkPhysicalDevice device,
                                                            VkSurfaceKHR surface )
{
	Capabilities capabilities;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface, &capabilities.capabilities );

	uint32_t numFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &numFormats, nullptr );
	if( numFormats > 0 )
	{
		capabilities.formats.resize( numFormats );
		vkGetPhysicalDeviceSurfaceFormatsKHR( device,
		                                      surface,
		                                      &numFormats,
		                                      capabilities.formats.data() );
	}

	uint32_t numPresentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &numPresentModes, nullptr );
	if( numPresentModes > 0 )
	{
		capabilities.presentModes.resize( numPresentModes );
		vkGetPhysicalDeviceSurfacePresentModesKHR( device,
		                                           surface,
		                                           &numPresentModes,
		                                           capabilities.presentModes.data() );
	}

	return capabilities;
}

VkSurfaceFormatKHR SwapChain::selectSurfaceFormat( const Capabilities& capabilities )
{
	if( capabilities.formats.size() == 1 &&
	    capabilities.formats[ 0 ].format == VK_FORMAT_UNDEFINED ) // no format restrictions
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for( const auto& format : capabilities.formats )
	{
		if( format.format == VK_FORMAT_B8G8R8A8_UNORM &&
		    format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
		{
			return format;
		}
	}

	log_warning( "Cannot find optimal surface format." );

	return capabilities.formats[ 0 ];
}

VkPresentModeKHR SwapChain::selectPresentMode( const Capabilities& capabilities )
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR; // FIFO support is guaranteed
//	for( const auto& mode : capabilities.presentModes )
//	{
//		if( mode == VK_PRESENT_MODE_MAILBOX_KHR /*||
//			( mode == VK_PRESENT_MODE_IMMEDIATE_KHR && bestMode == VK_PRESENT_MODE_FIFO_KHR )*/ )
//		{ // uncomment above line to prefer immediate mode instead of FIFO
//			bestMode = mode;
//		}
//	}
//
//	if( bestMode != VK_PRESENT_MODE_MAILBOX_KHR )
//	{
//		log_warning( "Cannot find optimal present mode." );
//	}

	return bestMode;
}

VkExtent2D SwapChain::selectSurfaceExtent( const Capabilities& capabilities )
{
	if( capabilities.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() )
	{
		return capabilities.capabilities.currentExtent;
	}
	else
	{
		if( m_pRenderer == nullptr )
		{
			log_error( "Cannot determine swap chain extent." );
			return {};
		}

		return {
			std::max( capabilities.capabilities.minImageExtent.width,
			          std::min( m_pRenderer->getWindowSurface().getWindow().getWidth(),
			                    capabilities.capabilities.maxImageExtent.width ) ),
			std::max( capabilities.capabilities.minImageExtent.height,
			          std::min( m_pRenderer->getWindowSurface().getWindow().getHeight(),
			                    capabilities.capabilities.maxImageExtent.height ) )
		};
	}
}

bool SwapChain::createSwapchain( SwapChain* oldSwapchain )
{
	Capabilities capabilities = queryDeviceCapabilities(
	                                m_pRenderer->getNativePhysicalDeviceHandle(),
	                                m_pRenderer->getWindowSurface().getNativeHandle() );

	uint32_t         numImages;
	m_vkFormat = selectSurfaceFormat( capabilities );
	m_vkExtent = selectSurfaceExtent( capabilities );
	m_vkPresentMode = selectPresentMode( capabilities );

	numImages = capabilities.capabilities.minImageCount + 1;
	if( capabilities.capabilities.maxImageCount > 0 && // maxImageCount = 0 means no restriction
	    numImages > capabilities.capabilities.maxImageCount )
	{
		numImages = capabilities.capabilities.maxImageCount;
	}

	const Renderer::QueueFamilies& queueFamilies = m_pRenderer->getQueueFamilies();
	uint32_t queueFamilyIndices[] = {
	    queueFamilies[ QueueFamily::Graphics ].index,
	    queueFamilies[ QueueFamily::Present ].index
	};

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext            = nullptr;
	createInfo.flags            = 0;
	createInfo.surface          = m_pRenderer->getWindowSurface().getNativeHandle();
	createInfo.minImageCount    = numImages;
	createInfo.imageFormat      = m_vkFormat.format;
	createInfo.imageColorSpace  = m_vkFormat.colorSpace;
	createInfo.imageExtent      = m_vkExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if( queueFamilies[ QueueFamily::Graphics ].index ==
	    queueFamilies[ QueueFamily::Present ].index )
	{
		createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices   = nullptr;
	}
	else
	{
		createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices   = queueFamilyIndices;
	}
	createInfo.preTransform     = capabilities.capabilities.currentTransform;
	createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // transparent windows anyone?
	createInfo.presentMode      = m_vkPresentMode;
	createInfo.clipped          = VK_TRUE;
	createInfo.oldSwapchain     = ( oldSwapchain != nullptr ? oldSwapchain->m_vkSwapchain
	                                                        : nullptr );

	VkDevice device = m_pRenderer->getNativeDeviceHandle();

	VkResult res = vkCreateSwapchainKHR( device, &createInfo, nullptr, &m_vkSwapchain );
	if( res == VK_SUCCESS )
	{
		vkGetSwapchainImagesKHR( device, m_vkSwapchain, &numImages, nullptr );
		m_vkImages.resize( numImages );
		vkGetSwapchainImagesKHR( device, m_vkSwapchain, &numImages, m_vkImages.data() );
		return true;
	}
	else
	{
		log_error( "Cannot create swap chain." );
		return false;
	}
}

bool SwapChain::createImageViews()
{
	m_vkImageViews.resize( m_vkImages.size() );

	for( auto i = 0; i < m_vkImages.size(); ++i )
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.pNext                           = nullptr;
		createInfo.flags                           = 0;
		createInfo.image                           = m_vkImages[ i ];
		createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format                          = m_vkFormat.format;
		createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel   = 0;
		createInfo.subresourceRange.levelCount     = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount     = 1;

		VkResult res = vkCreateImageView( m_pRenderer->getNativeDeviceHandle(),
		                                  &createInfo,
		                                  nullptr,
		                                  &m_vkImageViews[ i ] );
		if( res != VK_SUCCESS )
		{
			log_error( "Cannot create swap chain image views." );
			return false;
		}
	}
	return true;
}
