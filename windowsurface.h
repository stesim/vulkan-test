#ifndef WINDOWSURFACE_H
#define WINDOWSURFACE_H

#include "common.h"

#include <vulkan/vulkan.h>

class Window;

class WindowSurface
{
public:
    WindowSurface( Window& window );
	~WindowSurface();

	void destroy();

	VkSurfaceKHR getNativeHandle()
	{
		return m_vkHandle;
	}

	bool         isValid()
	{
		return ( m_vkHandle != VK_NULL_HANDLE );
	}

	Window& getWindow()
	{
		return *m_pWindow;
	}

private:
	VkSurfaceKHR m_vkHandle;

	Window*      m_pWindow;
};

#endif // WINDOWSURFACE_H
