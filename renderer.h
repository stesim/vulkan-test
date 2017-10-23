#ifndef RENDERER_H
#define RENDERER_H

#include "common.h"
#include "shadercache.h"

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class WindowSurface;
class SwapChain;
class Pipeline;
class MemoryPool;
class Buffer;
class CommandPool;
class CommandBuffer;

struct TransformUBO
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

enum class QueueFamily
{
	Graphics = 0,
	Transfer,
	Compute,
	Present,
	COUNT
};

class Renderer
{
public:
	static constexpr uint32_t INVALID_FRAME = ~(uint32_t)0;

	class QueueFamilies
	{
	friend class Renderer;

	public:
	    struct Desc
		{
			uint32_t index;
			uint32_t count;
		};

	public:
		const Desc& operator[]( QueueFamily family ) const
		{
			return m_Descriptors[ (size_t)family ];
		}

	private:
		Desc& operator[]( QueueFamily family )
		{
			return m_Descriptors[ (size_t)family ];
		}

	private:
		VkSurfaceKHR m_CompatibleSurface;
		Desc         m_Descriptors[ (size_t)QueueFamily::COUNT ];
	};

public:
	Renderer( WindowSurface& surface );
	~Renderer();

	static VkInstance    getNativeInstanceHandle()
	{
		return s_VkInstance;
	}

	static bool          initializeRenderingSystem( std::vector<const char*> extensions );
	static void          terminateRenderingSystem();

	static QueueFamilies queryQueueFamilies( VkPhysicalDevice device,
	                                         VkSurfaceKHR surface );

	void destroy();

	VkPhysicalDevice     getNativePhysicalDeviceHandle()
	{
		return m_vkPhysicalDevice;
	}
	VkDevice             getNativeDeviceHandle()
	{
		return m_vkDevice;
	}

	WindowSurface&       getWindowSurface()
	{
		return *m_pWindowSurface;
	}
	const QueueFamilies& getQueueFamilies()
	{
		return m_UsedQueueFamilies;
	}
	SwapChain&           getSwapChain()
	{
		return *m_pSwapchain;
	}
	Pipeline&            getPipeline()
	{
		return *m_pPipeline;
	}

	uint32_t renderFrame();
	void     presentFrame( uint32_t imageIndex );

	void     waitForIdle();
	void     recreateSwapchain();

private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
	        VkDebugReportFlagsEXT      flags,
	        VkDebugReportObjectTypeEXT objType,
	        uint64_t                   obj,
	        size_t                     location,
	        int32_t                    code,
	        const char*                layerPrefix,
	        const char*                msg,
	        void*                      userData );

	static bool attachDebugCallback();
	static void detachDebugCallback();

	static bool checkDeviceCompatibility( VkPhysicalDevice device,
	                                      VkSurfaceKHR surface );

	bool selectPhysicalDevice();
	bool createLogicalDevice();
	bool getQueues();
	bool createSwapchain();
	bool createPipeline();
	bool createFramebuffers();
	bool createCommandPool();
	bool allocateCommandBuffers();
	bool recordCommandBuffers();
	bool createTransferBuffers();
	bool createSemaphores();
	bool createBuffers();

	bool copyStagingBuffer();

	void cleanupSwapchain();

private:
	static VkInstance               s_VkInstance;
	static VkDebugReportCallbackEXT s_VkDebugCallback;

	VkPhysicalDevice             m_vkPhysicalDevice;
	VkDevice                     m_vkDevice;
	VkQueue                      m_vkGraphicsQueue;
	VkQueue                      m_vkPresentQueue;
	std::vector<VkFramebuffer>   m_vkFramebuffers;
	VkSemaphore                  m_vkImageAvailableSemaphore;
	VkSemaphore                  m_vkRenderFinishedSemaphore;

	ShaderCache                  m_ShaderCache;
	QueueFamilies                m_UsedQueueFamilies;

	WindowSurface*               m_pWindowSurface;
	SwapChain*                   m_pSwapchain;
	Pipeline*                    m_pPipeline;
	MemoryPool*                  m_pHostMemoryPool;
	MemoryPool*                  m_pDeviceMemoryPool;
	Buffer*                      m_pGeometryBuffer;
	Buffer*                      m_pStagingBuffer;
	CommandPool*                 m_pCommandPool;
	std::vector<CommandBuffer*>  m_CommandBuffers;
	CommandBuffer*               m_pTransferCommandBuffer;
};

#endif // RENDERER_H
