#include "windowsurface.h"
#include "renderer.h"
#include "window.h"

#include <GLFW/glfw3.h>

WindowSurface::WindowSurface( Window& window )
    : m_pWindow( &window ),
      m_Surface( VK_NULL_HANDLE )
{
	VkResult res = glfwCreateWindowSurface( Renderer::getNativeInstanceHandle(),
	                                        m_pWindow->getNativeHandle(),
	                                        nullptr,
	                                        &m_Surface );

	if( res != VK_SUCCESS )
	{
		log_error( "Cannot create window surface." );
	}
}

WindowSurface::~WindowSurface()
{
	if( m_Surface != VK_NULL_HANDLE )
	{
		vkDestroySurfaceKHR( Renderer::getNativeInstanceHandle(), m_Surface, nullptr );
	}
}
