#include "renderer.h"
#include "window.h"
#include "windowsurface.h"
#include "swapchain.h"
#include "pipeline.h"
#include "memorypool.h"
#include "buffer.h"
#include "vertex.h"
#include "commandpool.h"
#include "commandbuffer.h"
#include "renderpass.h"

#include <set>
#include <unordered_set>
#include <string>
#include <cstring>

VkInstance               Renderer::s_VkInstance      = VK_NULL_HANDLE;
VkDebugReportCallbackEXT Renderer::s_VkDebugCallback = VK_NULL_HANDLE;

Renderer::Renderer( WindowSurface& surface )
    : m_vkPhysicalDevice( VK_NULL_HANDLE ),
      m_vkDevice( VK_NULL_HANDLE ),
      m_vkGraphicsQueue( VK_NULL_HANDLE ),
      m_vkPresentQueue( VK_NULL_HANDLE ),
      m_vkFramebuffers(),
      m_vkImageAvailableSemaphore( VK_NULL_HANDLE ),
      m_vkRenderFinishedSemaphore( VK_NULL_HANDLE ),
      m_ShaderCache( *this ),
      m_UsedQueueFamilies(),
      m_pWindowSurface( &surface ),
      m_pSwapchain( nullptr ),
      m_pRenderPass( nullptr ),
      m_pPipeline( nullptr ),
      m_pHostMemoryPool( nullptr ),
      m_pDeviceMemoryPool( nullptr ),
      m_pGeometryBuffer( nullptr ),
      m_pStagingBuffer( nullptr ),
      m_pCommandPool( nullptr ),
      m_CommandBuffers()
{
	if( selectPhysicalDevice() )
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties( m_vkPhysicalDevice, &properties );

		log_info( std::string( "Creating renderer using device: " ) + properties.deviceName );

		if( !createLogicalDevice() ||
		    !getQueues() ||
		    !createSwapchain() ||
		    !createRenderPass() ||
		    !createPipeline() ||
		    !createFramebuffers() ||
		    !createBuffers() ||
		    !createCommandPool() ||
		    !allocateCommandBuffers() ||
		    !recordCommandBuffers() ||
		    !createTransferBuffers() ||
		    !createSemaphores() ||
		    !copyStagingBuffer() )
		{
			destroy();
		}
	}
}

Renderer::~Renderer()
{
	destroy();
}

bool Renderer::initializeRenderingSystem( std::vector<const char*> extensions )
{
	VkApplicationInfo appInfo{};
	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext              = nullptr;
	appInfo.pApplicationName   = "vulkan-test";
	appInfo.applicationVersion = VK_MAKE_VERSION( 0, 0, 1 );
	appInfo.pEngineName        = "No Engine";
	appInfo.engineVersion      = VK_MAKE_VERSION( 0, 0, 1 );
	appInfo.apiVersion         = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext                   = nullptr;
	createInfo.flags                   = 0;
	createInfo.pApplicationInfo        = &appInfo;
#ifdef NDEBUG
	createInfo.enabledLayerCount       = 0;
	createInfo.ppEnabledLayerNames     = nullptr;
#else
	static const char* const debugLayer = "VK_LAYER_LUNARG_standard_validation";
	createInfo.enabledLayerCount       = 1;
	createInfo.ppEnabledLayerNames     = &debugLayer;

	extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
#endif
	createInfo.enabledExtensionCount   = (uint32_t)extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	log_info( "Creating Vulkan instance with extensions:" );
	for( auto ext : extensions )
	{
		log_info( ext );
	}

	VkResult res = vkCreateInstance( &createInfo, nullptr, &s_VkInstance );

#ifndef NDEBUG
	if( res == VK_SUCCESS && !attachDebugCallback() )
	{
		log_error( "Cannot attach Vulkan validation layer debug callback." );
		return false;
	}
#endif

	return ( res == VK_SUCCESS );
}

void Renderer::terminateRenderingSystem()
{
	if( s_VkDebugCallback != VK_NULL_HANDLE )
	{
		detachDebugCallback();
	}

	vkDestroyInstance( s_VkInstance, nullptr );
	s_VkInstance = VK_NULL_HANDLE;
}

Renderer::QueueFamilies Renderer::queryQueueFamilies( VkPhysicalDevice device, VkSurfaceKHR surface )
{
	uint32_t numQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties( device, &numQueueFamilies, nullptr );
	std::vector<VkQueueFamilyProperties> deviceFamilies( numQueueFamilies );
	vkGetPhysicalDeviceQueueFamilyProperties( device, &numQueueFamilies, deviceFamilies.data() );

	VkBool32 presentCapable = VK_FALSE;

	QueueFamilies families;
	for( auto i = 0; i < deviceFamilies.size(); ++i )
	{
		const auto& queueFamily = deviceFamilies[ i ];
		if( queueFamily.queueCount > 0 )
		{
			if( ( queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT ) != 0 )
			{
				families[ QueueFamily::Graphics ].index = i;
				families[ QueueFamily::Graphics ].count = queueFamily.queueCount;
			}
			if( ( queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT ) != 0 )
			{
				families[ QueueFamily::Transfer ].index = i;
				families[ QueueFamily::Transfer ].count = queueFamily.queueCount;
			}
			if( ( queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT ) != 0 )
			{
				families[ QueueFamily::Compute ].index = i;
				families[ QueueFamily::Compute ].count = queueFamily.queueCount;
			}

			if( surface != VK_NULL_HANDLE && presentCapable == VK_FALSE )
			{
				vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface, &presentCapable );
				if( presentCapable == VK_TRUE )
				{
					families[ QueueFamily::Present ].index = i;
					families[ QueueFamily::Present ].count = 1;
				}
			}
		}
	}

	return families;
}

void Renderer::destroy()
{
	safe_delete( m_pGeometryBuffer );
	safe_delete( m_pStagingBuffer );

	safe_delete( m_pDeviceMemoryPool );
	safe_delete( m_pHostMemoryPool );

	if( m_vkImageAvailableSemaphore != VK_NULL_HANDLE )
	{
		vkDestroySemaphore( m_vkDevice, m_vkImageAvailableSemaphore, nullptr );
		m_vkImageAvailableSemaphore = VK_NULL_HANDLE;
	}
	if( m_vkRenderFinishedSemaphore != VK_NULL_HANDLE )
	{
		vkDestroySemaphore( m_vkDevice, m_vkRenderFinishedSemaphore, nullptr );
		m_vkRenderFinishedSemaphore = VK_NULL_HANDLE;
	}

	safe_delete( m_pPipeline );
	safe_delete( m_pRenderPass );

	m_ShaderCache.destroy();

	cleanupSwapchain();

	safe_delete( m_pTransferCommandBuffer );
	safe_delete( m_pCommandPool );

	if( m_vkDevice != VK_NULL_HANDLE )
	{
		vkDestroyDevice( m_vkDevice, nullptr );
		m_vkDevice = VK_NULL_HANDLE;
	}
}

uint32_t Renderer::renderFrame()
{
	static const VkPipelineStageFlags waitStages[] = {
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	uint32_t imageIndex;
	VkResult res = vkAcquireNextImageKHR( m_vkDevice,
	                                      m_pSwapchain->getNativeHandle(),
	                                      ~(uint64_t)0,
	                                      m_vkImageAvailableSemaphore,
	                                      VK_NULL_HANDLE,
	                                      &imageIndex );

	if( res == VK_ERROR_OUT_OF_DATE_KHR )
	{
		log_info( "Recreating out-of-date swap chain." );

		recreateSwapchain();

		// attempt to acquire image after recreating the swap chain
		imageIndex = renderFrame();

		// give up if the attempt failed as well
		if( imageIndex == INVALID_FRAME )
		{
			log_error( "Cannot acquire swap chain image after swap chain recreation." );
		}

		return imageIndex;
	}
	else if( res == VK_SUBOPTIMAL_KHR )
	{
		log_warning( "Acquired image on suboptimal swap chain." );
	}
	else if( res != VK_SUCCESS )
	{
		log_error( "Cannot acquire swap chain image." );
		return INVALID_FRAME;
	}

	VkCommandBuffer commandBuffer = m_CommandBuffers[ imageIndex ]->getNativeHandle();

	VkSubmitInfo submitInfo{};
	submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext                = nullptr;
	submitInfo.waitSemaphoreCount   = 1;
	submitInfo.pWaitSemaphores      = &m_vkImageAvailableSemaphore;
	submitInfo.pWaitDstStageMask    = waitStages;
	submitInfo.commandBufferCount   = 1;
	submitInfo.pCommandBuffers      = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores    = &m_vkRenderFinishedSemaphore;

	vkQueueSubmit( m_vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	return imageIndex;
}

void Renderer::presentFrame( uint32_t imageIndex )
{
	VkSwapchainKHR swapChains[] = { m_pSwapchain->getNativeHandle() };

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext              = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores    = &m_vkRenderFinishedSemaphore;
	presentInfo.swapchainCount     = 1;
	presentInfo.pSwapchains        = swapChains;
	presentInfo.pImageIndices      = &imageIndex;
	presentInfo.pResults           = nullptr;

	VkResult res = vkQueuePresentKHR( m_vkPresentQueue, &presentInfo );

	if( res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR )
	{
		log_info( "Recreating out-of-date or suboptimal swap chain." );
		recreateSwapchain();
	}
}

void Renderer::waitForIdle()
{
	vkDeviceWaitIdle( m_vkDevice );
}

void Renderer::recreateSwapchain()
{
	SwapChain* newSwapchain = new SwapChain( *m_pSwapchain );

	bool recreateRenderPass = ( newSwapchain->getFormat() != m_pSwapchain->getFormat() );

	vkDeviceWaitIdle( m_vkDevice );

	if( recreateRenderPass )
	{
		safe_delete( m_pPipeline );
		safe_delete( m_pRenderPass );
	}

	cleanupSwapchain();

	if( newSwapchain->isValid() )
	{
		m_pSwapchain = newSwapchain;

		if( recreateRenderPass )
		{
			log_info( "Recreating render pass and pipeline due to swap chain format change." );
			createRenderPass();
			createPipeline();
		}

		createFramebuffers();
		allocateCommandBuffers();
		recordCommandBuffers();
	}
	else
	{
		destroy();
		log_error( "Cannot recreate swapchain." );
	}
}

VkBool32 Renderer::vulkanDebugCallback(
        VkDebugReportFlagsEXT      flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t                   obj,
        size_t                     location,
        int32_t                    code,
        const char*                layerPrefix,
        const char*                msg,
        void*                      userData )
{
	switch( flags )
	{
	case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
		log_info( msg );
		break;
	case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		log_debug( msg );
		break;
	case VK_DEBUG_REPORT_WARNING_BIT_EXT:
	case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
		log_warning( msg );
		break;
	default:
		log_error( msg );
		break;
	}
	return VK_FALSE;
}

bool Renderer::attachDebugCallback()
{
	auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
	                                          s_VkInstance, "vkCreateDebugReportCallbackEXT" );

	if( vkCreateDebugReportCallbackEXT != nullptr )
	{
		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.pNext       = nullptr;
		createInfo.flags       = VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		                         VK_DEBUG_REPORT_ERROR_BIT_EXT |
		                         VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		                         VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		                         VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = &vulkanDebugCallback;
		createInfo.pUserData   = nullptr;

		VkResult res = vkCreateDebugReportCallbackEXT(
		                   s_VkInstance, &createInfo, nullptr, &s_VkDebugCallback );
		return ( res == VK_SUCCESS );
	}
	return false;
}

void Renderer::detachDebugCallback()
{
	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
	                                           s_VkInstance, "vkDestroyDebugReportCallbackEXT" );

	if( vkDestroyDebugReportCallbackEXT != nullptr )
	{
		vkDestroyDebugReportCallbackEXT( s_VkInstance, s_VkDebugCallback, nullptr );
	}
	s_VkDebugCallback = VK_NULL_HANDLE;
}

bool Renderer::checkDeviceCompatibility( VkPhysicalDevice device, VkSurfaceKHR surface )
{
	// check queue families
	QueueFamilies queueFamilies = queryQueueFamilies( device, surface );
	bool requiredQueueFamiliesFound = ( queueFamilies[ QueueFamily::Graphics ].count > 0 &&
	                                    queueFamilies[ QueueFamily::Present ].count > 0 );

	// check device extensions
	std::unordered_set<std::string> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	uint32_t numDeviceExtensions;
	vkEnumerateDeviceExtensionProperties( device, nullptr, &numDeviceExtensions, nullptr );
	std::vector<VkExtensionProperties> deviceExtensions( numDeviceExtensions );
	vkEnumerateDeviceExtensionProperties( device,
	                                      nullptr,
	                                      &numDeviceExtensions,
	                                      deviceExtensions.data() );

	for( const auto& extension : deviceExtensions )
	{
		requiredExtensions.erase( extension.extensionName );
	}
	bool requiredExtensionsFound = requiredExtensions.empty();

	// check swap chain support
	bool swapChainIsSuitable = false;
	if( requiredExtensions.empty() ) // swapchain is supported
	{
		SwapChain::Capabilities capabilities = SwapChain::queryDeviceCapabilities( device,
		                                                                           surface );
		swapChainIsSuitable = ( !capabilities.formats.empty() &&
		                        !capabilities.presentModes.empty() );
	}

	return ( requiredQueueFamiliesFound &&
	         requiredExtensionsFound &&
	         swapChainIsSuitable );
}

bool Renderer::selectPhysicalDevice()
{
	uint32_t numDevices;
	vkEnumeratePhysicalDevices( s_VkInstance, &numDevices, nullptr );

	if( numDevices > 0 )
	{
		std::vector<VkPhysicalDevice> devices( numDevices );
		vkEnumeratePhysicalDevices( s_VkInstance, &numDevices, devices.data() );

		VkPhysicalDevice fallbackDevice = VK_NULL_HANDLE;
		for( const auto& device : devices )
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties( device, &properties );

			if( checkDeviceCompatibility( device, m_pWindowSurface->getNativeHandle() ) )
			{
				if( properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU )
				{
					fallbackDevice = device;
				}
				else if( properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
				{
					m_vkPhysicalDevice = device;
					break;
				}
			}
		}

		if( m_vkPhysicalDevice == VK_NULL_HANDLE )
		{
			m_vkPhysicalDevice = fallbackDevice;
		}
	}

	if( m_vkPhysicalDevice == VK_NULL_HANDLE )
	{
		log_error( "Cannot find suitable physical device." );
		return false;
	}
	return true;
}

bool Renderer::createLogicalDevice()
{
	static const float queuePriority = 1.0f;
	static const QueueFamily useQueueFamilies[] = { QueueFamily::Graphics, QueueFamily::Present };

	QueueFamilies availableQueueFamilies = queryQueueFamilies(
	                                           m_vkPhysicalDevice,
	                                           m_pWindowSurface->getNativeHandle() );

	std::set<uint32_t> uniqueQueueFamilies;
	for( auto family : useQueueFamilies )
	{
		uint32_t index = availableQueueFamilies[ family ].index;
		m_UsedQueueFamilies[ family ].index = index;
		m_UsedQueueFamilies[ family ].count = 1;

		uniqueQueueFamilies.insert( index );
	}

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos( uniqueQueueFamilies.size() );
	{
		size_t i = 0;
		for( auto familyIndex : uniqueQueueFamilies )
		{
			VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos[ i++ ];
			queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext            = nullptr;
			queueCreateInfo.flags            = 0;
			queueCreateInfo.queueFamilyIndex = familyIndex;
			queueCreateInfo.queueCount       = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
		}
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo createInfo{};
	createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext                   = nullptr;
	createInfo.flags                   = 0;
	createInfo.queueCreateInfoCount    = queueCreateInfos.size();
	createInfo.pQueueCreateInfos       = queueCreateInfos.data();
#ifdef NDEBUG
	createInfo.enabledLayerCount       = 0;
	createInfo.ppEnabledLayerNames     = nullptr;
#else
	const char* debugLayer = "VK_LAYER_LUNARG_standard_validation";
	createInfo.enabledLayerCount       = 1;
	createInfo.ppEnabledLayerNames     = &debugLayer;
#endif
	createInfo.enabledExtensionCount   = requiredExtensions.size();
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	createInfo.pEnabledFeatures        = &deviceFeatures;

	VkResult res = vkCreateDevice( m_vkPhysicalDevice, &createInfo, nullptr, &m_vkDevice );
	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create logical device." );
		return false;
	}
	return true;
}

bool Renderer::getQueues()
{
	QueueFamilies queueFamilies = queryQueueFamilies( m_vkPhysicalDevice,
	                                                  m_pWindowSurface->getNativeHandle() );

	vkGetDeviceQueue( m_vkDevice,
	                  queueFamilies[ QueueFamily::Graphics ].index,
	                  0,
	                  &m_vkGraphicsQueue );

	vkGetDeviceQueue( m_vkDevice,
	                  queueFamilies[ QueueFamily::Present ].index,
	                  0,
	                  &m_vkPresentQueue );

	if( m_vkGraphicsQueue == VK_NULL_HANDLE || m_vkPresentQueue == VK_NULL_HANDLE )
	{
		log_error( "Cannot get device queues." );
		return false;
	}
	return true;
}

bool Renderer::createSwapchain()
{
	m_pSwapchain = new SwapChain( *this );
	return m_pSwapchain->isValid();
}

bool Renderer::createRenderPass()
{
	m_pRenderPass = new RenderPass( *this, m_pSwapchain->getFormat() );
	return m_pRenderPass->isValid();
}

bool Renderer::createPipeline()
{
	m_pPipeline = new Pipeline( *m_pRenderPass,
	                            {
	                                &m_ShaderCache.getVertexShader( "vert" ),
	                                &m_ShaderCache.getFragmentShader( "frag" )
	                            } );
	return m_pPipeline->isValid();
}

bool Renderer::createFramebuffers()
{
	const auto& swapchainViews = m_pSwapchain->getImageViews();
	m_vkFramebuffers.resize( swapchainViews.size() );

	for( auto i = 0; i < swapchainViews.size(); ++i )
	{
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.pNext           = nullptr;
		createInfo.flags           = 0;
		createInfo.renderPass      = m_pPipeline->getRenderPass().getNativeHandle();
		createInfo.attachmentCount = 1;
		createInfo.pAttachments    = &swapchainViews[ i ];
		m_pSwapchain->getExtent( createInfo.width, createInfo.height );
		createInfo.layers          = 1;

		VkResult res = vkCreateFramebuffer( m_vkDevice,
		                                    &createInfo,
		                                    nullptr,
		                                    &m_vkFramebuffers[ i ] );

		if( res != VK_SUCCESS )
		{
			log_error( "Cannot create framebuffers." );
			return false;
		}
	}
	return true;
}

bool Renderer::createCommandPool()
{
	m_pCommandPool = new CommandPool( *this, m_UsedQueueFamilies[ QueueFamily::Graphics ].index );
	return m_pCommandPool->isValid();
}

bool Renderer::allocateCommandBuffers()
{
	m_CommandBuffers.resize( m_pSwapchain->getImageViews().size() );

	// TODO: implement allocation of multiple command buffers with single call
	for( auto i = 0; i < m_CommandBuffers.size(); ++i )
	{
		m_CommandBuffers[ i ] = new CommandBuffer( *m_pCommandPool );

		if( !m_CommandBuffers[ i ]->isValid() )
			return false;
	}

	return true;
}

bool Renderer::recordCommandBuffers()
{
	static const VkClearValue clearColor{ VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } } };

	VkRect2D renderArea = { { 0, 0 }, m_pSwapchain->getExtent() };

	VkViewport viewport{};
	viewport.x        = 0.0f;
	viewport.y        = 0.0f;
	viewport.width    = renderArea.extent.width;
	viewport.height   = renderArea.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	for( auto i = 0; i < m_CommandBuffers.size(); ++i )
	{
		CommandBuffer& commandBuffer = *m_CommandBuffers[ i ];

		if( !commandBuffer.begin( VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT ) )
			return false;

		commandBuffer.beginRenderPass( *m_pRenderPass,
		                               m_vkFramebuffers[ i ],
		                               renderArea,
		                               { clearColor } );

		commandBuffer.bindPipeline( VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pPipeline );

		commandBuffer.bindVertexBuffers( 0, { m_pGeometryBuffer }, { 0 } );
		commandBuffer.bindIndexBuffer( *m_pGeometryBuffer, 4 * sizeof( Vertex ), VK_INDEX_TYPE_UINT32 ); // TODO: replace fixed offset !!!!!!!

		commandBuffer.setViewports( 0, { viewport } );
		commandBuffer.setScissors( 0, { renderArea } );

		//commandBuffer.draw( 0, m_pGeometryBuffer->getSize() / sizeof( Vertex ), 0, 1 );
		commandBuffer.drawIndexed( 0, 6, 0, 0, 1 ); // TODO: replace fixed index count !!!!!!!

		commandBuffer.endRenderPass();

		if( !commandBuffer.end() )
			return false;
	}

	return true;
}

bool Renderer::createTransferBuffers()
{
	m_pTransferCommandBuffer = new CommandBuffer( *m_pCommandPool );

	if( !m_pTransferCommandBuffer->isValid() ||
	    !m_pTransferCommandBuffer->begin( VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ) )
	{
		return false;
	}

	VkBufferCopy region{};
	region.srcOffset = 0;
	region.dstOffset = 0;
	region.size      = m_pStagingBuffer->getSize();

	m_pTransferCommandBuffer->copyBuffer( *m_pGeometryBuffer, *m_pStagingBuffer, { region } );

	m_pTransferCommandBuffer->end();

	return true;
}

bool Renderer::createSemaphores()
{
	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;

	VkResult res = vkCreateSemaphore( m_vkDevice,
	                                   &createInfo,
	                                   nullptr,
	                                   &m_vkImageAvailableSemaphore );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create semaphores." );
		return false;
	}

	res = vkCreateSemaphore( m_vkDevice,
	                         &createInfo,
	                         nullptr,
	                         &m_vkRenderFinishedSemaphore );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create semaphores." );
		return false;
	}
	return true;
}

bool Renderer::createBuffers()
{
	static const std::vector<Vertex> vertices = {
	    { { -0.50f,  0.50f }, { 1.0f, 0.0f, 0.0f } },
	    { { -0.50f, -0.50f }, { 0.0f, 1.0f, 0.0f } },
	    { {  0.50f, -0.50f }, { 0.0f, 0.0f, 1.0f } },
	    { {  0.50f,  0.50f }, { 1.0f, 1.0f, 1.0f } }
	};

	static const std::vector<uint32_t> indices = {
	    0, 1, 2,
	    2, 3, 0
	};

	m_pStagingBuffer = new Buffer( *this,
	                               vertices.size() * sizeof( Vertex ) +
	                               indices.size() * sizeof( uint32_t ),
	                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT );

	if( !m_pStagingBuffer->isValid() )
		return false;

	uint32_t typeFilter;
	m_pStagingBuffer->getMemoryRequirements( nullptr, &typeFilter, nullptr );

	m_pHostMemoryPool = new MemoryPool( *this,
	                                    1024,
	                                    typeFilter,
	                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

	if( !m_pHostMemoryPool->isValid() ||
	    !m_pStagingBuffer->allocateMemoryFromPool( *m_pHostMemoryPool ) )
	{
		return false;
	}

	void* mappedPtr = m_pStagingBuffer->map();
	std::memcpy( mappedPtr, vertices.data(), vertices.size() * sizeof( Vertex ) );
	std::memcpy( (char*)mappedPtr + vertices.size() * sizeof( Vertex ),
	             indices.data(),
	             indices.size() * sizeof( uint32_t ) );
	m_pStagingBuffer->unmap();


	m_pGeometryBuffer = new Buffer( *this,
	                                m_pStagingBuffer->getSize(),
	                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
	                                VK_BUFFER_USAGE_TRANSFER_DST_BIT );

	if( !m_pGeometryBuffer->isValid() )
		return false;

	m_pGeometryBuffer->getMemoryRequirements( nullptr, &typeFilter, nullptr );

	m_pDeviceMemoryPool = new MemoryPool( *this,
	                                      1024,
	                                      typeFilter,
	                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

	if( !m_pHostMemoryPool->isValid() ||
	    ! m_pGeometryBuffer->allocateMemoryFromPool( *m_pDeviceMemoryPool ) )
	{
		return false;
	}

	return true;
}

bool Renderer::copyStagingBuffer()
{
	VkCommandBuffer commandBuffer = m_pTransferCommandBuffer->getNativeHandle();

	VkSubmitInfo submitInfo{};
	submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext                = nullptr;
	submitInfo.waitSemaphoreCount   = 0;
	submitInfo.pWaitSemaphores      = nullptr;
	submitInfo.pWaitDstStageMask    = nullptr;
	submitInfo.commandBufferCount   = 1;
	submitInfo.pCommandBuffers      = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores    = nullptr;

	VkResult res = vkQueueSubmit( m_vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	if( res != VK_SUCCESS )
		return false;

	vkQueueWaitIdle( m_vkGraphicsQueue );

	return true;
}

void Renderer::cleanupSwapchain()
{
	for( auto framebuffer : m_vkFramebuffers )
	{
		if( framebuffer != VK_NULL_HANDLE )
		{
			vkDestroyFramebuffer( m_vkDevice, framebuffer, nullptr );
		}
	}
	m_vkFramebuffers.clear();

	for( auto commandBuffer : m_CommandBuffers )
	{
		delete commandBuffer;
	}
	m_CommandBuffers.clear();

	safe_delete( m_pSwapchain );
}
