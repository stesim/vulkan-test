#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"

#include <vulkan/vulkan.h>
#include <vector>

class WindowSurface;
class Renderer;

class SwapChain
{
public:
	struct Capabilities
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

public:
	SwapChain();
	SwapChain( Renderer& renderer );
	SwapChain( SwapChain& oldSwapchain );
	~SwapChain();

	static Capabilities queryDeviceCapabilities( VkPhysicalDevice device,
	                                             VkSurfaceKHR surface );

	//SwapChain& operator=( SwapChain&& rhs );

	void                destroy();

	VkSwapchainKHR      getNativeHandle()
	{
		return m_vkSwapchain;
	}
	bool                isValid()
	{
		return ( m_vkSwapchain != VK_NULL_HANDLE );
	}

	VkExtent2D          getExtent()
	{
		return m_vkExtent;
	}
	void                getExtent( uint32_t& width, uint32_t& height )
	{
		width  = m_vkExtent.width;
		height = m_vkExtent.height;
	}
	VkFormat            getFormat()
	{
		return m_vkFormat.format;
	}

	const std::vector<VkImageView> getImageViews()
	{
		return m_vkImageViews;
	}

private:
	VkSurfaceFormatKHR  selectSurfaceFormat( const Capabilities& capabilities );
	VkPresentModeKHR    selectPresentMode( const Capabilities& capabilities );
	VkExtent2D          selectSurfaceExtent( const Capabilities& capabilities );

	bool                createSwapchain( SwapChain* oldSwapchain );
	bool                createImageViews();

private:
	VkSwapchainKHR             m_vkSwapchain;
	VkSurfaceFormatKHR         m_vkFormat;
	VkExtent2D                 m_vkExtent;
	VkPresentModeKHR           m_vkPresentMode;
	std::vector<VkImage>       m_vkImages;
	std::vector<VkImageView>   m_vkImageViews;

	Renderer*                  m_pRenderer;
};

#endif // SWAPCHAIN_H
