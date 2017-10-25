#include "windowsurface.h"
#include "renderer.h"
#include "window.h"

#include <GLFW/glfw3.h>

WindowSurface::WindowSurface( Window& window )
    : m_vkHandle( VK_NULL_HANDLE ),
      m_pWindow( &window )
{
	VkResult res = glfwCreateWindowSurface( Renderer::getNativeInstanceHandle(),
	                                        m_pWindow->getNativeHandle(),
	                                        nullptr,
	                                        &m_vkHandle );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create window surface." );
		destroy();
	}
}

WindowSurface::~WindowSurface()
{
	destroy();
}

void WindowSurface::destroy()
{
	if( m_vkHandle != VK_NULL_HANDLE )
	{
		vkDestroySurfaceKHR( Renderer::getNativeInstanceHandle(), m_vkHandle, nullptr );
		m_vkHandle = VK_NULL_HANDLE;
	}
}
