#ifndef WINDOWSURFACE_H
#define WINDOWSURFACE_H

#include "common.h"

#include <vulkan/vulkan.h>

class Window;

class WindowSurface
{
friend class Window;

public:
    WindowSurface( Window& window );
	~WindowSurface();

	Window& getWindow()
	{
		return *m_pWindow;
	}

	VkSurfaceKHR getNativeHandle()
	{
		return m_Surface;
	}

private:
	Window*      m_pWindow;
	VkSurfaceKHR m_Surface;
};

#endif // WINDOWSURFACE_H
