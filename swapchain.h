#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"
#include "vulkanobjectwrapper.h"

#include <vulkan/vulkan.h>
#include <vector>

class WindowSurface;
class Renderer;

class SwapChain : public VulkanObjectWrapper<VkSwapchainKHR, vkDestroySwapchainKHR>
{
public:
	struct Capabilities
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

public:
	SwapChain() = default;
	SwapChain( Renderer& renderer );
	SwapChain( SwapChain& oldSwapchain );

	virtual ~SwapChain();

	static Capabilities queryDeviceCapabilities( VkPhysicalDevice device,
	                                             VkSurfaceKHR surface );

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
	SwapChain( Renderer& renderer, SwapChain* oldSwapchain );

private:
	VkSurfaceFormatKHR  selectSurfaceFormat( const Capabilities& capabilities );
	VkPresentModeKHR    selectPresentMode( const Capabilities& capabilities );
	VkExtent2D          selectSurfaceExtent( const Capabilities& capabilities );

	bool                createSwapchain( SwapChain* oldSwapchain );
	bool                createImageViews();

private:
	VkSurfaceFormatKHR         m_vkFormat;
	VkExtent2D                 m_vkExtent;
	VkPresentModeKHR           m_vkPresentMode;
	std::vector<VkImage>       m_vkImages;
	std::vector<VkImageView>   m_vkImageViews;

	Renderer*                  m_pRenderer;
};

#endif // SWAPCHAIN_H
